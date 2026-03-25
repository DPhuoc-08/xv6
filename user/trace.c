#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int i;
  char *nargv[MAXARG];

  if(argc < 3){
    fprintf(2, "usage: trace mask command\n");
    exit(1);
  }

  if(trace(atoi(argv[1])) < 0){
    fprintf(2, "trace: failed\n");
    exit(1);
  }

  for(i = 2; i < argc && i < MAXARG; i++){
    nargv[i - 2] = argv[i];
  }
  nargv[argc - 2] = 0;

  exec(nargv[0], nargv);
  fprintf(2, "trace: exec failed\n");
  exit(1);
}