#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include "tree.h"
#include "list.h"

// Указатель на текущий элемент списка для разбора
static list_node *current = NULL;

// Вспомогательная функция strdup (если не определена в стандартной библиотеке)
#ifndef strdup
static char *my_strdup(const char *s) {
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char *dup = malloc(len);
    if (dup) {
        memcpy(dup, s, len);
    }
    return dup;
}
#define strdup my_strdup
#endif

// Прототипы статических функций
static tree parse_command_list(void);
static tree parse_pipeline(void);
static tree parse_simple_command(void);
static void add_argument(tree node, const char *arg);
static tree create_node(void);
static int is_operator(const char *word);
static int is_redirection(const char *word);
static int is_background(const char *word);
static int is_subshell_start(const char *word);
static int is_subshell_end(const char *word);

// Функция обработки ошибок
void error(const char *msg) {
    fprintf(stderr, "Ошибка синтаксиса: %s\n", msg);
    longjmp(begin, 1);
}

// Вспомогательные функции для проверки типа токена
static int is_operator(const char *word) {
    return strcmp(word, "|") == 0 || strcmp(word, "&&") == 0 || 
           strcmp(word, "||") == 0 || strcmp(word, ";") == 0;
}

static int is_redirection(const char *word) {
    return strcmp(word, "<") == 0 || strcmp(word, ">") == 0 || 
           strcmp(word, ">>") == 0;
}

static int is_background(const char *word) {
    return strcmp(word, "&") == 0;
}

static int is_subshell_start(const char *word) {
    return strcmp(word, "(") == 0;
}

static int is_subshell_end(const char *word) {
    return strcmp(word, ")") == 0;
}

// Создание нового узла дерева
static tree create_node(void) {
    tree node = (tree)malloc(sizeof(struct cmd_inf));
    if (!node) {
        error("Не удалось выделить память");
    }
    
    node->argv = NULL;
    node->infile = NULL;
    node->outfile = NULL;
    node->append = 0;
    node->backgrnd = 0;
    node->psubcmd = NULL;
    node->pipe = NULL;
    node->next = NULL;
    node->type = NXT;
    
    return node;
}

// Добавление аргумента в массив argv
static void add_argument(tree node, const char *arg) {
    int count = 0;
    
    // Подсчитываем текущее количество аргументов
    if (node->argv) {
        while (node->argv[count] != NULL) {
            count++;
        }
    }
    
    // Выделяем память под новый массив
    char **new_argv = (char **)realloc(node->argv, (count + 2) * sizeof(char *));
    if (!new_argv) {
        error("Не удалось выделить память для аргументов");
    }
    
    node->argv = new_argv;
    node->argv[count] = strdup(arg);
    node->argv[count + 1] = NULL;
}

// Разбор простой команды (самый низкий уровень)
static tree parse_simple_command(void) {
    tree cmd = create_node();
    
    while (current != NULL) {
        if (is_redirection(current->word)) {
            // Обработка перенаправления
            char *op = current->word;
            current = current->next;
            
            if (current == NULL) {
                error("Ожидается имя файла после оператора перенаправления");
            }
            
            if (strcmp(op, "<") == 0) {
                if (cmd->infile) {
                    error("Неоднозначное перенаправление ввода");
                }
                cmd->infile = strdup(current->word);
            } 
            else if (strcmp(op, ">") == 0) {
                if (cmd->outfile) {
                    error("Неоднозначное перенаправление вывода");
                }
                cmd->outfile = strdup(current->word);
                cmd->append = 0;
            } 
            else if (strcmp(op, ">>") == 0) {
                if (cmd->outfile) {
                    error("Неоднозначное перенаправление вывода");
                }
                cmd->outfile = strdup(current->word);
                cmd->append = 1;
            }
            
            current = current->next;
        } 
        else if (is_subshell_start(current->word)) {
            // Обработка подкоманды в скобках
            current = current->next;
            cmd->psubcmd = parse_command_list();
            
            if (current == NULL || !is_subshell_end(current->word)) {
                error("Ожидается закрывающая скобка ')'");
            }
            current = current->next;
        }
        else if (is_operator(current->word) || is_background(current->word) || 
                 is_subshell_end(current->word)) {
            // Конец простой команды
            break;
        }
        else {
            // Обычное слово (команда или аргумент)
            add_argument(cmd, current->word);
            current = current->next;
        }
    }
    
    // Проверка: команда должна иметь либо аргументы, либо подкоманду
    if (cmd->argv == NULL && cmd->psubcmd == NULL) {
        // Но может быть только перенаправление, это допустимо
        if (cmd->infile == NULL && cmd->outfile == NULL) {
            free(cmd);
            return NULL;
        }
    }
    
    return cmd;
}

// Разбор конвейера (pipeline)
static tree parse_pipeline(void) {
    tree first = parse_simple_command();
    if (first == NULL) return NULL;
    
    tree current_cmd = first;
    
    while (current != NULL && strcmp(current->word, "|") == 0) {
        current = current->next; // Пропускаем '|'
        
        tree next_cmd = parse_simple_command();
        if (next_cmd == NULL) {
            error("Ожидается команда после '|'");
        }
        
        current_cmd->pipe = next_cmd;
        current_cmd = next_cmd;
    }
    
    return first;
}

// Разбор списка команд (самый верхний уровень)
static tree parse_command_list(void) {
    tree first = parse_pipeline();
    if (first == NULL) return NULL;
    
    tree last = first;
    
    while (current != NULL) {
        if (is_background(current->word)) {
            // Фоновый режим
            last->backgrnd = 1;
            current = current->next;
            
            // После & может быть конец или ;
            if (current != NULL && strcmp(current->word, ";") == 0) {
                current = current->next;
            }
            break;
        }
        else if (strcmp(current->word, ";") == 0) {
            // Простая последовательность
            current = current->next;
            last->type = NXT;
            
            tree next_cmd = parse_pipeline();
            if (next_cmd == NULL) {
                // Допустимо: команда может заканчиваться на ;
                break;
            }
            
            last->next = next_cmd;
            last = next_cmd;
        }
        else if (strcmp(current->word, "&&") == 0) {
            // Логическое И
            current = current->next;
            last->type = AND;
            
            tree next_cmd = parse_pipeline();
            if (next_cmd == NULL) {
                error("Ожидается команда после '&&'");
            }
            
            last->next = next_cmd;
            last = next_cmd;
        }
        else if (strcmp(current->word, "||") == 0) {
            // Логическое ИЛИ
            current = current->next;
            last->type = OR;
            
            tree next_cmd = parse_pipeline();
            if (next_cmd == NULL) {
                error("Ожидается команда после '||'");
            }
            
            last->next = next_cmd;
            last = next_cmd;
        }
        else if (is_subshell_end(current->word)) {
            // Конец подкоманды
            break;
        }
        else {
            // Неожиданный токен
            error("Неожиданный токен в списке команд");
        }
    }
    
    return first;
}

// Основная функция построения дерева из списка
tree build_tree(List *list) {
    if (list == NULL || list->head == NULL) {
        return NULL;
    }
    
    current = list->head;
    tree result = parse_command_list();
    
    // Проверяем, что разобрали все токены
    if (current != NULL) {
        error("Лишние токены в конце команды");
    }
    
    return result;
}

// Функция для отступа при печати дерева
static void make_shift(int n) {
    for (int i = 0; i < n; i++) {
        putchar(' ');
    }
}

// Печать массива аргументов
static void print_argv(char **p, int shift) {
    if (p == NULL) return;
    
    int i = 0;
    while (p[i] != NULL) {
        make_shift(shift);
        printf("argv[%d] = %s\n", i, p[i]);
        i++;
    }
}

// Печать дерева (для отладки)
void print_tree(tree t, int shift) {
    if (t == NULL) {
        make_shift(shift);
        printf("(пусто)\n");
        return;
    }
    
    // Печатаем аргументы
    if (t->argv != NULL) {
        print_argv(t->argv, shift);
    } else {
        make_shift(shift);
        printf("psubshell\n");
    }
    
    // Печатаем перенаправления
    make_shift(shift);
    if (t->infile == NULL) {
        printf("infile = NULL\n");
    } else {
        printf("infile = %s\n", t->infile);
    }
    
    make_shift(shift);
    if (t->outfile == NULL) {
        printf("outfile = NULL\n");
    } else {
        printf("outfile = %s\n", t->outfile);
    }
    
    // Печатаем флаги
    make_shift(shift);
    printf("append = %d\n", t->append);
    
    make_shift(shift);
    printf("background = %d\n", t->backgrnd);
    
    // Печатаем тип связи
    make_shift(shift);
    char *type_str;
    switch (t->type) {
        case NXT: type_str = "NXT"; break;
        case AND: type_str = "AND"; break;
        case OR:  type_str = "OR";  break;
        default:  type_str = "UNKNOWN";
    }
    printf("type = %s\n", type_str);
    
    // Печатаем подкоманду
    make_shift(shift);
    if (t->psubcmd == NULL) {
        printf("psubcmd = NULL\n");
    } else {
        printf("psubcmd --->\n");
        print_tree(t->psubcmd, shift + 4);
    }
    
    // Печатаем конвейер
    make_shift(shift);
    if (t->pipe == NULL) {
        printf("pipe = NULL\n");
    } else {
        printf("pipe --->\n");
        print_tree(t->pipe, shift + 4);
    }
    
    // Печатаем следующую команду
    make_shift(shift);
    if (t->next == NULL) {
        printf("next = NULL\n");
    } else {
        printf("next --->\n");
        print_tree(t->next, shift + 4);
    }
}

// Очистка дерева
void clear_tree(tree t) {
    if (t == NULL) return;
    
    // Рекурсивно очищаем связанные структуры
    if (t->psubcmd) clear_tree(t->psubcmd);
    if (t->pipe) clear_tree(t->pipe);
    if (t->next) clear_tree(t->next);
    
    // Освобождаем аргументы
    if (t->argv) {
        for (int i = 0; t->argv[i] != NULL; i++) {
            free(t->argv[i]);
        }
        free(t->argv);
    }
    
    // Освобождаем файлы
    if (t->infile) free(t->infile);
    if (t->outfile) free(t->outfile);
    
    // Освобождаем сам узел
    free(t);
}