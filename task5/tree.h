#ifndef TREE_H
#define TREE_H

#include "list.h"
#include <setjmp.h>

// Типы связей между командами
enum type_of_next {
    NXT,    // обычная последовательность (; или &)
    AND,    // логическое И (&&)
    OR      // логическое ИЛИ ()
};

// Структура узла дерева команд
struct cmd_inf {
    char **argv;               // список из имени команды и аргументов
    char *infile;              // переназначенный файл стандартного ввода
    char *outfile;             // переназначенный файл стандартного вывода
    int append;                // флаг дополнения файла (>>)
    int backgrnd;              // =1, если команда в фоновом режиме
    struct cmd_inf *psubcmd;   // команды для запуска в дочернем shell (в скобках)
    struct cmd_inf *pipe;      // следующая команда после "|"
    struct cmd_inf *next;      // следующая команда после ";", "&&", ""
    enum type_of_next type;    // тип связи со следующей командой
};

typedef struct cmd_inf *tree;
typedef struct cmd_inf node;

// Функции модуля
void print_tree(tree t, int shift);
tree build_tree(List *list);
void clear_tree(tree t);
void error(const char *msg);

// Внешняя переменная для обработки ошибок
extern jmp_buf begin;

#endif
