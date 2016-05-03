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
  
 int getCurrentime( )  
{  
    struct timeval tv1;  
    unsigned long j1;  
    
    struct tm curTime;
    int k_time;
    j1 = jiffies;  


    do_gettimeofday(&tv1);  
    printk("show value sec:%d\n",tv1.tv_sec);
      
    time_to_tm(tv1.tv_sec,0,&curTime);
    k_time = curTime.tm_year+1900;
//    k_time = curTime.tm_min;
    printk("show year:%ld,month:%d,day:%d,min=%d\n",curTime.tm_year+1900,curTime.tm_mon+1,curTime.tm_mday,curTime.tm_min);
    if(k_time >=2016)
    {
	printk("kill me############################*************************************************************\n");
///	system("reboot");
    }
//    sleep(1);
    return 1;  
} 
int __init sysRTC_init(void)  
{  
   getCurrentime();
    return 0;  
}  
  
void __exit sysRTC_exit(void)  
{  
    printk("Exit .......\n");
}  
  
MODULE_LICENSE("Dual BSD/GPL");  
module_init(sysRTC_init);  
module_exit(sysRTC_exit);
