#include "kernel/types.h"
#include "user/user.h"
int main() {

    int fd1[2]; // From parent to child
    int fd2[2]; // From child to parent
    char buffer[20];

    // Create pipes
    pipe(fd1);
    pipe(fd2);

    if (fork() == 0) {
        // Child process
        close(fd1[1]); // Child's role is to read 
        close(fd2[0]); // Child's role is to write

        read(fd1[0], buffer, sizeof(buffer)); // Read from parent 
        printf("%d: %s\n", getpid(), buffer);
        write(fd2[1], "received pong", 13); // Write to parent

        close(fd1[0]); // Close pipe after using
        close(fd2[1]); // Close pipe after using
    } else {
        // Parent process
        close(fd1[0]); // Parent's role is to write
        close(fd2[1]); // Parent's role is to read

        write(fd1[1], "received ping", 13); // Write to child
        read(fd2[0], buffer, sizeof(buffer)); // Read from child
        printf("%d: %s\n", getpid(), buffer); 

        close(fd1[1]); // Close pipe after using
        close(fd2[0]); // Close pipe after using
    }
    exit(0);
}