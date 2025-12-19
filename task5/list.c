#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "list.h"

// Если strdup не определена в стандартной библиотеке
#ifndef strdup
char *strdup(const char *s) {
    size_t len = strlen(s) + 1;
    char *dup = malloc(len);
    if (dup) {
        memcpy(dup, s, len);
    }
    return dup;
}
#endif

static int is_special(char c) {
    return c == ';' || c == '<' || c == '>' || c == '|' || c == '&' ||  
           c == '(' || c == ')' || c == ':' || c == ' ' || c == '\t'||  
           c == '\n'||  c == '\0';
}

// Инициализация списка
void init_list(List *list) {
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

// Добавление слова в список
void add_word(List *list, const char *word) {
    if (!word || strlen(word) == 0) return;
    
    list_node *node = malloc(sizeof(list_node));
    if (!node) return;
    
    node->word = strdup(word);
    node->next = NULL;
    
    if (!list->head) {
        list->head = node;
        list->tail = node;
    } else {
        list->tail->next = node;
        list->tail = node;
    }
    list->size++;
}

// Печать списка
void print_list(List *list) {
    list_node *current = list->head;
    printf("List (%d words):\n", list->size);
    while (current) {
        printf("  '%s'\n", current->word);
        current = current->next;
    }
}

// Очистка списка
void free_list(List *list) {
    list_node *current = list->head;
    while (current) {
        list_node *next = current->next;
        free(current->word);
        free(current);
        current = next;
    }
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

// Замена переменных окружения ($HOME, $USER и т.д.)
void change_lst(List *list) {
    list_node *current = list->head;
    while (current) {
        char *word = current->word;
        if (word[0] == '$') {
            char *var_name = word + 1;
            char *env_value = getenv(var_name);
            if (env_value) {
                free(current->word);
                current->word = strdup(env_value);
            }
        }
        current = current->next;
    }
}

// Основная функция построения списка из строки
void build_list(List *list, char *str) {
    init_list(list);
    
    int i = 0;
    int len = strlen(str);
    int in_single_quotes = 0;
    int in_double_quotes = 0;
    char buffer[256];
    int buf_pos = 0;
    
    while (i < len) {
        char c = str[i];
        
        // Обработка кавычек
        if (c == '\'' && !in_double_quotes) {
            in_single_quotes = !in_single_quotes;
            i++;
            continue;
        } else if (c == '"' && !in_single_quotes) {
            in_double_quotes = !in_double_quotes;
            i++;
            continue;
        }
        
        // Если пробел или табуляция вне кавычек
        if ((c == ' ' || c == '\t') && !in_single_quotes && !in_double_quotes) {
            if (buf_pos > 0) {
                buffer[buf_pos] = '\0';
                add_word(list, buffer);
                buf_pos = 0;
            }
            i++;
            continue;
        }
        
        // Если специальный символ вне кавычек
        if (is_special(c) && !in_single_quotes && !in_double_quotes) {
            if (buf_pos > 0) {
                buffer[buf_pos] = '\0';
                add_word(list, buffer);
                buf_pos = 0;
            }
            // Добавляем специальный символ как отдельное слово
            if (c != '\0' && c != '\n') {
                buffer[0] = c;
                buffer[1] = '\0';
                add_word(list, buffer);
            }
            i++;
            continue;
        }
        
        // Обычный символ
        buffer[buf_pos++] = c;
        i++;
        
        if (buf_pos >= 255) {
            buffer[255] = '\0';
            add_word(list, buffer);
            buf_pos = 0;
        }
    }
    
    // Добавляем последнее слово, если есть
    if (buf_pos > 0) {
        buffer[buf_pos] = '\0';
        add_word(list, buffer);
    }
}
