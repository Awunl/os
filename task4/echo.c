#include <stdio.h>
#include <stdlib.h>
#include <string.h> 


int main(int argc, char *argv[]) {
    int no_newline = 0;
    int enable_escapes = 0;
    
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (strcmp(argv[i], "-n") == 0) {
                no_newline = 1;
            } else if (strcmp(argv[i], "-e") == 0) {
                enable_escapes = 1;
            }
        }
    }
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') continue;
        if (enable_escapes) {
            char *p = argv[i];
            while (*p) {
                if (*p == '\\' && *(p+1)) {
                    switch (*(p+1)) {
                        case 'n': putchar('\n'); break;
                        case 't': putchar('\t'); break;
                        case '\\': putchar('\\'); break;
                        default: putchar(*p); putchar(*(p+1)); break;
                    }
                    p += 2;
                } else {
                    putchar(*p);
                    p++;
                }
            }
        } 
	else printf("%s", argv[i]);

        if (i < argc - 1) putchar(' ');
    }
    
    if (!no_newline) putchar('\n');
    return 0;
}



