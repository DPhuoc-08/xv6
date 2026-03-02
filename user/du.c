#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int s_option = 0;
int a_option = 0;

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

long du(const char *path, int is_root) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        fprintf(2, "du: cannot open %s\n", path);
        return 0;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        fprintf(2, "du: fstat failed %s\n", path);
        close(fd);
        return 0;
    }

    if (st.type == T_FILE) {
        close(fd);
        long size = st.size;
        int should_print = 0;
        if (s_option) {
            if (is_root) should_print = 1;
        } else {
            if (a_option || is_root) should_print = 1;
        }
        if (should_print) {
            printf("%d\t%s\n", (int)size, path);
        }
        return size;
    } else if (st.type == T_DIR) {
        long total_size = 0;
        struct dirent de;
        while (read(fd, &de, sizeof(de)) == sizeof(de)) {
            if (de.inum == 0) continue;
            char name[DIRSIZ + 1];
            name_from_dirent(name, &de);

            if (is_dot_or_dotdot(name)) continue;

            char child[512];
            if (join_path(child, sizeof(child), path, name) < 0) continue;

            total_size += du(child, 0);
        }
        close(fd);

        int should_print = 0;
        if (s_option) {
            if (is_root) should_print = 1;
        } else {
            should_print = 1;
        }

        if (should_print) {
            printf("%d\t%s\n", (int)total_size, path);
        }
        return total_size;
    }

    close(fd);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc > 4) {
        fprintf(2, "usage: du [path] [-a] [-s]\n");
        exit(1);
    }

    const char *start = ".";

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-a") == 0) {
            a_option = 1;
        } else if (strcmp(argv[i], "-s") == 0) {
            s_option = 1;
        } else {
            start = argv[i];
        }
    }

    du(start, 1);
    exit(0);
}
