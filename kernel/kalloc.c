// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

// static inline 
int
pa2rf(uint64 pa){
  return (PGROUNDDOWN(pa)-KERNBASE)/PGSIZE;
}

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
  int reference_count[(PHYSTOP - KERNBASE)/PGSIZE];
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  memset(kmem.reference_count, 0, (PHYSTOP - KERNBASE)/4096 * sizeof(int));
  freerange(end, (void*)PHYSTOP);
}

void
kfree_init(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  acquire(&kmem.lock);
  int *ref_cnt = &kmem.reference_count[pa2rf((uint64)pa)];
  --(*ref_cnt);
  if(*ref_cnt == 0){
    // Fill with junk to catch dangling refs.
    memset(pa, 1, PGSIZE);
    r = (struct run*)pa;
    r->next = kmem.freelist;
    kmem.freelist = r;
  } else if(*ref_cnt < 0){
    panic("kfree: error reference");
  }
  // 如果引用计数-1后大于0，则直接返回
  release(&kmem.lock);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree_init(p);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r){
    kmem.freelist = r->next;
    ++kmem.reference_count[pa2rf((uint64)r)];
  }
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

void
kshare(void *pa)
{
  acquire(&kmem.lock);
  ++kmem.reference_count[pa2rf((uint64)pa)];
  release(&kmem.lock);
}

// 复制物理页，如果该页的引用计数为1，则直接返回，如果大于1，则分配一个新的物理页，并复制值
// 返回值标识分配物理页是否成功0成功，-1失败（失败只会因为kalloc）
// 新地址保存在dupaddr中
int
kdup(void *pa, void **dup_addr)
{
  acquire(&kmem.lock);
  int *ref_cnt = &kmem.reference_count[pa2rf((uint64)pa)];
  if(*ref_cnt < 1){
    panic("kdup");
  } 

  if(*ref_cnt == 1){
    release(&kmem.lock);
    *dup_addr = pa;
    return 0;
  } else if(*ref_cnt > 1){
    void *new_page;
    // kalloc代码，去掉加锁
    struct run *r = kmem.freelist;
    if(r){
      kmem.freelist = r->next;
      ++kmem.reference_count[pa2rf((uint64)r)];
    }
    new_page = (void *)r;
    ////
    if(new_page){
      --(*ref_cnt);
      release(&kmem.lock);
      memmove(new_page, pa, PGSIZE);
      *dup_addr = new_page;
      return 0;
    }else{
      release(&kmem.lock);
      *dup_addr = pa;
      return -1;
    }
  } else {
    release(&kmem.lock);
    panic("kdup");
    return -1;
  }
}