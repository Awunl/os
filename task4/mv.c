#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Использование: %s <файл1> <файл2>\n", argv[0]);
        return 1;
    }

    const char *src = argv[1];
    const char *dst = argv[2];

    struct stat dst_stat;
    if (stat(dst, &dst_stat) == 0 && S_ISDIR(dst_stat.st_mode)) {
        char *src_name = strrchr(src, '/');
        if (src_name == NULL) {
            src_name = (char*)src;
        } else {
            src_name++;
        }
        char new_dst[1024];
        snprintf(new_dst, sizeof(new_dst), "%s/%s", dst, src_name);
        dst = new_dst;
    }

    if (rename(src, dst) != 0) {
        perror("mv");
        return 1;
    }

    return 0;
}
