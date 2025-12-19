#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int cmp(const char *filename1, const char *filename2) {
    FILE *file1 = fopen(filename1, "r");
    FILE *file2 = fopen(filename2, "r");
    if (!file1) {
        perror(filename1); 
	return -1;         
    }
    if (!file2) {
        perror(filename2);  
        fclose(file1);  
        return -1;        
    }
    
    int line = 1;     
    int column = 1;      
    int ch1, ch2;       
    while (1) {
        ch1 = fgetc(file1);  
	ch2 = fgetc(file2);         
        if (ch1 == EOF && ch2 == EOF) {
            printf("Файлы идентичны\n");
            fclose(file1);
            fclose(file2);
            return 0;  
	}
        
       
	if (ch1 == EOF || ch2 == EOF) {
            printf("%s отличается от %s: строка %d, символ %d (разная длина файлов)\n", filename1, filename2, line, column);
            fclose(file1);
            fclose(file2);
            return 1; 
	}
        if (ch1 != ch2) {
            printf("%s отличается от %s: строка %d, символ %d\n",
                   filename1, filename2, line, column);
            printf("В первом файле: '%c', во втором: '%c'\n", ch1, ch2);
            fclose(file1);
            fclose(file2);
            return 1;
        }
        if (ch1 == '\n') {  
            line++;          
            column = 1;      
        } else {            
            column++;       
        }
    }
    
    fclose(file1);
    fclose(file2);
    return 0;
}

int main(int argc, char *argv[]) {
       if (argc != 3) {
        printf("Использование: %s <файл1> <файл2>\n", argv[0]);
        return 1;
    }
    return cmp(argv[1], argv[2]);
}
