
/***********************************************************************************  
 *       Copyright Saftop Technologies Co., Ltd. 1998-2015. All rights reserved.   
 *       File name:     di.c
 *       Author:        linjinye 
 *       Version:       
 *       Date:          2015-10-12
 *       Dicription:    干接点输入驱动
 *       History:
 *                                                        
 *****************************************************************************/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/semaphore.h>
#include <linux/sched.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/delay.h>

#include "di.h"

#define GPIO_TO_PIN(bank, gpio) (32 * (bank) + (gpio))

/* am335x的GPIO接口 */
extern int  gpio_request(unsigned gpio,const char *tag);
extern int  gpio_direction_input(unsigned gpio);
extern int  gpio_direction_output(unsigned gpio, int value);
extern int  gpio_get_value(unsigned gpio);
extern void gpio_set_value(unsigned gpio, int value);
extern int  gpio_cansleep(unsigned gpio);


#define	DI_MAJOR	0	// 定义主设备号 0--自动分配 非零手动分配
#define	DI_MINOR	0	// 定义次设备号 一般由0开始

#define DI_CH		3

#define GPIO_DI_0		GPIO_TO_PIN(1, 17)
#define GPIO_DI_1		GPIO_TO_PIN(1, 18)
#define GPIO_DI_2		GPIO_TO_PIN(1, 19)

u32 GPIO_DI[ ] = {
	GPIO_DI_0,
	GPIO_DI_1,
	GPIO_DI_2
};

typedef struct
{
	struct cdev cdev;		// 字符设备结构体
	struct class *cls;
	struct device *dev;
	struct rw_semaphore rw_sem;
	struct timer_list timer;
	struct work_struct di_work;
	u16 di_data[DI_CH];
	u16 di_bak_data[DI_CH];
	u32 di_count[DI_CH];
} ST_DI_DRV, *LPST_DI_DRV;

static int di_open(struct inode *inode, struct file *filp);
static int di_release(struct inode *inode, struct file *filp);
static ssize_t di_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos);

static int di_major 			= DI_MAJOR;
static int di_minor 			= DI_MINOR;
static int di_nr_devs			= 1;			// 字符设备的个数
static const char di_dev_name[] 	= "am335x_di";	// 注册的名称

ST_DI_DRV *pst_di_drv = NULL;

//文件操作结构体
static const struct file_operations di_fops = 
{
	.owner 		= THIS_MODULE,
	.read 		= di_read,
	.open 		= di_open,
	.release 	= di_release,
};

static inline u16 to_real_di_val(u32 ch)
{
	if (ch >= 0 && ch < DI_CH)
	{
		if (gpio_get_value(GPIO_DI[ch]))
		{
			return 500;
		}
		else
		{
			return 0;
		}
	}
	else
	{// 超出范围 返回高电平
		return 500;
	}
}

static void di_work_handler(struct work_struct *work)
{
	int i;
	LPST_DI_DRV pdi_drv = container_of(work, ST_DI_DRV, di_work);
	
	for (i = 0; i < DI_CH; i++)
	{
		pdi_drv->di_bak_data[i] = to_real_di_val(i);
		
		// 获取读锁
		down_read(&pdi_drv->rw_sem);
		if (pdi_drv->di_bak_data[i] != pdi_drv->di_data[i])
		{
			up_read(&pdi_drv->rw_sem);
			if (pdi_drv->di_count[i])
			{
				pdi_drv->di_count[i] -= 1;
				if (pdi_drv->di_count[i] == 0)
				{// 更新di_data
					down_write(&pdi_drv->rw_sem);
					pdi_drv->di_data[i] = pdi_drv->di_bak_data[i];
					up_write(&pdi_drv->rw_sem);
					pdi_drv->di_count[i] = 10;
				}
			}
		}
		else
		{
			up_read(&pdi_drv->rw_sem);
			pdi_drv->di_count[i] = 10;
		}
	}
}

static void timer_handler(unsigned long data)
{
	LPST_DI_DRV pdi_drv = (LPST_DI_DRV)data;
	
	schedule_work(&pdi_drv->di_work);
	mod_timer(&pdi_drv->timer, jiffies + (10 * HZ / 1000));
}

static inline void di_setup_pin(void)
{// 3路DI输入
	gpio_request(GPIO_DI_0,NULL);
	gpio_request(GPIO_DI_1,NULL);
	gpio_request(GPIO_DI_2,NULL);
	gpio_direction_input(GPIO_DI_0);
	gpio_direction_input(GPIO_DI_1);
	gpio_direction_input(GPIO_DI_2);
}

static int di_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int di_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t di_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos)
{
	if (size != DI_CH*sizeof(u16))
	{
		printk("size error %d\n",size);
		return -EINVAL;
	}
	
	down_read(&pst_di_drv->rw_sem);
	if (copy_to_user(buf, pst_di_drv->di_data, DI_CH*sizeof(u16)))
	{
		up_read(&pst_di_drv->rw_sem);
		return -EBUSY;
	}
	else
	{
		up_read(&pst_di_drv->rw_sem);
		return (DI_CH*sizeof(u16));
	}
}

static int __init di_init(void)
{
	int err;
	int i;
	dev_t devno = MKDEV(di_major, 0);
	
	/***********************申请字符设备************************/
	if (di_major)
	{// 手动申请
		err = register_chrdev_region(devno, di_nr_devs, di_dev_name);
	}
	else
	{// 内核自动申请
		err = alloc_chrdev_region(&devno, di_minor, di_nr_devs, di_dev_name);
		di_major = MAJOR(devno);
	}

	if (err < 0)
	{
		printk("am335x_di can't get the major %d\n", di_major);
		return err;
	}
	else
	{
		printk(KERN_DEBUG "am335x_di success get the major is %d\n", di_major);
	}
	
	/**************注册字符设备的具体方法**********************/
	pst_di_drv = kmalloc(sizeof(ST_DI_DRV), GFP_KERNEL);
	if (pst_di_drv == NULL)
	{
		err = -ENOMEM;
		//goto fail_malloc;
	}
 	
	memset(pst_di_drv, 0, sizeof(ST_DI_DRV));
	cdev_init(&pst_di_drv->cdev, &di_fops);	// 与文件操作函数关联起来
	pst_di_drv->cdev.owner	= THIS_MODULE;
	init_rwsem(&pst_di_drv->rw_sem);
	INIT_WORK(&pst_di_drv->di_work, di_work_handler);
	init_timer(&pst_di_drv->timer);

	// add 后所有方法生效 在此之前进一步设置硬件
	di_setup_pin();

	for (i = 0; i < DI_CH; i++)
	{// 生效前先读入原始值
		pst_di_drv->di_data[i]		= to_real_di_val(i);
		pst_di_drv->di_bak_data[i]	= to_real_di_val(i);
		pst_di_drv->di_count[i]		= 10;
	}
	err = cdev_add(&pst_di_drv->cdev, devno, 1);
	
	if (err < 0)
	{
		printk("cdev_add fail\n");
		goto fail_cdev_add;
	}
	
	// 为proc文件系统 创建一个设备类
	pst_di_drv->cls = class_create(THIS_MODULE, di_dev_name);
	
	// 为该类 创建一个设备
	pst_di_drv->dev = device_create(pst_di_drv->cls, NULL, MKDEV(di_major, 0), NULL,"%s", "di_dev");
	pst_di_drv->timer.function	= timer_handler;
	pst_di_drv->timer.data		= (unsigned long)pst_di_drv;
	pst_di_drv->timer.expires	= jiffies + (8 * HZ / 1000);	// 8ms为单位
	add_timer(&pst_di_drv->timer);

	return 0;

fail_cdev_add:
	kfree(pst_di_drv);
fail_malloc:
	unregister_chrdev_region(MKDEV(di_major, di_minor),
							di_nr_devs);
	return err;
}

static void __exit di_exit(void)
{
	del_timer(&pst_di_drv->timer);
	
	cdev_del(&pst_di_drv->cdev);
	unregister_chrdev_region(MKDEV(di_major, di_minor),
							di_nr_devs);
	device_destroy(pst_di_drv->cls, MKDEV(di_major, 0));
	class_destroy(pst_di_drv->cls);
	
	kfree(pst_di_drv);
	printk(KERN_DEBUG "am335x_di exit...\n");
}

module_init(di_init);
module_exit(di_exit);
MODULE_LICENSE("GPL");
