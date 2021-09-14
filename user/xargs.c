#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/stat.h"
#include "user/user.h"

#define STR_LEN (32)

// 分割字符串
void split(const char *str, char c, char * ans[], int *strCnt){
  int i = 0;
  int prevPos = 0;
  int cnt = 0;
  do{
    // printf("%c\n", str[i]);
    if( (str[i]==c || str[i]==0) && i-prevPos>0 ){
      // printf("%d, %d, %d\n", cnt, i, prevPos);
      ans[cnt] = (char *)malloc(STR_LEN);
      memcpy(ans[cnt], str+prevPos, i-prevPos);
      ans[cnt][i-prevPos] = 0;
      // printf("%s\n", (ans+VEC_LEN*cnt));
      // printf("%s\n", ans);
      prevPos = i+1;
      cnt++;
    }
  }while(str[i++] != 0);
  *strCnt = cnt;
}

// 读取一行(简单，无块缓存)
int readline(int fd, char *buf){
  int nchar = 0;
  while(read(fd, buf+nchar, 1) != 0){
    if(buf[nchar] == '\n'){
      buf[nchar] = 0;
      return 1;
    }
    nchar++;
  }
  buf[nchar] = 0;
  return 0;
}



int
main(int argc, char *argv[]){
  int maxArgs = -1;
  char *cmdArgv[MAXARG];
  
  int cmdArgc = 0;
  if(strcmp(argv[1], "-n") == 0){
    maxArgs = atoi(argv[2]);
    cmdArgc = argc-3;
    if(maxArgs <= 0){
      printf("param error\n");
      exit(0);
    }
    for(int i = 3; i < argc; i++){
      cmdArgv[i-3] = (char *)malloc(STR_LEN);
      strcpy(cmdArgv[i-3], argv[i]);
    }
  }else{
    cmdArgc = argc-1;
    for(int i = 1; i < argc; i++){
      cmdArgv[i-1] = (char *)malloc(STR_LEN);
      strcpy(cmdArgv[i-1], argv[i]);
    }
  }
  char buf[512];
  char *argvec[MAXARG];

  int strCnt = 0;
  int isEnd;
  do{
    isEnd = readline(0, buf);
    // printf("%s\n", buf);
    split(buf, ' ', argvec, &strCnt);
    // for(int i = 0; i < strCnt; i++){
    //   printf("%d, %s\n", i, argvec[i]);
    // }
    int pid = fork();
    if(pid == 0){
      for(int i = 0; i < strCnt; i++){
        cmdArgv[i+cmdArgc] = argvec[i];
      }
      // for(int i = 0; i < strCnt+cmdArgc; i++){
      //   printf("%s\n", cmdArgv[i]);
      // }
      exec(cmdArgv[0], cmdArgv);
    }else if(pid > 0){
      int tmp = 0;
      wait(&tmp);
    }
  }while(isEnd != 0);
  exit(0);
}
