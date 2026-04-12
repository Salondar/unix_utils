#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <err.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <linux/limits.h>

int fflag = 0;
int vflag = 0;
int pflag = 0;
int dflag = 0;
int iflag = 0;
int rflag = 0;

void usage(void);
int remove_file(char *path);
int remove_folder(char *path);
int prompt(char *path, struct stat *st);
int request_confirmation(char *path);
int remove_hierarchy(char *path);
int overwrite(char *path);

int main(int argc, char **argv) {
    int ch, exit_status = 0;
    struct stat st;

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
                dflag = 0;
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
        if (fflag) {
            return 0;
        }
        usage();    
    }
    
    
    for (; *argv; argv++) {
        if (strcmp(*argv, ".") == 0 || strcmp(*argv, "..") == 0) {
            fprintf(stderr, "rm: \".\" and \"..\" may not be removed\n");
            exit_status = 1;
            continue;
        }

        if (strcmp(*argv, "/") == 0) {
            fprintf(stderr, "rm: \"/\" may not be removed\n");
            exit_status = 1;
            continue;
        }

        if (rflag) {
            exit_status  = remove_hierarchy(*argv);
            continue;
        }
        errno = 0;
        if (stat(*argv, &st) == -1) {
            if (!fflag) {
                warn("%s", *argv);
                exit_status = 1;
            }
            continue;
        }
        if (!(st.st_mode & S_IWUSR) && isatty(fileno(stdin)) && iflag == 0 && !(S_ISDIR(st.st_mode))) {
            if (prompt(*argv, &st)) {
               exit_status |= remove_file(*argv);
                continue;
            }
        }

        if (dflag) {
            if (!(st.st_mode & S_IWUSR) && isatty(fileno(stdin)) && iflag == 0) {
                if (prompt(*argv, &st) == 0) {
                    continue;
                }
            }
            if (S_ISDIR(st.st_mode)) {
                exit_status |= remove_folder(*argv);
            } else {
                exit_status |= remove_file(*argv);
            }
            continue;
        }
        exit_status |= remove_file(*argv);
    }
    return exit_status;
}

int prompt(char *path, struct stat *st) {
    int rv = 0;
    struct passwd *pw = getpwuid(st->st_uid); 
    struct group *gr =  getgrgid(st->st_gid);

    printf("override %c", st->st_mode & S_IRUSR ? 'r' : '-');
    printf("%c", st->st_mode & S_IWUSR ? 'w' : '-');
    printf("%c", st->st_mode & S_IXUSR ? 'x' : '-');

    printf("%c", st->st_mode & S_IRGRP ? 'r' : '-');
    printf("%c", st->st_mode & S_IWGRP ? 'w' : '-');
    printf("%c", st->st_mode & S_IXGRP ? 'x' : '-');

    printf("%c", st->st_mode & S_IROTH ? 'r' : '-');
    printf("%c", st->st_mode & S_IWOTH ? 'w' : '-');
    printf("%c", st->st_mode & S_IXOTH ? 'x' : '-');
    printf(" %s/%s for %s? ", pw->pw_name, gr->gr_name, path);
    
    if (request_confirmation(path)) {
        rv = 1;
    }
    return rv;
}

int request_confirmation(char *path) {
    int ch, rv = 0;

    if (iflag) {
        printf("remove %s? ", path);
    }

    if ((ch = getchar()) == 'y') {
        rv = 1;
    }
    while((ch = getchar()) != '\n');
    return rv;
}

int remove_file(char *path) {
    errno = 0;

    if (iflag && request_confirmation(path) == 0) {
        return 0;
    }

    if (unlink(path) != 0) {
        warn("%s", path);
        return 1;
    }

    if (vflag) {
        printf("%s\n", path);
    }
    return 0;
}

int remove_folder(char *path) {
    int rv = 0;

    if (iflag && request_confirmation(path) == 0) {
        return 0;
    }

    errno = 0;
    if (rmdir(path) == -1) {
        warn("%s", path);
        rv = 1;
    }

    if (vflag) {
        printf("%s\n", path);
    }
    return rv;
}


int remove_hierarchy(char *path) {
    struct stat st;
    DIR *dir;
    struct dirent *direntry;
    char filepath[PATH_MAX];
    int rv = 0;
    
    errno = 0;
    if ((dir = opendir(path)) == NULL) {
        prompt(path, &st);
        return 1;
    }

    while ((direntry = readdir(dir)) != NULL) {
        if (strcmp(direntry->d_name, ".") != 0 && strcmp(direntry->d_name, "..") != 0) {
            snprintf(filepath, PATH_MAX, "%s/%s", path, direntry->d_name);
            errno = 0;
            if (stat(filepath, &st) == -1) {
                warn("%s", filepath);
                return 1;
            }
            if (S_ISDIR(st.st_mode)) {
                rv = remove_hierarchy(filepath);
            } else {
                remove_file(filepath);
            }
        }
    }
    rv = remove_folder(path);
    return rv;
}

int overwrite(char *path) {
    FILE *fp;
    struct stat st;
    int rv = 0;

    errno = 0;
    if (stat(path, &st) == -1) {
        if (!fflag) {
            warn("%s", path);
            rv = 1;
        }
    }

    if (st.st_nlink == 1) {
        if (S_ISREG(st.st_mode) && (st.st_mode & S_IWUSR)) {
            errno = 0;
            fp = fopen(path, "r+");
            if (fp == NULL) {
                warn("%s", path);
                rv = 1;
            }
            int ch;
            while((ch = fgetc(fp)) != EOF) {
                fputc(arc4random(), fp);
            }
        }
        
    } else {
        fprintf(stderr, "not overwritten due to multiple links");
        rv = 1;
    }

    if (S_ISDIR(st.st_mode) && dflag) {
        rv = remove_folder(path);
    } else {
        rv = remove_file(path);
    }
    return rv;
}

void usage(void) {
    fprintf(stderr, "./rm [-dfiPRrv] file ...\n");
    exit(EXIT_FAILURE);
}