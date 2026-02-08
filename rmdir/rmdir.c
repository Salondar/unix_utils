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
            size_t len = strlen(*argv);
            char dst[len + 2];
            char *path;
            if (*argv[0] != '/') {
                dst[0] = '/';
                dst[1] = '\0';
                path = strcat(dst, *argv);
                len  = strlen(path);
            } else {
                path = *argv;
            }
            
            if (path[len - 1] == '/') {
                path[len - 1] = '\0';
            }
            while ((chr = strrchr(path, '/')) != NULL) {
                retval = remove_dir(path);
                if (retval == -1) {
                    warn("%s", path);
                    status += 1;
                }
                *chr = '\0';
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