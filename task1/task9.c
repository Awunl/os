#include <stdio.h>

int main() {
    
    int a = 3, b = 3, c = 4;
    
    printf("a = %d, b = %d, c = %d\n", a, b, c);
    printf("\n");
    
    /* Тест 1: */
    printf("1 Проверка: a < b == c > a \n");
    printf("   Хотим: (a < b) == (c > a) \n");
    printf("   Получилось: %d\n", a < b == c > a);
    printf("\n");
    
    /* Тест 2: */
    printf("2: a != b > c \n");
    printf("   Хотим: a != (b > c) \n");
    printf("   Получилось: %d\n", a != b > c);
    printf("\n");
   
    /* Тест 3: */
    printf("3: общая проверка \n");
    printf("   (a < b) == (c > a): %d\n", (a < b) == (c > a));
    printf("   a < (b == c) > a: %d\n", a < (b == c) > a);
    printf("   a == b < c: %d\n", a == b < c);
    printf("   (a == b) < c: %d\n", (a == b) < c);
    printf("   a == (b < c): %d\n", a == (b < c));
    printf("\n");
    
    printf("Операции отношения <, >, <=, >= имеют более высокий приоритет, чем == и != \n");
    printf("   a == b != c: %d/n", (a == b != c));
    
    return 0;
}
