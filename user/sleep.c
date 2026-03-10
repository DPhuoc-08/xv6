#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    // 1. Thieu hoac du doi so
    if (argc != 2) {
        fprintf(2, "Usage: sleep <ticks>\n");
        exit(1);
    }

    // 2. Chuoi rong
    if (argv[1][0] == '\0') {
        fprintf(2, "Error: Ticks must be a positive integer\n");
        exit(1);
    }

    // 3. Kiem tra tung ky tu phai la so
    for (int i = 0; argv[1][i] != '\0'; i++) {
        if (argv[1][i] < '0' || argv[1][i] > '9') {
            fprintf(2, "Error: Ticks must be a positive integer\n");
            exit(1);
        }
    }

    int ticks = atoi(argv[1]);

    // 4. Kiem tra so <= 0
    if (ticks <= 0) {
        fprintf(2, "Error: Ticks must be a positive integer\n");
        exit(1);
    }

    // 5. Truong hop hop le
    sleep(ticks);

    exit(0);
}