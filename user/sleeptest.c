#include "kernel/types.h"
#include "user/user.h"
int main() {
 printf("Sleeping for 100 ticks...\n");
 sleep(100); // ngủ khoảng 1 giây
 printf("Woke up!\n");
 exit(0);
}
