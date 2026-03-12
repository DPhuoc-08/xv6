#include "kernel/types.h"
#include "user/user.h"

static void sieve(int leftReadFd) __attribute__((noreturn));

static void
sieve(int leftReadFd)
{
    int p, n;

    if (read(leftReadFd, &p, sizeof(p)) == 0)
    {
        close(leftReadFd);
        exit(0);
    }
    printf("prime %d\n", p);

    int rightPipe[2];
    pipe(rightPipe);

    int pid = fork();
    if (pid == 0)
    {
        close(leftReadFd);
        close(rightPipe[1]);
        sieve(rightPipe[0]);
    }
    else
    {
        close(rightPipe[0]);

        while (read(leftReadFd, &n, sizeof(n)) > 0)
        {
            if (n % p != 0)
                write(rightPipe[1], &n, sizeof(n));
        }

        close(leftReadFd);
        close(rightPipe[1]); 
        wait(0);  
        exit(0);
    }
}

int
main(int argc, char *argv[])
{
    int initFd[2];
    pipe(initFd);
    int pid = fork();
    if (pid == 0)
    {
        close(initFd[1]);
        sieve(initFd[0]);
    }
    else
    {
        close(initFd[0]);
        for (int i = 2; i <= 280; i++)
            write(initFd[1], &i, sizeof(i));

        close(initFd[1]);
        wait(0);
    }

    exit(0);
}