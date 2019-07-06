#include <linux/module.h>  
#include <linux/moduleparam.h>  
#include <linux/init.h>  
#include <linux/time.h>  
#include <linux/timer.h>  
#include <linux/proc_fs.h>  
#include <linux/spinlock.h>  
#include <linux/interrupt.h>  
#include <asm/hardirq.h>  
#include <linux/sched.h>//jiffies  
#include <linux/kernel.h>  
#include <linux/types.h>//u64  
#include <linux/fs.h>//file_operations, file  
#include <linux/completion.h>  
#include <asm/uaccess.h>//copy_to_user & copy_from_user  
  
int jit_currentime(char *buf, char **start, off_t offset, int len, int *eof, void *data)  
{  
    struct timeval tv1;  
    struct timespec tv2;  
    unsigned long j1;  
    u64 j2;  
  
    j1 = jiffies;  
    j2 = get_jiffies_64();  
    do_gettimeofday(&tv1);  
    tv2 = current_kernel_time();  
      
    len = 0;  
    len += sprintf(buf,"0x%08lx 0x%016Lx %10i.%06i\n"  
               "%40i.%09i\n",  
               j1, j2,  
               (int) tv1.tv_sec, (int) tv1.tv_usec,  
               (int) tv2.tv_sec, (int) tv2.tv_nsec);  
    *start = buf;  
    return len;  
}  
  
int __init jit_init(void)  
{  
    printk(KERN_DEBUG "jit_init......");  
    printk("study test...\n");
  //  create_proc_read_entry("jit_currentime", 0, NULL, jit_currentime, NULL);  
    return 0;  
}  
  
void __exit jit_exit(void)  
{  
   /// remove_proc_entry("jit_currentime", NULL);  
}  
  
MODULE_LICENSE("Dual BSD/GPL");  
module_init(jit_init);  
module_exit(jit_exit);
