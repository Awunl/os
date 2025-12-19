#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifndef MAJOR
#define MINOR_BITS 8
#define MAJOR(dev) ((unsigned) dev >> MINOR_BITS)
#define MINOR(dev) (dev & ((1 << MINOR_BITS) - 1))
#endif


int show_long = 0;
int show_group_only = 0; 
int recursive = 0;


void display_file_type(int st_mode) 
{                                   
    switch (st_mode & S_IFMT) {
        case S_IFDIR:  putchar('d'); return;
        case S_IFCHR:  putchar('c'); return;
        case S_IFBLK:  putchar('b'); return;
        case S_IFREG:  putchar('-'); return;
        case S_IFLNK:  putchar('l'); return;
        case S_IFSOCK: putchar('s'); return;
    }
}


void display_permission(int st_mode) {
    static const char xtbl[10] = "rwxrwxrwx";
    char amode[10];
    int i, j;

    for (i = 0, j = (1 << 8); i < 9; i++, j >>= 1)
        amode[i] = (st_mode & j) ? xtbl[i] : '-';
    if (st_mode & S_ISUID) amode[2] = (st_mode & S_IXUSR) ? 's' : 'S';
    if (st_mode & S_ISGID) amode[5] = (st_mode & S_IXGRP) ? 's' : 'S';
    if (st_mode & S_ISVTX) amode[8] = (st_mode & S_IXOTH) ? 't' : 'T';  
    amode[9] = '\0';
    printf("%s ", amode);
}


void get_link_target(const char *path, char *buf, size_t buf_size) {
    ssize_t len = readlink(path, buf, buf_size - 1);
    if (len != -1) {
        buf[len] = '\0';
    } else {
        buf[0] = '\0';
    }
}

void format_time(time_t mtime, char *time_buf, size_t buf_size) {
    struct tm *timeinfo = localtime(&mtime);
    time_t now = time(NULL);
    
    if (now - mtime > 15778476) { 
        strftime(time_buf, buf_size, "%b %d  %Y", timeinfo);
    } else {
        strftime(time_buf, buf_size, "%b %d %H:%M", timeinfo);
    }
}

long print_file_info(const char *path, const char *name) {
    struct stat statv;
    struct passwd *pw_d;
    struct group *gr_d;
    long blocks = 0;
    
    if (lstat(path, &statv)) {
        perror(path);
        return 0;
    }
    
    blocks = statv.st_blocks;
    
    if (!show_long && !show_group_only) {
        printf("%s\n", name);
        return blocks;
    }
    
    display_file_type(statv.st_mode);
    display_permission(statv.st_mode);
    
    printf("%2ld ", (long)statv.st_nlink);
    
    pw_d = getpwuid(statv.st_uid);
    printf("%-8s ", pw_d ? pw_d->pw_name : "unknown");
    
    if (show_long && !show_group_only) {
        gr_d = getgrgid(statv.st_gid);
        printf("%-8s ", gr_d ? gr_d->gr_name : "unknown");
    }
    
    if ((statv.st_mode & S_IFMT) == S_IFCHR || 
        (statv.st_mode & S_IFMT) == S_IFBLK) {
        printf("%3d, %3d ", MAJOR(statv.st_rdev), MINOR(statv.st_rdev));
    } else {
        printf("%8ld ", (long)statv.st_size);
    }
    
    char time_buf[64];
    format_time(statv.st_mtime, time_buf, sizeof(time_buf));
    printf("%s ", time_buf);
    
    printf("%s", name);
    
    if ((statv.st_mode & S_IFMT) == S_IFLNK) {
        char link_buf[1024];
        get_link_target(path, link_buf, sizeof(link_buf));
        if (link_buf[0] != '\0') {
            printf(" -> %s", link_buf);
        }
    }
    
    printf("\n");
    return blocks;
}

typedef struct {
    char name[256];
    time_t mtime;
} file_entry;

int compare_by_date(const void *a, const void *b) {
    const file_entry *fa = (const file_entry *)a;
    const file_entry *fb = (const file_entry *)b;
    return (fb->mtime - fa->mtime);
}

void list_directory(const char *dirname) {
    DIR *dir;
    struct dirent *entry;
    char full_path[1024];
    long total_blocks = 0;
    int has_entries = 0;
    int count = 0;
    file_entry *entries = NULL;
    
    if (recursive) {
        printf("\n%s:\n", dirname);
    }
    
    dir = opendir(dirname);
    if (!dir) {
        perror(dirname);
        return;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        
        if (show_long) {
            snprintf(full_path, sizeof(full_path), "%s/%s", dirname, entry->d_name);
            struct stat statv;
            if (lstat(full_path, &statv) == 0) {
                total_blocks += statv.st_blocks;
                has_entries = 1;
            }
        }
        
        count++;
    }
    
    if (show_long && has_entries) {
        printf("итого %ld\n", total_blocks / 2); 
    }
    
    entries = malloc(count * sizeof(file_entry));
    rewinddir(dir);
    
    int index = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        
        snprintf(full_path, sizeof(full_path), "%s/%s", dirname, entry->d_name);
        struct stat statv;
        if (lstat(full_path, &statv) == 0) {
            strncpy(entries[index].name, entry->d_name, sizeof(entries[index].name) - 1);
            entries[index].mtime = statv.st_mtime;
            index++;
        }
    }
    
    if (show_long) {
        qsort(entries, count, sizeof(file_entry), compare_by_date);
    }
    
    for (int i = 0; i < count; i++) {
        snprintf(full_path, sizeof(full_path), "%s/%s", dirname, entries[i].name);
        print_file_info(full_path, entries[i].name);
        
        if (recursive) {
            struct stat statv;
            if (lstat(full_path, &statv) == 0 && S_ISDIR(statv.st_mode)) {
                list_directory(full_path);
            }
        }
    }
    
    free(entries);
    closedir(dir);
}

void parse_flags(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            char *flag = argv[i] + 1;
            while (*flag) {
                switch (*flag) {
                    case 'l': 
                        show_long = 1;
                        break;
                    case 'g': 
                        show_group_only = 1;
                        break;
                    case 'R': 
                        recursive = 1; 
                        break;
                }
                flag++;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    parse_flags(argc, argv);
    list_directory(".");
    return 0;
}
