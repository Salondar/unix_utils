#include <sys/stat.h>
#include <errno.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void usage(void);
int add_path(char *path, __mode_t mode);

int main(int argc, char **argv) {
    __mode_t mode = S_IRWXU | S_IRWXG | S_IRWXO;
    int ch, errors, pflag;

    pflag = 0;
    opterr = 0;
    while ((ch = getopt(argc, argv, "p")) != -1) {
        switch(ch) {
            case 'p':
                pflag = 1;
                break;
            default:
                usage();               
        }
    }
    argc -= optind;
    argv += optind;

    if (argc == 0) {
        usage();
    }

    errors = 0;
    for (; *argv != NULL; argv++) {
        if (pflag)
            errors |= add_path(*argv, mode);
        else {
            if (mkdir(*argv, mode) == -1) {
                warn("%s", *argv);
                errors = 1;
            }
        }
    }
    
    return errors;
}

int add_path(char *path, __mode_t mode) {

    char *p = strrchr(path, '\0');
    int path_not_end_with_slash = 0;
    if (*(--p) != '/') {
        path_not_end_with_slash = 1;
    }
    for (p = path; *p != '\0'; p++) {
        if (p[0] == '/') {
            p[0] = '\0';
            if (mkdir(path, mode) == -1) {
                warn("%s", path);
                return 1;
            }
            p[0] = '/';
        }
    }

    if (path_not_end_with_slash) {
        if (mkdir(path, mode) == -1) {
            warn("%s", path);
            return 1;
        }
    }

    return 0;
}

void usage(void) {
    fprintf(stderr, "usage : mkdir [-p] [-m mode] directory ...\n");
    exit(EXIT_FAILURE);
}