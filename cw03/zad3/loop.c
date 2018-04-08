#include <stdio.h>
#include <stdlib.h>

unsigned int to_int(char *string) {
    char *errptr;
    unsigned int number = (unsigned int) strtol(string, &errptr, 10);
    if (*errptr != '\0') {
        printf("Can't parse argument: Only numbers are expected.\n");
        exit(EXIT_FAILURE);
    }
    return number;
}

int main(int argc, char *argv[]) {

    // ALLOCATE ARGV[1] SECONDS
    unsigned int limit = to_int(argv[1]);
    char **data = calloc(limit, sizeof(char *));
    size_t bytes = (1024 * 1024);

    for (int how = 0; how < limit; how++) {
        data[how] = (char *) malloc(bytes);
        for(int i=0; i<bytes; i++) data[how][i] = (char)(70+rand()%10);
        if (data[how] == NULL) {
            printf("Can't allocate more memory!");
        } else {
            printf("Memory allocated: %dMB\n", how + 1);
        }
    }

    for (int how = 0; how < limit; how++) {
        free(data[how]);
    }

    free(data);
    return 0;
}
