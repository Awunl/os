#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <stdlib.h>

// Структура для узла списка слов
typedef struct list_node {
    char *word;
    struct list_node *next;
} list_node;

// Основная структура списка
typedef struct {
    list_node *head;
    list_node *tail;
    int size;
} List;

// Инициализация списка
void init_list(List *list);

// Добавление слова в список
void add_word(List *list, const char *word);

// Печать списка
void print_list(List *list);

// Очистка списка
void free_list(List *list);

// Построение списка из строки (основная функция)
void build_list(List *list, char *str);

// Замена переменных окружения в списке
void change_lst(List *list);

#endif
