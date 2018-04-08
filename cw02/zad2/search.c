//
// Created by Dawid Drozd on 19.03.2018.
//

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <ftw.h>

char *PROGRAM_NAME;
const int PATH_MAX = 128;
struct tm TIME_ARGS;
int HOW;

void print_usage(FILE *stream, int exit_code) {
    fprintf(stream, "Usage: %s [path_to_directory] [< = >] [YYYY-mm-DD]\n", PROGRAM_NAME);
    exit(exit_code);
}

void check_if_correct(bool is_ok, const char *message) {
    if (!is_ok) {
        fprintf(stderr, "Error: %s: %s\n", strerror(errno), message);
        exit(errno);
    }
}

void check_if_exists(const char *dirpath) {
    struct stat sb;

    if (!(stat(dirpath, &sb) == 0 && S_ISDIR(sb.st_mode))) {
        fprintf(stderr, "Can't find given directory: %s\n", dirpath);
        exit(1);
    }
}

char *convert_time(time_t time, char *buff) {
    struct tm *timeinfo;
    timeinfo = localtime(&time);
    strftime(buff, 20, "%F", timeinfo);
    return buff;
}

struct tm convert_to_tm(char *time_details) {
    struct tm tm;
    strptime(time_details, "%F", &tm);
    return tm;
}

int compare_tm(struct tm tm1, struct tm tm2) {
    if (tm1.tm_year < tm2.tm_year) {
        return -1;
    } else if (tm1.tm_year > tm2.tm_year) {
        return 1;
    }

    if (tm1.tm_mon < tm2.tm_mon) {
        return -1;
    } else if (tm1.tm_mon > tm2.tm_mon) {
        return 1;
    } else if (tm1.tm_mday < tm2.tm_mday) {
        return -1;
    } else if (tm1.tm_mday > tm2.tm_mday) {
        return 1;
    } else
        return 0;
}

int fn(const char *path, const struct stat *buf, int flag, struct FTW *ftw) {
    if (flag == FTW_F) {
        struct tm *time_file = localtime(&buf->st_mtime);
        if (compare_tm(*time_file, TIME_ARGS) == HOW) {
            char bits_buff[20];
            char time_buff[20];
            mode_t bits = buf->st_mode;
            strmode(bits, bits_buff);
            convert_time(buf->st_mtime, time_buff);
            printf("%s %-10zd %-5s %-5s\n", bits_buff, buf->st_size, time_buff, path);
        }
    }

    return 0;
}

void traverse(char *absolute_path) {
    DIR *dir = opendir(absolute_path);

    if (dir == NULL) {
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        char local_path[1024];
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            snprintf(local_path, sizeof(local_path), "%s/%s", absolute_path, entry->d_name);
            traverse(local_path);
        } else if (entry->d_type == DT_REG) {
            struct stat *buf;
            buf = malloc(sizeof(struct stat));
            snprintf(local_path, sizeof(local_path), "%s/%s", absolute_path, entry->d_name);
            int file_stat = stat(local_path, buf);
            struct tm *time_file = localtime(&buf->st_mtime);
            if (file_stat == 0 && compare_tm(*time_file, TIME_ARGS) == HOW) {
                char bits_buff[20];
                char time_buff[20];
                mode_t bits = buf->st_mode;
                strmode(bits, bits_buff);
                convert_time(buf->st_mtime, time_buff);
                printf("%s %-10zd %-5s %-5s\n", bits_buff, buf->st_size, time_buff, local_path);
            } /*else {
                printf("%s\n", strerror(errno));
            }*/
            free(buf);
        }
    }
    closedir(dir);
}

int traverse_nftw(const char *absolute_path) {
    int result;

    if (absolute_path == NULL || *absolute_path == '\0')
        return errno;

    result = nftw(absolute_path, fn, 1024, FTW_PHYS);

    if (result >= 0)
        errno = result;

    return errno;
}

int main(int argc, char *argsv[]) {
    PROGRAM_NAME = argsv[0];
    if (argc < 4) {
        fprintf(stderr, "Program should have 3 or 4 arguments! Actual: %d\n", argc - 1);
        print_usage(stderr, 1);
    }

    const char *dirpath = argsv[1];
    const char *which = argsv[2];
    TIME_ARGS = convert_to_tm(argsv[3]);

    if (strcmp(which, "<") == 0) {
        HOW = -1;
    } else if (strcmp(which, "=") == 0) {
        HOW = 0;
    } else if (strcmp(which, ">") == 0) {
        HOW = 1;
    } else {
        fprintf(stderr, "Only < = > is accepted as second argument!\n");
        print_usage(stderr, 1);
    }

    check_if_exists(dirpath);

    char absolute_path[PATH_MAX + 1];
    char *ptr;
    ptr = realpath(dirpath, absolute_path);
    check_if_correct(ptr != NULL, "Can't find directory.");

    int result = 0;

    if (argc == 5 && strcmp(argsv[4], "-nftw") == 0) {
        result = traverse_nftw(absolute_path);
        check_if_correct(result == 0, "Something is wrong.");
    } else {
        traverse(absolute_path);
    }

    return 0;
}