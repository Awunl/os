#include <stdio.h>


double str2double( char str[] )
{
    double answer = 0;
    double drob = 0;
    double mnozh = 10;
    int exp = 0;
    int exp_sign = 1;
    int sign = 1;
    int i = 0;
    
    if (str[i] == '-')
    {
	sign = -1;
	i++;
    }
    else if (str[i] == '+')
    {
	i++;
    }
    
    while (('0' <= str[i]) && (str[i] <= '9'))
    {
	answer = answer*10 + (str[i] - '0');
	i++;
    }
    if (str[i] == '.')
    {
	i++;
	while (('0' <= str[i]) && (str[i] <= '9'))
	{
	    drob = drob + (str[i] - '0')/mnozh;
	    mnozh *=10;
	    i++;
	}
    }

    answer += drob;

    if (str[i] == 'e' || str[i] == 'E')
    {
	i++;
	if (str[i] == '-') 
	{
            exp_sign = -1;
            i++;
        } 
	else if (str[i] == '+') 
	{
            i++;
        }
	while (('0' <= str[i]) && (str[i] <= '9'))
        {
  	    exp = exp*10 + (str[i] - '0');
	    i++;
	}
	double exp_mn = 1;
	for (int j = 0; j < exp; j++)
	{
	    if (exp_sign == 1)
	    {
		exp_mn *= 10;
	    }
	    else
	    {
		exp_mn /= 10;
	    }
	}
	answer *= exp_mn;
    }
    
    return sign*answer;
}



int main()
{
    char mas[100];
    double number;
    while (scanf("%s", mas) == 1)
	{
  	    number = str2double(mas);
	    printf("%.10g\n", number);
	}
    return 0;
}
