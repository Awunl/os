#include <stdio.h>

int main() 
{
    double e, x;
    scanf("%lf", &e);
    while (scanf("%lf", &x) == 1) 
    {
        if (x == 0) 
	{
            printf("%.10g\n", x);
            continue;
        }
        double xi, xi1, diff;
        xi = 1;
	xi1 = 0.5*(1 + x); 
        while ((xi1 - xi) >= 0 ? (xi1 - xi) >= e: (xi - xi1) >= e)  
	{
            xi = xi1;
            xi1 = 0.5*(xi + x/xi);
        } 
        printf("%.10g\n", xi1);
    }
    return 0;
}

