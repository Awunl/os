/* Вычисление значения выражения, содержащего цифры '0'-'9', знаки
* операций '+', '-', '*', '/', '^' и скобки '(', ')''. ( Предполагается, что коды цифр
* упорядочены по возрастанию цифр и справедливо равенство '9'-'0'==9 как,
* например, в ASCII кодировке)
*/
#include <stdio.h>
#include <setjmp.h>

jmp_buf begin; /* точка начала диалога с пользователем */

char curlex; /* текущая лексема */

void getlex(void); /* выделяет из входного потока очередную лексему */
int expr(void);    /* распознает выражение и вычисляет его значение */
int term(void);    /* распознает слагаемое и вычисляет его значение */
int factor(void);  /* распознает множитель и вычисляет его значение */
int power(void);   /* распознает степень и вычисляет ее значение */
void error(void);  /* сообщает об ошибке в выражении и передает управление
                    в начало функции main (точка begin) */

int main()
{
    int result;
    setjmp(begin);
    printf("==>");
    getlex();
    result = expr();
    if (curlex != '\n') error();
    printf("\n%d\n", result);
    return 0;
}

void getlex()
{
    while ((curlex = getchar()) == ' ');
}

void error(void)
{
    printf("\nОШИБКА!\n");
    while (getchar() != '\n');
    longjmp(begin, 1);
}

int expr()
{
    int e = term();
    while (curlex == '+' || curlex == '-')
    {
        char op = curlex;
        getlex();
        if (op == '+')
            e += term();
        else
            e -= term();
    }
    return e;
}

int term()
{
    int a = factor();
    while (curlex == '*' || curlex == '/')
    {
        char op = curlex;
        getlex();
        int b = factor();
        if (op == '*')
            a *= b;
        else {
            if (b == 0) error(); 
            a /= b;
        }
    }
    return a;
}

int factor()
{
    int base = power();
    while (curlex == '^')
    {
        getlex();
        int exp = power();
        if (exp < 0) error(); 
        int result = 1;
        for (int i = 0; i < exp; i++) {
            result *= base;
        }
        base = result;
    }
    return base;
}

int power()
{
    int m;
    switch (curlex)
    {
    case '0': case '1': case '2': case '3': case '4': case '5':
    case '6': case '7': case '8': case '9':
        m = curlex - '0';
        break;
    case '(':
        getlex();
        m = expr();
        if (curlex == ')') break;
        /* иначе ошибка - нет закрывающей скобки */
    default:
        error();
    }
    getlex();
    return m;
}





























