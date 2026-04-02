#include "kernel/types.h"
#include "kernel/ptree.h"
#include "user/user.h"

#define MAXPROC 64

// Print the subtree rooted at pid, using 'depth' levels of indentation.
void
print_tree(struct ptreeinfo *procs, int n, int pid, int depth)
{
    for (int i = 0; i < n; i++) {
        if (procs[i].ppid == pid && procs[i].pid != pid) {
            for (int j = 0; j < depth * 2; j++)
                printf(" ");
            printf("%d %s state = %d mem = %d\n",
                   procs[i].pid, procs[i].name,
                   procs[i].state, (int)procs[i].memsize);
            print_tree(procs, n, procs[i].pid, depth + 1);
        }
    }
}

int
main(void)
{
    struct ptreeinfo procs[MAXPROC];
    int n;

    n = ptree(procs, MAXPROC);
    if (n < 0) {
        fprintf(2, "pstree: ptree failed\n");
        exit(1);
    }

    // Find the root process (ppid == 0 or ppid not in list)
    // Typically pid 1 (init) is the root with ppid 0.
    for (int i = 0; i < n; i++) {
        if (procs[i].ppid == 0) {
            printf("%d %s state = %d mem = %d\n",
                   procs[i].pid, procs[i].name,
                   procs[i].state, (int)procs[i].memsize);
            print_tree(procs, n, procs[i].pid, 1);
        }
    }

    exit(0);
}
