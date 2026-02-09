#include <dirent.h>
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
    int ch, pflag = 0, status = 0;
    char *chr;

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
    
    if (pflag) {
        for (; *argv != NULL; argv++) {
            size_t len = strlen(*argv);
            if ((*argv)[len - 1] == '/') {
                (*argv)[len - 1] = '\0';
            }
            while ((chr = strrchr(*argv, '/')) != NULL) {
                status += remove_dir(*argv);
                *chr = '\0';
            }
            status += remove_dir(*argv);
        }
    } else {
        for (; *argv != NULL; argv++) {
            status += remove_dir(*argv);
        }
    }
    return status;
}

int remove_dir(char *pathname) {
    int status = 0;
    errno = 0;
    if (unlinkat(AT_FDCWD, pathname, AT_REMOVEDIR) == -1) {
        warn("%s", pathname);
        status = 1;
    }

    return status;
}

void usage(void) {
    fprintf(stderr, "usage: rmdir [-p] directory ...\n");
    exit(EXIT_FAILURE);
}