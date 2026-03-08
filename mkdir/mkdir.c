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
    char *endptr;
    pflag = 0;
    opterr = 0;
    while ((ch = getopt(argc, argv, "pm:")) != -1) {
        switch(ch) {
            case 'p':
                pflag = 1;
                break;
            case 'm':
                errno = 0;
                mode = strtol(optarg, &endptr, 8);

                if (optarg == endptr || *endptr != '\0') {
                    fprintf(stderr, "invalid file mode: %s\n", optarg);
                    exit(EXIT_FAILURE);
                }

                if (errno == EINVAL || errno == ERANGE) {
                    usage();
                }
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
        if (pflag) {
            errors |= add_path(*argv, mode);
        }
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

    char *pathname = malloc(1 + strlen(path) * sizeof(char));
    strcpy(pathname, path);

    char *p = strrchr(path, '\0');
    if (*(p - 1) != '\0') {
        pathname = realloc(pathname, strlen(path) + 2);
        pathname[strlen(path)] = '/';
        pathname[strlen(path) + 1] = '\0';

    }

    for (p = pathname; *p != '\0'; p++) {
        if (p[0] == '/') {
            p[0] = '\0';
            errno = 0;
            if (mkdir(pathname, mode) == -1) {
                if (errno == EEXIST) {
                    p[0] = '/';
                    continue;
                } else {
                    warn("%s", pathname);
                    free(pathname);
                    return 1;
                }
            }
            p[0] = '/';
        }
    }
    free(pathname);
    return 0;
}

void usage(void) {
    fprintf(stderr, "usage : mkdir [-p] [-m mode] directory ...\n");
    exit(EXIT_FAILURE);
}