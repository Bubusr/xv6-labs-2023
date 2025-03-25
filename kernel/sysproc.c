#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
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
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
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


  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  uint64 addr;         // Địa chỉ bắt đầu
    int num_pages;      // Số lượng trang cần kiểm tra
    uint64 mask_addr;   // Địa chỉ bộ đệm để lưu kết quả
    struct proc *p = myproc();
    uint64 mask = 0;    // Biến lưu kết quả

    // Nhận tham số từ người dùng
    argaddr(0, &addr);argint(1, &num_pages);argaddr(2, &mask_addr);



    for (int i = 0; i < num_pages; i++) {
        pte_t *pte = walk(p->pagetable, addr + (i * PGSIZE), 0);
        if (pte && (*pte & PTE_V)) {  // Kiểm tra xem PTE có hợp lệ không
            if (*pte & PTE_A) {  // Kiểm tra bit PTE_A
                mask |= (1 << i);  // Bật bit tương ứng trong mask
            }
            *pte &= ~PTE_A;  // Xóa bit PTE_A sau khi kiểm tra
        }
    }

    // Ghi kết quả vào bộ đệm
    if (copyout(p->pagetable, mask_addr, (char *)&mask, sizeof(mask)) < 0) {
        return -1;  // Trả về lỗi nếu không sao chép được
    }

    return 0;  // Thành công
}
#endif

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
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
