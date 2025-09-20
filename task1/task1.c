#include <stdio.h>

int main() {
    printf("Размеры типов данных в байтах:\n");
    printf("short: %zu байта\n", sizeof(short));
    printf("int: %zu байта\n", sizeof(int));
    printf("long: %zu байт\n", sizeof(long));
    printf("float: %zu байта\n", sizeof(float));
    printf("double: %zu байт\n", sizeof(double));
    printf("long double: %zu байт\n", sizeof(long double));
    
    return 0;
}
