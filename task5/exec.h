#ifndef EXEC_H
#define EXEC_H

#include "tree.h"

// Структура для отслеживания фоновых процессов
typedef struct background_process {
    pid_t pid;
    struct background_process *next;
} background_process;

// Структура для списка фоновых процессов
typedef struct {
    background_process *head;
    background_process *tail;
    int count;
} backgrndList;

// Функции модуля
int execute_tree(tree t);
void execute_simple_command(tree t);
void execute_pipeline(tree t);
void execute_subshell(tree t);
void wait_for_background(backgrndList *list);
void add_background_process(backgrndList *list, pid_t pid);
void cleanup_background_processes(backgrndList *list);

// Объявляем переменную как extern (определена в exec.c)
extern backgrndList *bckgrnd;

#endif
