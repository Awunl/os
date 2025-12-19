#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include "list.h"
#include "tree.h"
#include "exec.h"

#define BUFF_SIZE 1024

jmp_buf begin;
List *plst;
tree cmd_tree;
int exit_val = 0;

void handler(int s) {
    (void)s;
    signal(SIGINT, handler);
    printf("\n");
}

int main() {
    char buffer[BUFF_SIZE];
    List lst;
    
    signal(SIGINT, handler);
    
    // Устанавливаем точку возврата для обработки ошибок
    if (setjmp(begin) != 0) {
        printf("\nОшибка синтаксиса. Начинаем заново...\n");
        if (cmd_tree) clear_tree(cmd_tree);
        cmd_tree = NULL;
        free_list(&lst);
    }
    
    while (1) {
        printf("myshell> ");
        fflush(stdout);
        
        if (!fgets(buffer, BUFF_SIZE, stdin)) {
            if (feof(stdin)) {
                printf("\nВыход\n");
                cleanup_background_processes(bckgrnd);
                exit(0);
            }
            continue;
        }
        
        // Убираем символ новой строки
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }
        
        // Игнорируем пустые строки
        if (strlen(buffer) == 0) {
            continue;
        }
        
        // Выход из оболочки
        if (strcmp(buffer, "exit") == 0) {
            printf("До свидания!\n");
            cleanup_background_processes(bckgrnd);
            exit(0);
        }
        
        // Построение списка слов из строки
        build_list(&lst, buffer);
        
        // Замена переменных окружения
        change_lst(&lst);
        
        // Построение дерева команд
        cmd_tree = build_tree(&lst);
        
        if (cmd_tree) {
            // Выполнение команды
            execute_tree(cmd_tree);
            
            // Очистка дерева
            clear_tree(cmd_tree);
            cmd_tree = NULL;
        }
        
        // Очистка списка для следующей команды
        free_list(&lst);
    }
    
    return 0;
}
