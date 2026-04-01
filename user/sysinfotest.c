#include "kernel/types.h"
#include "kernel/sysinfo.h"
#include "user/user.h"

void testcall() {
  struct sysinfo info;
  
  printf("sysinfotest: testing call\n");
  
  if (sysinfo(&info) < 0) {
    printf("sysinfotest: sysinfo failed\n");
    exit(1);
  }

  if (sysinfo((struct sysinfo *) 0xffffffffffffffff) != -1) {
    printf("sysinfotest: sysinfo error (invalid address)\n");
    exit(1);
  }
}

void testmem() {
  struct sysinfo info;
  uint64 m1, m2;

  printf("sysinfotest: testing mem\n");
  
  if (sysinfo(&info) < 0) {
    printf("sysinfotest: sysinfo failed\n");
    exit(1);
  }
  m1 = info.freemem;

  if ((uint64)sbrk(4096) == 0xffffffffffffffff) {
    printf("sysinfotest: sbrk failed\n");
    exit(1);
  }

  if (sysinfo(&info) < 0) {
    printf("sysinfotest: sysinfo failed\n");
    exit(1);
  }
  m2 = info.freemem;

  if (m1 - m2 != 4096) {
    printf("sysinfotest: mem error! (m1=%ld, m2=%ld)\n", m1, m2);
    exit(1);
  }
}

void testproc() {
  struct sysinfo info;
  uint64 n1, n2;

  printf("sysinfotest: testing nproc\n");

  if (sysinfo(&info) < 0) {
    printf("sysinfotest: sysinfo failed\n");
    exit(1);
  }
  n1 = info.nproc;

  int pid = fork();
  if (pid < 0){
    printf("sysinfotest: fork failed\n");
    exit(1);
  }

  if (pid == 0) {
    sleep(1);
    exit(0);
  } else {
    if (sysinfo(&info) < 0) {
      printf("sysinfotest: sysinfo failed\n");
      exit(1);
    }
    n2 = info.nproc;

    wait(0);

    if (n2 - n1 != 1) {
      printf("sysinfotest: nproc error! (n1=%ld, n2=%ld)\n", n1, n2);
      exit(1);
    }
  }
}

int
main(int argc, char *argv[]) {
  testcall();
  testmem();
  testproc();

  printf("sysinfotest: OK\n");
  exit(0);
}