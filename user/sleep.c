#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    // 1. Thiếu hoặc dư đối số
    if (argc != 2) {
        fprintf(2, "Usage: sleep <ticks>\n");
        exit(1);
    }

    // 2. Chuỗi rỗng
    if (argv[1][0] == '\0') {
        fprintf(2, "Error: Ticks must be a positive integer\n");
        exit(1);
    }

    // 3. Kiểm tra từng ký tự phải là số
    for (int i = 0; argv[1][i] != '\0'; i++) {
        if (argv[1][i] < '0' || argv[1][i] > '9') {
            fprintf(2, "Error: Ticks must be a positive integer\n");
            exit(1);
        }
    }

    int ticks = atoi(argv[1]);

    // 4. Kiểm tra số <= 0
    if (ticks <= 0) {
        fprintf(2, "Error: Ticks must be a positive integer\n");
        exit(1);
    }

    // 5. Trường hợp hợp lệ
    sleep(ticks);

    exit(0);
}