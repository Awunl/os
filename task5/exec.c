#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include "tree.h"
#include "exec.h"

// Определяем глобальную переменную здесь
backgrndList *bckgrnd = NULL;

// Инициализация списка фоновых процессов
static void init_background_list(void) {
    if (bckgrnd == NULL) {
        bckgrnd = malloc(sizeof(backgrndList));
        bckgrnd->head = NULL;
        bckgrnd->tail = NULL;
        bckgrnd->count = 0;
    }
}

// Добавление фонового процесса в список
void add_background_process(backgrndList *list, pid_t pid) {
    background_process *new_proc = malloc(sizeof(background_process));
    if (!new_proc) return;
    
    new_proc->pid = pid;
    new_proc->next = NULL;
    
    if (list->head == NULL) {
        list->head = new_proc;
        list->tail = new_proc;
    } else {
        list->tail->next = new_proc;
        list->tail = new_proc;
    }
    list->count++;
    
    printf("[%d] %d\n", list->count, pid);
}

// Ожидание завершения фоновых процессов
void wait_for_background(backgrndList *list) {
    if (!list || list->head == NULL) return;
    
    background_process *current = list->head;
    background_process *prev = NULL;
    int status;
    
    while (current != NULL) {
        pid_t result = waitpid(current->pid, &status, WNOHANG);
        
        if (result > 0) {
            // Процесс завершился
            printf("[%d] Завершен процесс %d\n", 
                   (current == list->head ? 1 : 2), current->pid);
            
            // Удаляем из списка
            if (prev == NULL) {
                list->head = current->next;
                if (list->head == NULL) {
                    list->tail = NULL;
                }
            } else {
                prev->next = current->next;
                if (current->next == NULL) {
                    list->tail = prev;
                }
            }
            
            background_process *to_free = current;
            current = current->next;
            free(to_free);
            list->count--;
        } else if (result == 0) {
            // Процесс ещё работает
            prev = current;
            current = current->next;
        } else {
            // Ошибка
            perror("waitpid");
            prev = current;
            current = current->next;
        }
    }
}

// Очистка списка фоновых процессов
void cleanup_background_processes(backgrndList *list) {
    if (!list) return;
    
    background_process *current = list->head;
    while (current != NULL) {
        background_process *next = current->next;
        kill(current->pid, SIGTERM);
        free(current);
        current = next;
    }
    
    free(list);
    bckgrnd = NULL;
}

// Вспомогательная функция для открытия файлов с учетом флагов
static int open_file(const char *filename, int flags, mode_t mode) {
    int fd = open(filename, flags, mode);
    if (fd < 0) {
        perror(filename);
        exit(EXIT_FAILURE);
    }
    return fd;
}

// Выполнение простой команды (без конвейеров)
void execute_simple_command(tree t) {
    if (t == NULL) return;
    
    // Если это подкоманда в скобках
    if (t->psubcmd != NULL) {
        execute_subshell(t);
        return;
    }
    
    // Если нет команды для выполнения
    if (t->argv == NULL || t->argv[0] == NULL) {
        return;
    }
    
    // Проверка на встроенные команды
    if (strcmp(t->argv[0], "cd") == 0) {
        // Встроенная команда cd
        if (t->argv[1] == NULL) {
            fprintf(stderr, "cd: ожидается аргумент\n");
        } else {
            if (chdir(t->argv[1]) != 0) {
                perror("cd");
            }
        }
        return;
    } else if (strcmp(t->argv[0], "exit") == 0) {
        // Встроенная команда exit
        printf("Выход из оболочки\n");
        exit(EXIT_SUCCESS);
    }
    
    // Создаем процесс для выполнения внешней команды
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }
    
    if (pid == 0) {
        // Дочерний процесс
        
        // Обработка перенаправления ввода
        if (t->infile != NULL) {
            int fd_in = open_file(t->infile, O_RDONLY, 0);
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }
        
        // Обработка перенаправления вывода
        if (t->outfile != NULL) {
            int flags = O_WRONLY | O_CREAT;
            if (t->append) {
                flags |= O_APPEND;
            } else {
                flags |= O_TRUNC;
            }
            int fd_out = open_file(t->outfile, flags, 0644);
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }
        
        // Выполняем команду
        execvp(t->argv[0], t->argv);
        
        // Если execvp вернул управление, значит произошла ошибка
        perror(t->argv[0]);
        exit(EXIT_FAILURE);
    } else {
        // Родительский процесс
        if (!t->backgrnd) {
            // Ожидаем завершения дочернего процесса
            int status;
            waitpid(pid, &status, 0);
        } else {
            // Фоновый режим - добавляем в список
            init_background_list();
            add_background_process(bckgrnd, pid);
        }
    }
}

// Выполнение конвейера (пайплайна)
void execute_pipeline(tree t) {
    if (t == NULL) return;
    
    // Если это всего одна команда в конвейере
    if (t->pipe == NULL) {
        execute_simple_command(t);
        return;
    }
    
    // Создаем массив PID для отслеживания процессов конвейера
    int pipe_count = 0;
    tree current = t;
    while (current != NULL) {
        pipe_count++;
        current = current->pipe;
    }
    
    // Создаем пайпы для связи между командами
    int (*pipes)[2] = malloc((pipe_count - 1) * sizeof(int[2]));
    if (!pipes) {
        perror("malloc");
        return;
    }
    
    for (int i = 0; i < pipe_count - 1; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("pipe");
            free(pipes);
            return;
        }
    }
    
    // Создаем процессы для каждой команды в конвейере
    pid_t *pids = malloc(pipe_count * sizeof(pid_t));
    if (!pids) {
        perror("malloc");
        free(pipes);
        return;
    }
    
    current = t;
    for (int i = 0; i < pipe_count; i++) {
        pids[i] = fork();
        
        if (pids[i] < 0) {
            perror("fork");
            free(pipes);
            free(pids);
            return;
        }
        
        if (pids[i] == 0) {
            // Дочерний процесс
            
            // Подключаем входной поток
            if (i > 0) {
                dup2(pipes[i-1][0], STDIN_FILENO);
            }
            
            // Подключаем выходной поток
            if (i < pipe_count - 1) {
                dup2(pipes[i][1], STDOUT_FILENO);
            }
            
            // Закрываем все пайпы в дочернем процессе
            for (int j = 0; j < pipe_count - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            
            // Выполняем текущую команду
            if (current->argv != NULL && current->argv[0] != NULL) {
                // Обработка перенаправлений для каждой команды
                if (current->infile != NULL && i == 0) {
                    int fd_in = open_file(current->infile, O_RDONLY, 0);
                    dup2(fd_in, STDIN_FILENO);
                    close(fd_in);
                }
                
                if (current->outfile != NULL && i == pipe_count - 1) {
                    int flags = O_WRONLY | O_CREAT;
                    if (current->append) {
                        flags |= O_APPEND;
                    } else {
                        flags |= O_TRUNC;
                    }
                    int fd_out = open_file(current->outfile, flags, 0644);
                    dup2(fd_out, STDOUT_FILENO);
                    close(fd_out);
                }
                
                execvp(current->argv[0], current->argv);
                perror(current->argv[0]);
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        }
        
        current = current->pipe;
    }
    
    // Родительский процесс: закрываем все пайпы
    for (int i = 0; i < pipe_count - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
    
    // Ожидаем завершения всех процессов конвейера
    if (!t->backgrnd) {
        for (int i = 0; i < pipe_count; i++) {
            waitpid(pids[i], NULL, 0);
        }
    } else {
        // Фоновый режим - добавляем все процессы в список
        init_background_list();
        for (int i = 0; i < pipe_count; i++) {
            add_background_process(bckgrnd, pids[i]);
        }
    }
    
    free(pipes);
    free(pids);
}

// Выполнение подкоманды в скобках
void execute_subshell(tree t) {
    if (t == NULL || t->psubcmd == NULL) return;
    
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }
    
    if (pid == 0) {
        // Дочерний процесс выполняет подкоманду
        execute_tree(t->psubcmd);
        exit(EXIT_SUCCESS);
    } else {
        // Родительский процесс ожидает завершения
        if (!t->backgrnd) {
            int status;
            waitpid(pid, &status, 0);
        } else {
            // Фоновый режим
            init_background_list();
            add_background_process(bckgrnd, pid);
        }
    }
}

// Основная функция выполнения дерева команд
int execute_tree(tree t) {
    if (t == NULL) return 0;
    
    while (t != NULL) {
        // Ожидаем завершения фоновых процессов перед выполнением новой команды
        if (bckgrnd != NULL) {
            wait_for_background(bckgrnd);
        }
        
        // Выполняем текущую команду в зависимости от наличия конвейера
        if (t->pipe != NULL) {
            execute_pipeline(t);
        } else {
            execute_simple_command(t);
        }
        
        // Проверяем условное выполнение
        if (t->type == AND) {
            // && - выполнять следующую команду только если текущая успешна
            // В реальной реализации нужно проверять код возврата
            // Здесь для простоты всегда переходим к следующей
            t = t->next;
        } else if (t->type == OR) {
            // || - выполнять следующую команду только если текущая неуспешна
            // Здесь для простоты всегда переходим к следующей
            t = t->next;
        } else {
            // ; или конец - всегда переходим к следующей
            t = t->next;
        }
    }
    
    return 0;
}
