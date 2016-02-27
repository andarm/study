/*******************************************
*���ڣ�2015-04-22
*
*
*
**********************************/

#include "key.h"

#define DEV_NAME "am335x_key"
#define DEV_DRV_NAME "am335x_drv_key"
#define	DEV_MAJOR	0	// �������豸�� 0--�Զ����� �����ֶ�����
#define	DEV_MINOR	0	// ������豸�� һ����0��ʼ

#define DEV_CH			1

#define GPIO_DO_0			GPIO_TO_PIN(0,27)

u32 GPIO_DO[]={GPIO_DO_0};

#define GPIO_KEY   GPIO_TO_PIN(0,27)

typedef struct
{
	struct timer_list timer;
	u32 ch;
	u32 pin_state;
} ST_DO_DEV, *LPST_DO_DEV;

typedef struct
{
	struct cdev cdev;		// �ַ��豸�ṹ��
	struct class *cls;
	struct device *dev;
	u16 dev_data[DEV_CH];
	struct rw_semaphore rw_sem;	// ���������õ����ź���
	ST_DO_DEV doDev;
} ST_DO_DRV, *LPST_DO_DRV;

static int dev_open(struct inode *inode, struct file *filp);
static int dev_release(struct inode *inode, struct file *filp);
static ssize_t dev_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos);
static ssize_t dev_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos);

static int  dev_major				        = DEV_MAJOR;                      
static int  dev_minor 				        = DEV_MINOR;                      
static int  do_nr_devs				      = 1;			      // �ַ��豸�ĸ���      
static const char do_dev_name[] 	= "am335x_key";	// ע�������

ST_DO_DRV *pst_do_drv  =  NULL;

//�ļ������ṹ��
static const struct file_operations do_fops = 
{
	.owner 		= THIS_MODULE,
	.write 		= dev_write,
	.read       = dev_read,
	.open 		= dev_open,
	.release 	= dev_release,
};


static inline void do_setup_pin(void)
{
	gpio_free(GPIO_KEY);
	gpio_request(GPIO_KEY,NULL);
	gpio_direction_input(GPIO_KEY);// ����Ϊ����
}



static int __init am335x_do_init(void)
{
	int err;
	int i;
	dev_t devno = MKDEV(dev_major, 0);
	printk("Beginning Register Key...\n");
	/***********************�����ַ��豸************************/
	if (dev_major)
	{// �ֶ�����
		err = register_chrdev_region(devno, do_nr_devs,DEV_NAME);
	}
	else
	{// �ں��Զ�����
		err = alloc_chrdev_region(&devno, DEV_MINOR, do_nr_devs,DEV_NAME);
		dev_major = MAJOR(devno);
	}
	printk("major : %d,devno:%d\n",dev_major,devno);
	
	if (err < 0)
	{
		printk("am335x_do can't get the major %d\n", DEV_MAJOR);
		return err;
	}
	else
	{
		printk(KERN_DEBUG "am335x_do success get the major is %d\n", DEV_MAJOR);
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
	init_rwsem(&pst_do_drv->rw_sem);

	// Ϊproc�ļ�ϵͳ ����һ���豸��
	pst_do_drv->cls = class_create(THIS_MODULE, DEV_NAME);
	
	// Ϊ���� ����һ���豸
	pst_do_drv->dev = device_create(pst_do_drv->cls, NULL, 
									MKDEV(dev_major, 0), NULL,
									"%s",DEV_NAME);
	printk("Finish Register Am335x Key...\n");
	return 0;

fail_cdev_add:
	kfree(pst_do_drv);
fail_malloc:
	unregister_chrdev_region(MKDEV(dev_major, dev_minor),
							do_nr_devs);
	return err;
}

static void __exit am335x_do_exit(void)
{
	printk("Exit  Am335x Key...\n");
//	del_timer(&pst_do_drv->doDev.timer);
	
	cdev_del(&pst_do_drv->cdev);
	unregister_chrdev_region(MKDEV(dev_major, dev_minor),
							do_nr_devs);
	device_destroy(pst_do_drv->cls, MKDEV(dev_major, 0));
	class_destroy(pst_do_drv->cls);
	kfree(pst_do_drv);
	printk("Exit end \n");

}

static int dev_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int dev_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t dev_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos)
{
	printk("size :%d\n",size);
#if 1
	if (size != DEV_CH*sizeof(u16))
	{
		printk("size error %d\n",size);
		return -EINVAL;
	}
	
	down_read(&pst_do_drv->rw_sem);
	if (copy_to_user(buf, pst_do_drv->dev_data, DEV_CH*sizeof(u16)))
	{
		up_read(&pst_do_drv->rw_sem);
		return -EBUSY;
	}
	else
	{
		up_read(&pst_do_drv->rw_sem);
		return (DEV_CH*sizeof(u16));
	}
#endif 
}

static ssize_t dev_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos)
{
	return 0;
}

module_init(am335x_do_init);
module_exit(am335x_do_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("LinJinYe");
