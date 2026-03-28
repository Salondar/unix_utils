#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <err.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <linux/limits.h>
#include <string.h>

void usage(void);
void delete_folder(char *path);

int main(int argc, char **argv) {
    int ch, vflag, dflag, iflag, fflag, rflag, errors;
    
    
    dflag = 0;
    vflag = 0;
    iflag = 0;
    fflag = 0;
    rflag = 0;

    while((ch = getopt(argc, argv, "dfiRrv")) != -1) {
        switch(ch) {
            case 'd':
                dflag = 1;
                break;
            case 'f':
                fflag = 1;
                iflag = 0;
                break;
            case 'i':
                iflag = 1;
                fflag = 0;
                break;
            case 'R':
            case 'r':
                rflag = 1;
                break;
            case 'v':
                vflag = 1;
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
        if (iflag) {
            printf("remove %s: ", *argv);
            if ((ch = getchar()) == 'y') {
                if ((unlink(*argv) == -1)) {
                    warn("%s", *argv);
                    errors = 1;
                }
            }
            while ((ch = getchar()) != '\n' && ch != EOF);
        }
        if (vflag) {
            printf("%s\n", *argv); 
        }
        if (fflag) {
            unlink(*argv);
        }
        if (rflag) {
            struct stat st;
            stat(*argv, &st);
            if (S_ISDIR(st.st_mode)) {
                delete_folder(*argv);
            } else {
                errno = 0;
                if (unlink(*argv) == -1) {
                    warn("%s", *argv);
                    errors = 1;
                }
            }
        }
        else  {
            int notwrite = 1;
            struct stat statbuf;
            __mode_t mode;
            stat(*argv, &statbuf);
            struct passwd *pw = getpwuid(statbuf.st_uid);
            struct group *gr = getgrgid(statbuf.st_gid);
            mode = statbuf.st_mode;

            if (!(mode & S_IWUSR) && isatty(fileno(stdin))) {
                printf("override %c", mode & S_IRUSR ? 'r' : '-');
                printf("%c", mode & S_IWUSR ? 'w' : '-');
                printf("%c", mode & S_IXUSR ? 'x' : '-');

                printf("%c", mode & S_IRGRP ? 'r' : '-');
                printf("%c", mode & S_IWGRP ? 'w' : '-');
                printf("%c", mode & S_IXGRP ? 'x' : '-');

                printf("%c", mode & S_IROTH ? 'r' : '-');
                printf("%c", mode & S_IWOTH ? 'w' : '-');
                printf("%c", mode & S_IXOTH ? 'x' : '-');

                printf(" %s/%s for %s? ", pw->pw_name, gr->gr_name, *argv);

                if ((ch = getchar()) == 'y') {
                    if ((unlink(*argv) == -1)) {
                        warn("%s", *argv);
                        errors = 1;
                    }
                }
                while ((ch = getchar()) != '\n' && ch != EOF);
                notwrite = 0;
            }
            if (notwrite) {
                if ((unlink(*argv) == -1)) {
                    warn("%s", *argv);
                    errors = 1;
                }       
            }
          
        }
    }
    return errors;
}

void delete_folder(char *path) {
    struct stat st;
    DIR *directory;
    struct dirent *entry;
    char filepath[PATH_MAX];
    directory = opendir(path);

    while ((entry = readdir(directory)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            strcpy(filepath, path);
            strcat(filepath, "/");
            strcat(filepath, entry->d_name);
            stat(filepath, &st);
            if (S_ISDIR(st.st_mode)) {
                delete_folder(filepath);
            } else {
                unlink(filepath);
            } 
        }
      
    }
    rmdir(path);
}

void usage(void) {
    fprintf(stderr, "usage: ./rm [-dfiPRrv] file ...\n");
    exit(EXIT_FAILURE);
}