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

int rv = 0;

void usage(void);
void delete_folder(char *path);
void delete_file(char *path);

int main(int argc, char **argv) {
    int ch, vflag, dflag, iflag, fflag, rflag, pflag;
    struct stat st;
    
    
    dflag = 0;
    vflag = 0;
    iflag = 0;
    fflag = 0;
    rflag = 0;
    pflag = 0;
    while((ch = getopt(argc, argv, "dfiPRrv")) != -1) {
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
            case 'P':
                pflag = 1;
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
   
    for (; *argv != NULL; argv++) {
        errno = 0;

        if (dflag) {
            if (rmdir(*argv) == -1) {
                warn("%s", *argv);
                rv = 1;
            }
        }
        if (iflag) {
            printf("remove %s: ", *argv);
            if ((ch = getchar()) == 'y') {
                delete_file(*argv);
            }
            while ((ch = getchar()) != '\n' && ch != EOF);
        }
        if (vflag) {
            printf("%s\n", *argv);
            delete_file(*argv); 
        }

        if (pflag) {
            FILE *fp = fopen(*argv, "r+");

            if (!fp) {
                fprintf(stderr, "Could not open file\n");
            }
            while((ch = fgetc(fp)) != EOF) {
                fputc(arc4random(), fp);
            }
            fclose(fp);
            delete_file(*argv);
        }

        if (fflag) {
            unlink(*argv);
        }
        if (rflag) {
            stat(*argv, &st);
            if (S_ISDIR(st.st_mode)) {
                delete_folder(*argv);
            } else {
                delete_file(*argv);
            }
        }
        if (!(rflag | dflag | fflag | iflag | vflag | pflag))  {
            int notwrite = 1;
            __mode_t mode;
            stat(*argv, &st);
            struct passwd *pw = getpwuid(st.st_uid);
            struct group *gr = getgrgid(st.st_gid);
            mode = st.st_mode;

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
                    delete_file(*argv);
                }
                while ((ch = getchar()) != '\n' && ch != EOF);
                notwrite = 0;
            }
            if (notwrite) {
                delete_file(*argv);       
            }
          
        }
    }
    return rv;
}

void delete_file(char *path) {
    errno = 0;
    if (unlink(path) == -1) {
        warn("%s", path);
        rv = 1;
    }
}

void delete_folder(char *path) {
    struct stat st;
    DIR *directory;
    struct dirent *entry;
    char filepath[PATH_MAX];
    directory = opendir(path);

    while ((entry = readdir(directory)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            snprintf(filepath, PATH_MAX, "%s/%s", path, entry->d_name);
            stat(filepath, &st);
            if (S_ISDIR(st.st_mode)) {
                delete_folder(filepath);
            } else {
                delete_file(filepath);
            } 
        }
      
    }
    rmdir(path);
}

void usage(void) {
    fprintf(stderr, "usage: ./rm [-dfiPRrv] file ...\n");
    exit(EXIT_FAILURE);
}