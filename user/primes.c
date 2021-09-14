#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define NUMS_CNT 34


void selectNums(int fd){
    // 读数据
    int nums[NUMS_CNT];
    int cnt = 0;
    for(int i = 0; ;i++){
        if(0 == read(fd, (int*)nums+i, sizeof(int))){
            break;
        }
        // printf("%d | ", nums[i]);
        cnt++;
    }
    close(fd);

    printf("prime %d\n", nums[0]);

    if(cnt <= 1){
        return;
    }
    int pipefd[2];
    pipe(pipefd);

    int pid = fork();
    if(pid == 0){
        
        close(pipefd[0]);
        for(int i = 1; i < cnt; i++){
            if(nums[i] % nums[0] == 0){
                continue;
            }
            // printf("%d | ", nums[i]);
            write(pipefd[1], (int*)nums+i, sizeof(int));
        }
        close(pipefd[1]);
        return;
    }else if(pid > 0){
        // 父线程递归调用，这样可以保证最后一个数字输出之前主线程不会返回
        close(pipefd[1]);
        // printf("child, %d", getpid());
        selectNums(pipefd[0]);
        close(pipefd[0]);
    }
}

int
main(int argc, char *argv[]){
    int pipefd[2];
    pipe(pipefd);
    for(int i = 2; i <= 35; i++){
        int tmp = i;
        write(pipefd[1], &tmp, sizeof(int));
    }
    close(pipefd[1]);
    selectNums(pipefd[0]);
    // printf("term\n");
    // sleep(10);
    return exit(0);
}