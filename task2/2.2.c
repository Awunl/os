#include <stdio.h>

int main() 
{
    double x, res, pr, intg, a;
    int n;

    scanf("%d", &n);
    scanf("%lf", &x);
    
    pr = 0;
    intg = 0;
    res = 0;

    n++;

    while (scanf("%lf", &a) == 1) 
    {
	pr = pr*x + res;
        res = res*x + a;
	intg = intg*x + a/n;
	n--;
    }
    intg *= x;

    printf("%.10g\n", res);
    printf("%.10g\n", pr);
    printf("%.10g\n", intg);
    
    return 0;
}
