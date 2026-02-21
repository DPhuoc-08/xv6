#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
    int fd_src = 0; // Initial value for fd of source file

    if (argc == 3) { // Check if number of arguments is 3
        fd_src = open(argv[1], O_RDONLY);
        if (fd_src < 0) { // Check if file does not exist
            fprintf(2, "cp: cannot open %s\n", argv[1]); // 2 stands for standard error
            exit(1);
        }

    } else if (argc != 3) { // Check if number of arguments is not 3
        fprintf(2, "usage: cp src dst\n");
        exit(1);
    }

    int fd_dst = open(argv[2], O_CREATE | O_WRONLY); // Create or overwrite destination file
    if (fd_dst < 0) {
    close(fd_src);
        fprintf(2, "cp: cannot open or create %s\n", argv[2]);
        exit(1);
    }

    char buffer[512];

    while (1) {
        int n = read(fd_src, buffer, sizeof(buffer));
        if (n < 0) {
            fprintf(2, "cp: read error\n");
            if (fd_src != 0) {
                close(fd_src);
            } 
            exit(1);
        }
        
        if (n > 0) {
            if (write(fd_dst, buffer, n) != n) {
                fprintf(2, "cp: write error\n");
                close(fd_src);
                close(fd_dst);
                exit(1);
            }
        }

        if (n == 0) {
            break; // EOF
        }
    }
    if (fd_src != 0) {
        close(fd_src);
    }
    if (fd_dst != 0) {
        close(fd_dst);
    }
    exit(0);
}