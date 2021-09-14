#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[]){
  // 未处理read，write异常
  int pipefd1[2]; // 父进程->子进程
  int pipefd2[2]; // 子进程->父进程
  if(pipe(pipefd1) != 0 || pipe(pipefd2) != 0){
    printf("sys_call pipe error\n");
    exit(-1);
  }
  char buf[1];
  int pid = fork();
  if(pid > 0){
    close(pipefd1[0]);
    close(pipefd2[1]);
    buf[0] = '1';
    write(pipefd1[1], buf, 1);
    read(pipefd2[0], buf, 1);
    printf("%d: received pong\n", getpid());
    exit(0);
  }else if(pid == 0){
    close(pipefd1[1]);
    close(pipefd2[0]);
    read(pipefd1[0], buf, 1);
    printf("%d: received ping\n", getpid());
    buf[0] = '1';
    write(pipefd2[1], buf, 1);
    exit(0);
  }else{
    printf("sys_call fork error\n");
    exit(-1);
  }
}
