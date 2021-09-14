#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void findParentDir(char *path){
  char *p;
  for(p=path+strlen(path)-1; p >= path && *p != '/'; p--)
    ;
  p++;
  *p = 0;
}

void addName(char * path, const char * name){
  char *p = path+strlen(path);
  --p;
  if(*p != '/'){
    ++p;
    *p = '/';
  }
  for(int i = 0; name[i]!=0; i++){
    ++p;
    *p = name[i];
  }
  ++p;
  *p = 0;
}

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}

int isDir(char *path){
  return 0;
}

void
find(const char *path, const char *fileName)
{
  char buf[512];
  strcpy(buf, path);
  int fd;
  struct dirent de;
  struct stat st;

  // 校验path是否为目录
  if((fd = open(path, 0)) < 0){
    fprintf(2, "find: file not found %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  if(st.type != T_DIR){
    fprintf(2, "%s is not a directory", path);
    close(fd);
    return;
  }

  // 逐一检查目录项，如果目录项是目录，则递归调用，其余检查文件名称是否一致
  while(read(fd, &de, sizeof(de)) == sizeof(de)){
    if(de.inum == 0)
      continue;
    if(strcmp(de.name, ".")==0 ||strcmp(de.name, "..")==0){
      continue;
    }
    addName(buf, de.name);
    // printf("%s\n", buf);
    // printf("%s\n", de.name);
    stat(buf, &st);
    if( (st.type != T_DIR) && (strcmp(fileName, de.name) == 0) ){
      printf("%s\n", buf);
    }else if(st.type == T_DIR){
      find(buf, fileName);
    }
    findParentDir(buf);
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  if(argc < 2){
    return -1;
  }
  find(argv[1], argv[2]);
  exit(0);
}