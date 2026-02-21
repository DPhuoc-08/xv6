#include "kernel/types.h"
#include "user/user.h"

void sieve(int left_read_fd) __attribute__((noreturn));
// Ham de quy loc so nguyen to
void sieve(int left_read_fd)
{
    int p, n;

    // Neu da het so de doc thi dung chuong trinh
    if (read(left_read_fd, &p, sizeof(p)) == 0)
    {
        close(left_read_fd);
        exit(0);
    }
    printf("prime %d\n", p);

    // Tao pipe phia ben phai de truyen so da loc
    int right_pipe[2];
    pipe(right_pipe);

    int pid = fork();
    if (pid == 0)
    {
        // tien trinh con
        close(left_read_fd);
        close(right_pipe[1]);
        sieve(right_pipe[0]);
    }
    else
    {
        // Tien trinh cha
        close(right_pipe[0]);

        // Doc cac so con lai
        while (read(left_read_fd, &n, sizeof(n)) > 0)
        {
            if (n % p != 0)
                write(right_pipe[1], &n, sizeof(n));
        }

        close(left_read_fd);
        close(right_pipe[1]); 
        wait(0);  
        exit(0);
    }
}

int main(int argc, char *argv[])
{
    int fd[2];
    pipe(fd);
    int pid = fork();
    if (pid == 0)
    {
        // Tien trinh con
        close(fd[1]);
        sieve(fd[0]);
    }
    else
    {
        // Tien trinh cha
        close(fd[0]);
        for (int i = 2; i <= 280; i++)
            write(fd[1], &i, sizeof(i));

        close(fd[1]);
        wait(0);
    }

    exit(0);
}