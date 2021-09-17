#ifndef SYS_INFO_H
#define SYS_INFO_H
struct sysinfo {
  uint64 freemem;   // amount of free memory (bytes)
  uint64 nproc;     // number of process
};
#endif