#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
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
  //start
  backtrace();//调用backtrace
  //end
  return 0;
}

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

uint64
sys_sigalarm(void)
{
  // 收到系统调用参数, 存储在进程状态中
  int interval;
  uint64 pointer;
  // 从s0寄存器中取参数, 拿到间隔ticks时长, 存储为interval
  if(argint(0, &interval) < 0)
    return -1;
  /* 检查到最后发现是argaddr函数写错了(pointer有错误)
   第一个参数应该是1 */
  // 从s1寄存器中取参数, 拿到handler function函数指针, 存储为pointer)
  if(argaddr(1, &pointer) < 0) 
    return -1;
  struct proc *p = myproc();
  p->alarm_interval = interval;	 
  p->pointer = pointer;
  return 0;
}

uint64
sys_sigreturn(void)
{
  struct proc *p = myproc();
  if(p->alarm_flag == 0){	// 如果当前禁止中断
    memmove(p->trapframe, &(p->alarm_trapframe), sizeof(struct trapframe));	
    // memmove(dst, src, size)
    p->alarm_flag = 1;	// 设置为允许中断
  }
  return 0;
}


