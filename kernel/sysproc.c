#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#ifdef PGTBL_SOL
#include "riscv.h"
#endif
#include "vm.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if (t == SBRK_EAGER || n < 0) {
    if (growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if (addr + n < addr)
      return -1;
    if (addr + n > UTOP)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;


  argint(0, &n);
  if (n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n) {
    if (killed(myproc())) {
      release(&tickslock);
      return -1;
    }
    sleep_prepare(&ticks);
    release(&tickslock);
    sleep();
    acquire(&tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
int
sys_vmprint(void)
{
  struct proc *p;

  p = myproc();
  vmprint(p->pagetable);
  return 0;
}
#endif

#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  enum { MAX_PAGES = 256 };
  uchar mask[MAX_PAGES / 8] = {0};
  uint64 start, dst;
  int count;
  struct proc *p = myproc();

  argaddr(0, &start);
  argint(1, &count);
  argaddr(2, &dst);

  if (count < 1 || count > MAX_PAGES)
    return -1;

  uint64 mask_bytes = (count + 7) / 8;
  start = PGROUNDDOWN(start);
  if (start >= MAXVA || (uint64)count > (MAXVA - start) / PGSIZE)
    return -1;
  if (dst >= MAXVA || mask_bytes > MAXVA - dst)
    return -1;

  uint64 writable = PTE_V | PTE_U | PTE_W;
  for (uint64 va = PGROUNDDOWN(dst); va < dst + mask_bytes; va += PGSIZE) {
    pte_t *pte = walk(p->pagetable, va, 0);
    if (pte == 0 || (*pte & writable) != writable)
      return -1;
  }

  for (int i = 0; i < count; i++) {
    uint64 va = start + (uint64)i * PGSIZE;
    pte_t *pte = walk(p->pagetable, va, 0);
    if (pte == 0 || (*pte & (PTE_V | PTE_U)) != (PTE_V | PTE_U))
      return -1;

    if (*pte & PTE_A)
      mask[i / 8] |= 1U << (i % 8);
  }

  if (copyout(p->pagetable, p->sz, dst, (char *)mask, mask_bytes) < 0)
    return -1;

  for (int i = 0; i < count; i++) {
    if (mask[i / 8] & (1U << (i % 8))) {
      uint64 va = start + (uint64)i * PGSIZE;
      pte_t *pte = walk(p->pagetable, va, 0);
      *pte &= ~PTE_A;
    }
  }

  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
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

#ifdef LAB_LOCK
uint64
sys_cpupin(void)
{
  struct proc *p = myproc();
  int cpu;

  argint(0, &cpu);
  if (cpu < 0 || cpu >= NCPU)
    return -1;
  acquire(&p->lock);
  p->pincpu = &cpus[cpu];
  release(&p->lock);
  return 0;
}
#endif
