#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "date.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
extern pte_t *walk(pagetable_t, uint64, int);
int
sys_pgaccess(void)
{
  // lab pgtbl: your code here.
  uint64 va;
  uint64 mask;
  int num;  
  const int maxnum = 64;	
  // 设定可扫描页数的上限，设定为 64 是因为掩码类型 uint64 最多存储 64 位的值

  if(argaddr(0, &va) < 0 || argint(1, &num) < 0 || argaddr(2, &mask) < 0) // 解析参数
    return -1;
  
  if(num > maxnum)
    num = maxnum;
  
  struct proc *p = myproc();
  if(p == 0)
    return -1;
  
  pagetable_t pagetable = p->pagetable;  // 找到进程的页表
  uint64 procmask = 0;	// 【设定掩码初值为0】

  for (int i = 0; i < num; i++){
    // walk:
    // Return the address of the PTE in page table pagetable
    // that corresponds to virtual address va.
    // 【循环调用walk函数，得到页表中制定虚拟地址对应的页表项，并检查每一页的PTE_A位】 
    pte_t *pte = walk(pagetable, ((uint64)va) + (uint64)PGSIZE * i, 0);
    if(pte == 0)
      continue;
    if (((*pte) & PTE_A)) {
        procmask |= (1L << i);  // 记录在第i个有效位
        *pte = *pte & (~PTE_A); // 记录后清除PTE_A位，保证每一次pgaccess的正确性
    }
  }
  return copyout(pagetable, mask, (char *) &procmask, sizeof(uint64));
}
#endif

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
