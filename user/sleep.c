#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[]){
  if(argc < 2){
    printf("Usage: sleep second...\n");
    exit(-1);
  }

  int sleepTime = atoi(argv[1]);
  if(sleepTime < 0){
    printf("illeagle sleep time %d", sleepTime);
  }
  if(sleep(sleepTime) == 0){
    exit(0);
  }else{
    printf("sys_call sleep error");
    exit(0);
  }
}
