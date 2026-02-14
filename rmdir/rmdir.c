#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

void usage(void);
int remove_dir(char *path);

int main(int argc, char **argv) {
    int ch, pflag, status;

    opterr = 0;
    pflag = 0;
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
    
    status = 0;
    for (; *argv != NULL; argv++) {
        if (unlinkat(AT_FDCWD, *argv, AT_REMOVEDIR) == -1) {
            warn("%s", *argv);
            status = 1;
        } else if (pflag) {
            status |= remove_dir(*argv);
        }
    }
    return status;
}

int remove_dir(char *path) {
    int status = 0;
    errno = 0;

    char *p = strchr(path, '\0');

    while (--p > path && *p == '/')
        continue;

    for (; p > path; p--) {
        if (p[0] == '/' && p[-1] != '/') {
            *p = '\0';
            if (unlinkat(AT_FDCWD, path, AT_REMOVEDIR) == -1) {
                warn("%s", path);
                return 1;
            }
        }
    }
    return status;
}

void usage(void) {
    fprintf(stderr, "usage: rmdir [-p] directory ...\n");
    exit(EXIT_FAILURE);
}