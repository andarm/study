/*******************************************
*���ڣ�2015-04-22
*
*
*
**********************************/

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

#include "do.h"

#define GPIO_TO_PIN(bank, gpio) (32 * (bank) + (gpio))

/* am335x��GPIO�ӿ� */
extern int  gpio_request(unsigned gpio,const char *tag);
extern int  gpio_direction_input(unsigned gpio);
extern int  gpio_direction_output(unsigned gpio, int value);
extern int  gpio_get_value(unsigned gpio);
extern void gpio_set_value(unsigned gpio, int value);
extern int  gpio_cansleep(unsigned gpio);

#define	DO_MAJOR	0	// �������豸�� 0--�Զ����� �����ֶ�����
#define	DO_MINOR	0	// ������豸�� һ����0��ʼ

#define DO_CH			3
#if 1 
#define GPIO_DO_0			GPIO_TO_PIN(2, 1)
#define GPIO_DO_1			GPIO_TO_PIN(2, 0)
#define GPIO_DO_2                       GPIO_TO_PIN(0, 22) 
#else
#define GPIO_DO_0			GPIO_TO_PIN(2, 0)
#define GPIO_DO_1			GPIO_TO_PIN(2, 1)
#define GPIO_DO_2                       GPIO_TO_PIN(0, 22)
#endif 
u32 GPIO_DO[]={GPIO_DO_0,GPIO_DO_1,GPIO_DO_2};

typedef struct
{
	struct semaphore sem;	// ���������õ����ź���
	struct timer_list timer;
	u32 time_10ms;
	u32 ch;
	u32 pin_state;
} ST_DO_DEV, *LPST_DO_DEV;

typedef struct
{
	struct cdev cdev;		// �ַ��豸�ṹ��
	struct class *cls;
	struct device *dev;
	ST_DO_DEV doDev[DO_CH];
} ST_DO_DRV, *LPST_DO_DRV;

static int do_open(struct inode *inode, struct file *filp);
static int do_release(struct inode *inode, struct file *filp);
static ssize_t do_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos);

static int do_major 				        = DO_MAJOR;                      
static int do_minor 				        = DO_MINOR;                      
static int do_nr_devs				      = 1;			      // �ַ��豸�ĸ���      
static const char do_dev_name[] 	= "am335x_do";	// ע�������

ST_DO_DRV *pst_do_drv  =  NULL;

//�ļ������ṹ��
static const struct file_operations do_fops = 
{
	.owner 		= THIS_MODULE,
	.write 		= do_write,
	.open 		= do_open,
	.release 	= do_release,
};

static void timer_handler(unsigned long data)
{
	LPST_DO_DEV pdo_dev = (LPST_DO_DEV)data;
	
	if (pdo_dev->pin_state)
	{
		pdo_dev->pin_state = 0;
		gpio_set_value(GPIO_DO[pdo_dev->ch], 0);
	}
	else
	{
		pdo_dev->pin_state = 1;
		gpio_set_value(GPIO_DO[pdo_dev->ch], 1);
	}
}

static inline void do_setup_pin(void)
{
	int i;

	for(i=0;i<DO_CH;i++)
	{
		gpio_free(GPIO_DO[i]);
		gpio_request(GPIO_DO[i],NULL);
		gpio_direction_output(GPIO_DO[i], 1);// ����Ϊ������͵�ƽ
		gpio_set_value(GPIO_DO[i],1);
	}
}

static int __init am335x_do_init(void)
{
	int err;
	int i;
	dev_t devno = MKDEV(do_major, 0);
	printk(KERN_DEBUG"Register do init \n");
	/***********************�����ַ��豸************************/
	if (do_major)
	{// �ֶ�����
		err = register_chrdev_region(devno, do_nr_devs, do_dev_name);
	}
	else
	{// �ں��Զ�����
		err = alloc_chrdev_region(&devno, do_minor, do_nr_devs, do_dev_name);
		do_major = MAJOR(devno);
	}
	
	if (err < 0)
	{
		printk("am335x_do can't get the major %d\n", do_major);
		return err;
	}
	else
	{
		printk(KERN_DEBUG "am335x_do success get the major is %d\n", do_major);
	}
	
	/**************ע���ַ��豸�ľ��巽��**********************/
	pst_do_drv = kmalloc(sizeof(ST_DO_DRV), GFP_KERNEL);
	if (pst_do_drv == NULL)
	{
		err = -ENOMEM;
		goto fail_malloc;
	}
	
	memset(pst_do_drv, 0, sizeof(ST_DO_DRV));
	cdev_init(&pst_do_drv->cdev, &do_fops);	// ���ļ�����������������
	pst_do_drv->cdev.owner	= THIS_MODULE;
#if 1	
	for (i = 0; i < DO_CH; i++)
	{
		sema_init(&pst_do_drv->doDev[i].sem, 1);
		init_timer(&pst_do_drv->doDev[i].timer);
		pst_do_drv->doDev[i].ch = i;
		pst_do_drv->doDev[i].pin_state = 0;
	}
#endif 	
	//add �����з�����Ч �ڴ�֮ǰ��һ������Ӳ��
	do_setup_pin();
	err = cdev_add(&pst_do_drv->cdev, devno, 1);
	if (err < 0)
	{
		printk("cdev_add fail\n");
		goto fail_cdev_add;
	}
	
	// Ϊproc�ļ�ϵͳ ����һ���豸��
	pst_do_drv->cls = class_create(THIS_MODULE, do_dev_name);
	
	// Ϊ���� ����һ���豸
	pst_do_drv->dev = device_create(pst_do_drv->cls, NULL, 
									MKDEV(do_major, 0), NULL,
									"%s", "do_dev");
	
	return 0;

fail_cdev_add:
	kfree(pst_do_drv);
fail_malloc:
	unregister_chrdev_region(MKDEV(do_major, do_minor),
							do_nr_devs);
	return err;
}

static void __exit am335x_do_exit(void)
{
	int i;
	
	for (i = 0; i < DO_CH; i++)
	{
		del_timer(&pst_do_drv->doDev[i].timer);
	}
	
	cdev_del(&pst_do_drv->cdev);
	unregister_chrdev_region(MKDEV(do_major, do_minor),
							do_nr_devs);
	device_destroy(pst_do_drv->cls, MKDEV(do_major, 0));
	class_destroy(pst_do_drv->cls);
	
	kfree(pst_do_drv);
	printk(KERN_DEBUG "am335x_do exit...\n");
}

static int do_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int do_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t do_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos)
{
	LPST_S3C_DO_CFG pDoCfg = NULL;
	
	if (size != sizeof(ST_S3C_DO_CFG))
	{
		printk("size%d\n",sizeof(ST_S3C_DO_CFG));
		return -ENOMEM;
	}

	pDoCfg = kmalloc(sizeof(ST_S3C_DO_CFG), GFP_KERNEL);
	if (pDoCfg == NULL)
	{
		return -ENOMEM;
	}
	if (copy_from_user(pDoCfg, buf, sizeof(ST_S3C_DO_CFG)))
	{
		kfree(pDoCfg);
		return -EFAULT;
	}
	switch (pDoCfg->cmd)
	{
	case S3C_DO_NC:
	case S3C_DO_NO:
		// ���� ���տ���
		if (pDoCfg->ch >= 0 && pDoCfg->ch < DO_CH)
		{
			if (down_interruptible(&pst_do_drv->doDev[pDoCfg->ch].sem))
			{
				kfree(pDoCfg);
				return -ERESTARTSYS;
			}
			// ɾ��ԭ�е�timer ȷ��������ƻ��ں�������
			del_timer(&pst_do_drv->doDev[pDoCfg->ch].timer);
			// ʵ�ʼ̵�������
			if (pDoCfg->cmd == S3C_DO_NC)
			{
				 printk(KERN_DEBUG"DO_NC...with CH%d\n", pDoCfg->ch);
				gpio_set_value(GPIO_DO[pDoCfg->ch], 0);
			}
			else
			{
				printk(KERN_DEBUG"DO_NO...with CH%d\n", pDoCfg->ch);
				gpio_set_value(GPIO_DO[pDoCfg->ch], 1);
			}
			
			up(&pst_do_drv->doDev[pDoCfg->ch].sem);
		}
		break;
		
	case S3C_DO_PULSE_NC:
	case S3C_DO_PULSE_NO:
		// printk("S3C_DO_PULSE_NO\n");
		// ���峣�� ���տ���
		if (pDoCfg->ch >= 0 && pDoCfg->ch < DO_CH)
		{
			if (down_interruptible(&pst_do_drv->doDev[pDoCfg->ch].sem))
			{
				kfree(pDoCfg);
				return -ERESTARTSYS;
			}
			
			// ɾ��ԭ�е�timer
			del_timer(&pst_do_drv->doDev[pDoCfg->ch].timer);
			
			//������ʱ�临�Ƶ�doDev
			pst_do_drv->doDev[pDoCfg->ch].time_10ms = pDoCfg->time_10ms;
			
			// ���ô�����
			pst_do_drv->doDev[pDoCfg->ch].timer.function	= timer_handler;
			pst_do_drv->doDev[pDoCfg->ch].timer.data		= (unsigned long)&pst_do_drv->doDev[pDoCfg->ch];
			if (pDoCfg->time_10ms == 0)
				pDoCfg->time_10ms = 1;
			pst_do_drv->doDev[pDoCfg->ch].timer.expires	= jiffies + (pDoCfg->time_10ms * HZ / 100);	// 10msΪ��λ
			
			if (pDoCfg->cmd == S3C_DO_PULSE_NC)
			{// ���峣�� ������ά�ֵ�ƽΪ ����
				printk(KERN_DEBUG "DO_PULSE_NC...with CH%d\n", pDoCfg->ch);
				pst_do_drv->doDev[pDoCfg->ch].pin_state = 0;
				gpio_set_value(GPIO_DO[pDoCfg->ch], 0);
			}
			else
			{
				printk(KERN_DEBUG "DO_PULSE_NO...with CH%d\n", pDoCfg->ch);
				pst_do_drv->doDev[pDoCfg->ch].pin_state = 1;
				gpio_set_value(GPIO_DO[pDoCfg->ch], 1);
			}
			
			add_timer(&pst_do_drv->doDev[pDoCfg->ch].timer);
			
			up(&pst_do_drv->doDev[pDoCfg->ch].sem);
		}
		break;
	default:
		kfree(pDoCfg);
		return -EINVAL;
	}
	
	kfree(pDoCfg);
	return sizeof(ST_S3C_DO_CFG);
}

module_init(am335x_do_init);
module_exit(am335x_do_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("LinJinYe");
