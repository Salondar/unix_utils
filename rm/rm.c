#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <err.h>
#include <errno.h>
#include <getopt.h>

void usage(void);

int main(int argc, char **argv) {
    int ch, vflag, dflag, iflag, errors;
    
    dflag = 0;
    vflag = 0;
    iflag = 0;
    while((ch = getopt(argc, argv, "div")) != -1) {
        switch(ch) {
            case 'v':
                vflag = 1;
                break;
            case 'd':
                dflag = 1;
                break;
            case 'i':
                iflag = 1;
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
        errno = 0;

        if (dflag) {
            if (rmdir(*argv) == -1) {
                warn("%s", *argv);
                errors = 1;
            }
        }
        else if (iflag) {
            printf("remove: %s ", *argv);
            if ((ch = getchar()) == 'y') {
                if ((unlink(*argv) == -1)) {
                    warn("%s", *argv);
                    errors = 1;
                }
            }
        }
        else if (vflag) {
            printf("%s\n", *argv); 
        }
        else  {
            if ((unlink(*argv) == -1)) {
                warn("%s", *argv);
                errors = 1;
            }
        }
    }
    return errors;
}

void usage(void) {
    fprintf(stderr, "usage: ./rm [-dfiPRrv] file ...\n");
    exit(EXIT_FAILURE);
}