/*******************************************
*���ڣ�2015-04-22
*���������Ź�
*
*
**********************************/
#include "wdg.h"
#include "linux/delay.h"
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

#define DO_CH			1

#define GPIO_DO_0			GPIO_TO_PIN(3,8)

u32 GPIO_DO[]={GPIO_DO_0};

#define GPIO_WDG     GPIO_TO_PIN(3,8)
#define WATCHDOG_TIMER 500  // 1s feed dog
typedef struct
{
	struct semaphore sem;	// ���������õ����ź���
	struct timer_list timer;
	u32 ch;
	u32 pin_state;
} ST_DO_DEV, *LPST_DO_DEV;

typedef struct
{
	struct cdev cdev;		// �ַ��豸�ṹ��
	struct class *cls;
	struct device *dev;
	ST_DO_DEV doDev;
} ST_DO_DRV, *LPST_DO_DRV;

static int do_open(struct inode *inode, struct file *filp);
static int do_release(struct inode *inode, struct file *filp);
static ssize_t do_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos);

static int do_major 				        = DO_MAJOR;                      
static int do_minor 				        = DO_MINOR;                      
static int do_nr_devs				      = 1;			      // �ַ��豸�ĸ���      
static const char do_dev_name[] 	= "am335x_wdg";	// ע�������

ST_DO_DRV *pst_do_drv  =  NULL;

//�ļ������ṹ��
static const struct file_operations do_fops = 
{
	.owner 		= THIS_MODULE,
	.write 		= do_write,
	.open 		= do_open,
	.release 	= do_release,
};


static inline void do_setup_pin(void)
{
	gpio_free(GPIO_WDG);
	gpio_request(GPIO_WDG,NULL);
	gpio_direction_output(GPIO_WDG,1);// ����Ϊ���
}

/***************************************
 * function:
 * return :
 * author:
 * date:
 *
 * **************************************/
static void timer_handler(unsigned long data)
{
#if 1 
	LPST_DO_DEV  lpst_do_dev = (LPST_DO_DEV)data;

	if (lpst_do_dev->pin_state)
	{
		lpst_do_dev->pin_state = 0;
		gpio_set_value(GPIO_WDG, 0);
		mod_timer(&lpst_do_dev->timer, jiffies + (WATCHDOG_TIMER * HZ / 1000));
	}
	else
	{
		lpst_do_dev->pin_state = 1;
		gpio_set_value(GPIO_WDG, 1);
		mod_timer(&lpst_do_dev->timer, jiffies + ( WATCHDOG_TIMER * HZ / 1000));
	}
#endif 
}

static int __init am335x_do_init(void)
{
	int err;
	int i;
	dev_t devno = MKDEV(do_major, 0);
	
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
	
	
	//add �����з�����Ч �ڴ�֮ǰ��һ������Ӳ��
	do_setup_pin();
	err = cdev_add(&pst_do_drv->cdev, devno, 1);
	if (err < 0)
	{
		printk("cdev_add fail\n");
		goto fail_cdev_add;
	}

	pst_do_drv->doDev.pin_state  = 0 ;
	init_timer(&pst_do_drv->doDev.timer);
	pst_do_drv->doDev.timer.function	= timer_handler;
	pst_do_drv->doDev.timer.data		= (unsigned long)&pst_do_drv->doDev;
	pst_do_drv->doDev.timer.expires	= jiffies + (10 * HZ / 1000);	// 10msΪ��λ
	add_timer(&pst_do_drv->doDev.timer);

	// Ϊproc�ļ�ϵͳ ����һ���豸��
	pst_do_drv->cls = class_create(THIS_MODULE, do_dev_name);
	
	// Ϊ���� ����һ���豸
	pst_do_drv->dev = device_create(pst_do_drv->cls, NULL, 
									MKDEV(do_major, 0), NULL,
									"%s", "am335x_watchdog");
	
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
	del_timer(&pst_do_drv->doDev.timer);
	
	gpio_set_value(GPIO_WDG, 1);
	gpio_direction_input(GPIO_WDG);// ����Ϊ���
	cdev_del(&pst_do_drv->cdev);
	unregister_chrdev_region(MKDEV(do_major, do_minor),
							do_nr_devs);
	device_destroy(pst_do_drv->cls, MKDEV(do_major, 0));
	class_destroy(pst_do_drv->cls);
	kfree(pst_do_drv);
	printk(KERN_DEBUG "am335x watchdog  exit...\n");
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
	
	printk("new feed dog..3333333333..............\n");
	gpio_set_value(GPIO_WDG,0);
	
	del_timer(&pst_do_drv->doDev.timer);
	mdelay(1000);
	mdelay(1000);
	mdelay(1000);
	mdelay(1000);
	mdelay(1000);
	printk("endf.......dog..............\n");
	return 0;
}

module_init(am335x_do_init);
module_exit(am335x_do_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("LinJinYe");
