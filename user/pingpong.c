#include "kernel/types.h"
#include "user/user.h"

int 
main() {

    int fd1[2];
    int fd2[2];
    char buffer[20];

    pipe(fd1);
    pipe(fd2);

    if (fork() == 0) {
        close(fd1[1]);
        close(fd2[0]);

        read(fd1[0], buffer, sizeof(buffer));
        printf("%d: %s\n", getpid(), buffer);
        write(fd2[1], "received pong", 13);

        close(fd1[0]);
        close(fd2[1]);
    } else {
        close(fd1[0]);
        close(fd2[1]);

        write(fd1[1], "received ping", 13);
        read(fd2[0], buffer, sizeof(buffer));
        printf("%d: %s\n", getpid(), buffer); 

        close(fd1[1]);
        close(fd2[0]); 
    }
    exit(0);
}