#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int max_depth = 999999;
int d_option = 0;

static int is_positive_int_str(const char *s) {
    if (s[0] == '\0') return 0;
    for (int i = 0; s[i] != '\0'; i++) {
        if (s[i] < '0' || s[i] > '9') return 0;
    }
    return 1;
}

static int is_dot_or_dotdot(const char *name) {
    return strcmp(name, ".") == 0 || strcmp(name, "..") == 0;
}

static void name_from_dirent(char *out, struct dirent *de) {
    memmove(out, de->name, DIRSIZ);
    out[DIRSIZ] = '\0';
}

static int join_path(char *out, int outsz, const char *parent, const char *child) {
    int lp = strlen(parent);
    int lc = strlen(child);
    if (lp + 1 + lc + 1 > outsz) return -1;
    memmove(out, parent, lp);
    out[lp] = '/';
    memmove(out + lp + 1, child, lc);
    out[lp + 1 + lc] = '\0';
    return 0;
}

static void print_indent(int depth, int *is_last) {
    for (int i = 0; i < depth; i++) {
        if (is_last[i]) printf("    ");
        else             printf("│   ");
    }
}

static void tree(const char *path, int depth, int *is_last) {
    if (depth >= max_depth) return;

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        fprintf(2, "tree: cannot open %s\n", path);
        return;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        fprintf(2, "tree: fstat failed %s\n", path);
        close(fd);
        return;
    }

    if (st.type != T_DIR) {
        close(fd);
        return;
    }

    char pending[DIRSIZ + 1];
    int has_pending = 0;
    struct dirent de;

    while (read(fd, &de, sizeof(de)) == sizeof(de)) {
        if (de.inum == 0) continue;

        char name[DIRSIZ + 1];
        name_from_dirent(name, &de);

        if (is_dot_or_dotdot(name)) continue;

        if (d_option) {
            char child[512];
            if (join_path(child, sizeof(child), path, name) < 0) continue;
            struct stat cst;
            if (stat(child, &cst) < 0) continue;
            if (cst.type != T_DIR) continue;
        }

        if (has_pending) {
            print_indent(depth, is_last);
            printf("├── %s\n", pending);

            char child[512];
            if (join_path(child, sizeof(child), path, pending) == 0) {
                struct stat cst;
                if (stat(child, &cst) == 0 && cst.type == T_DIR) {
                    is_last[depth] = 0;
                    tree(child, depth + 1, is_last);
                }
            }
        }

        strcpy(pending, name);
        has_pending = 1;
    }
    close(fd);

    if (has_pending) {
        print_indent(depth, is_last);
        printf("└── %s\n", pending);

        char child[512];
        if (join_path(child, sizeof(child), path, pending) == 0) {
            struct stat cst;
            if (stat(child, &cst) == 0 && cst.type == T_DIR) {
                is_last[depth] = 1;
                tree(child, depth + 1, is_last);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc > 5) {
        fprintf(2, "usage: tree [path] [-L depth] [-d]\n");
        exit(1);
    }

    const char *start = ".";

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-L") == 0) {
            if (i + 1 < argc && is_positive_int_str(argv[i + 1])) {
                max_depth = atoi(argv[i + 1]);
                i++;
                continue;
            }
            fprintf(2, "tree: invalid depth\n");
            fprintf(2, "usage: tree [path] [-L depth] [-d]\n");
            exit(1);
        } else if (strcmp(argv[i], "-d") == 0) {
            d_option = 1;
        } else {
            start = argv[i];
        }
    }

    printf("%s\n", start);

    int is_last[128] = {0};
    tree(start, 0, is_last);

    exit(0);
}