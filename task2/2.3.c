#include <stdio.h>
#include <time.h>


long long iterative(int i) 
{
    if (i == 0) return 0;
    if (i == 1) return 1;
    
    long long a = 0, b = 1, temp;
    for (int j = 2; j <= i; j++) 
    {
        temp = a + b;
        a = b;
        b = temp;
    }
    return b;
}

long long recursive(int i) 
{
    if (i == 0) return 0;
    if (i == 1) return 1;
    return recursive(i - 2) + recursive(i - 1);
}

int main() 
{
    int i;
    clock_t start, end;
    double iter_t, rec_t;
        
    while (scanf("%d", &i) == 1) {
        
	start = clock();
        long long iter = iterative(i);
	end = clock();
	iter_t = ((double)(end - start)) / CLOCKS_PER_SEC;
	
	start = clock();
	long long rec = recursive(i);
        end = clock();
	rec_t = ((double)(end - start)) / CLOCKS_PER_SEC;
        printf("%d %lld\n", i, iter);
        printf("%d %lld\n", i, rec);
	printf("итеративно=%.6fс, рекурсивно=%.6fс\n", iter_t, rec_t);
    }
    return 0;
}
