#include <linux/module.h>

#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/major.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/tty.h>
#include <linux/kmod.h>
#include <linux/gfp.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/slab.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/poll.h> 
#define GPIO_TO_PIN(bank, gpio) (32 * (bank) + (gpio))

#define DEVICE_NAME 	"led"

//#define CONFIG_LED_DEBUG
#ifdef CONFIG_LED_DEBUG
#define dbg(format, x... )		pr_notice("led: "format, ##x)
#else
#define dbg(format, x... )
#endif

#define LED_ON		1
#define LED_OFF		0

#define LED0		0
#define LED1		1
#define LED2		2
#define LED3		3

typedef struct __GPIO_DESC {
	int gpio;
	const char *desc;
}led_desc;

static led_desc led_dev_data[] = {
	{GPIO_TO_PIN(1, 16),"LED0-1"},	/* LED0 */
	{GPIO_TO_PIN(1, 17),"LED1-1"},	/* LED1 */
};

static long led_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	
	dbg("%s called,cmd = 0x%x,arg =0x%x,\n",__func__,cmd,arg);
	if(cmd != LED_ON && cmd != LED_OFF)
	{
		printk("ioctl cmd erro!\n");
		return -1;
	}
	if(arg<1 || arg > sizeof(led_dev_data)/sizeof(led_desc))
	{
		printk("ioctl arg error!\n");
		return -1;
	}
	gpio_set_value(led_dev_data[arg-1].gpio, cmd);
	return 0;
}

static int led_open(struct inode *inode, struct file *file)
{
	int i=0;
	int ret = -1;
	dbg("%s called\n",__func__);
	for(i=0 ; i<sizeof(led_dev_data)/sizeof(led_desc) ; i++)
	{
		ret = gpio_request(led_dev_data[i].gpio,led_dev_data[i].desc);
		if(ret < 0)
		{
			printk("failed to request GPIO %d{%s}, error %d\n",	led_dev_data[i].gpio,led_dev_data[i].desc, ret);
			return ret;
		}
		gpio_direction_output(led_dev_data[i].gpio, 0);
		gpio_set_value(led_dev_data[i].gpio, 1);
	}
	return 0;

}
/*关闭设备的接口*/
static int led_close(struct inode *inode, struct file *file)
{
	int i = 0;
	dbg("Close device sucessfully\n");
	for(i=0 ; i<sizeof(led_dev_data)/sizeof(led_desc) ; i++)
	{
		gpio_free(led_dev_data[i].gpio);
	}
	return 0;
}


/*接口注册*/
static struct file_operations led_fops=
{
    .owner			=	THIS_MODULE,
    .unlocked_ioctl	=	led_ioctl,
	.open 			= 	led_open,
	.release	 	= 	led_close,
};

/*设备结构的设置*/
static struct miscdevice misc = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= DEVICE_NAME,
	.fops	= &led_fops,
};


/*设备初始化函数*/
static int __init led_t_init(void)
{
	int ret;
	ret = misc_register(&misc);
	
	if(ret!=0)
	{
		printk("init led unsuccessfully!\n");
		return -1;
	}
	printk("Register led successfully!\n");	
	return 0;
}

/*卸载函数*/
static void __exit led_t_exit(void)
{

	misc_deregister(&misc);
	printk("led  exit!\n");
} 



module_init(led_t_init);
module_exit(led_t_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Barack.Liu <tq_rd@embedsky.net>");
MODULE_DESCRIPTION("module Driver");
