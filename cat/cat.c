#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

void cat_file(int fd, const char *filename) {
    int n;
    char buffer[BUFSIZ];

    while ((n = read(fd, buffer, BUFSIZ)) > 0) {
        if (write(STDOUT_FILENO, buffer, n) < 0){
            printf("%s : %s\n",filename, strerror(errno));
        }
    }
    if (n < 0) {
        printf("%s : %s\n", filename,strerror(errno));
    }
}


int main (int argc, char **argv) {
    int fd;

    if (argc == 1) {
        cat_file(STDIN_FILENO, "stdin");
    } else {
        for (int i = 1; i < argc; i++) {
            fd = open(argv[i], O_RDONLY);
            if (fd < 0) {
                printf("Error opening file: %s\n", strerror(errno));
            } else {
                cat_file(fd, argv[i]);
                close(fd);
            }
        }
    }   
}