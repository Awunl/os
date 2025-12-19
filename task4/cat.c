#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int n = 0, num = 1;
    
    if (argc > 1 && strcmp(argv[1], "-n") == 0) {
        n = 1;
    }
    
    
    if (argc == 1 || (argc == 2 && n)) {
        char line[1024];
        while (fgets(line, sizeof(line), stdin)) {
            if (n) printf("%6d  %s", num++, line);
            else printf("%s", line);
        }
        return 0;
    }
    
    for (int i = 1 + n; i < argc; i++) {
        FILE *f = fopen(argv[i], "r");
        if (!f) {
            fprintf(stderr, "cat: %s: нет такого файла\n", argv[i]);
            continue;
        }
        
        char line[1024];
        while (fgets(line, sizeof(line), f)) {
            if (n) printf("%6d  %s", num++, line);
            else printf("%s", line);
        }
        
        fclose(f);
    }
    
    return 0;
}
