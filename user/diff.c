#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define MAX_LINE 1024

static int readline(int fd, char *buffer, int max) {
    int i = 0;
    char c;

    while (i + 1 < max) {            // 1 byte for '\0'
        int n = read(fd, &c, 1);
        if (n == 0) {                // EOF
            break;
        }
        if (n < 0) {                 // Error
            return -1;
        }
        buffer[i++] = c;
        if (c == '\n') {             // Endline
            break;
        }
    }

    buffer[i] = '\0';
    return i;
}

int main(int argc, char *argv[]) {
    int fd1, fd2;
    char *file1, *file2;
    int quiet = 0; // Flag
    if (argc < 3) { 
        fprintf(2, "usage: diff file1 file2 [-q]\n");
        exit(1);
    }
    if (argc == 4 && strcmp(argv[1], "-q") == 0) {
        quiet = 1;
        file1 = argv[2];
        file2 = argv[3];
    }
    else if (argc == 3) {
        file1 = argv[1];
        file2 = argv[2];
    }
    else { // Not equal to 3 or 4 arguments
        fprintf(2, "usage: diff file1 file2 [-q]\n");
        exit(1);
    }
    fd1 = open(file1, O_RDONLY);
    if (fd1 < 0) {
        fprintf(2, "diff: cannot open %s\n", file1);
        exit(1);
    }
    fd2 = open(file2, O_RDONLY);
    if (fd2 < 0) {
        fprintf(2, "diff: cannot open %s\n", file2);
        close(fd1);
        exit(1);
    }

    char line1[MAX_LINE];
    char line2[MAX_LINE];
    int len1, len2;
    int lineNum = 1;
    while (1) {
        len1 = readline(fd1, line1, MAX_LINE);
        len2 = readline(fd2, line2, MAX_LINE);

        if (len1 == 0 && len2 == 0) {
            break; 
        }
        if (strcmp(line1, line2) != 0) {

            if (quiet) {
                fprintf(1, "diff: files differ\n");
                close(fd1);
                close(fd2);
                exit(0);
            }
            if (len1 == 0) {
                fprintf(1, "%s:%d: < EOF\n", file1, lineNum);
            } else {
                fprintf(1, "%s:%d: < %s", file1, lineNum, line1);
            }
            if (len2 == 0) {
                fprintf(1, "%s:%d: > EOF\n", file2, lineNum);
            } else {
                fprintf(1, "%s:%d: > %s", file2, lineNum, line2);
            }
        }
        ++lineNum;
    }
    close(fd1);
    close(fd2);
    exit(0);
}