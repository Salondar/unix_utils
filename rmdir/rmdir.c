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
    int ch, retval, pflag = 0, status = 0;
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

    if (pflag) {
        for (; *argv != NULL; argv++) {
            while ((chr = strrchr(*argv, '/')) != NULL) {
                retval = remove_dir(*argv);
                if (retval == -1) {
                    warn("%s", *argv);
                    status += 1;
                }
                *chr = '\0';
            }
            retval = remove_dir(*argv);
            if (retval == -1) {
                warn("%s", *argv);
                status += 1;
            }
        }
    } else {
        for (; *argv != NULL; argv++) {
            retval = remove_dir(*argv);
            if (retval == -1) {
                warn("%s", *argv);
                status += 1;
            }
        }
    }
    return status;
}

int remove_dir(char *pathname) {
    errno = 0;
    return unlinkat(AT_FDCWD, pathname, AT_REMOVEDIR);
}

void usage(void) {
    fprintf(stderr, "usage: rmdir [-p] directory...\n");
    exit(EXIT_FAILURE);
}