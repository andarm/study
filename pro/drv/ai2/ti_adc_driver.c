/*
 * TI Touch Screen driver
 *
*/


#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/pm_runtime.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/types.h>
#include <linux/moduleparam.h>


#include <linux/fs.h>

#include <linux/device.h>
#include <linux/semaphore.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>

#include <asm/io.h>
#include <asm/delay.h>




#define TSCADC_REG_IRQEOI		0x020
#define TSCADC_REG_RAWIRQSTATUS		0x024
#define TSCADC_REG_IRQSTATUS		0x028
#define TSCADC_REG_IRQENABLE		0x02C
#define TSCADC_REG_IRQCLR		0x030
#define TSCADC_REG_IRQWAKEUP		0x034
#define TSCADC_REG_CTRL			0x040
#define TSCADC_REG_ADCFSM		0x044
#define TSCADC_REG_CLKDIV		0x04C
#define TSCADC_REG_SE			0x054
#define TSCADC_REG_IDLECONFIG		0x058
#define TSCADC_REG_CHARGECONFIG		0x05C
#define TSCADC_REG_CHARGEDELAY		0x060
#define TSCADC_REG_STEPCONFIG(n)	(0x64 + ((n-1) * 8))
#define TSCADC_REG_STEPDELAY(n)		(0x68 + ((n-1) * 8))
#define TSCADC_REG_STEPCONFIG13		0x0C4
#define TSCADC_REG_STEPDELAY13		0x0C8
#define TSCADC_REG_STEPCONFIG14		0x0CC
#define TSCADC_REG_STEPDELAY14		0x0D0
#define TSCADC_REG_FIFO0CNT		0xE4
#define TSCADC_REG_FIFO0THR		0xE8
#define TSCADC_REG_FIFO1CNT		0xF0
#define TSCADC_REG_FIFO1THR		0xF4
#define TSCADC_REG_FIFO0		0x100
#define TSCADC_REG_FIFO1		0x200


/*	Register Bitfields	*/
#define TSCADC_IRQWKUP_ENB		BIT(0)
#define TSCADC_IRQWKUP_DISABLE		0x00
#define TSCADC_STPENB_STEPENB		0x7FFF
#define TSCADC_IRQENB_FIFO0THRES	BIT(2)

#define ADC_STPENB_STEPENB		0x1FE 
#define TSCADC_IRQENB_STEPEND	BIT(1) 
#define TSCADC_IRQENB_FIFO1THRES	BIT(5)
#define TSCADC_IRQENB_PENUP		BIT(9)
#define TSCADC_IRQENB_HW_PEN		BIT(0)
#define TSCADC_STEPCONFIG_MODE_HWSYNC	0x2
#define TSCADC_STEPCONFIG_2SAMPLES_AVG	(1 << 4)
#define TSCADC_STEPCONFIG_XPP		BIT(5)
#define TSCADC_STEPCONFIG_XNN		BIT(6)
#define TSCADC_STEPCONFIG_YPP		BIT(7)
#define TSCADC_STEPCONFIG_YNN		BIT(8)
#define TSCADC_STEPCONFIG_XNP		BIT(9)
#define TSCADC_STEPCONFIG_YPN		BIT(10)

#define TSCADC_STEPCONFIG_1_NEGATIVE_INP (0)		
#define TSCADC_STEPCONFIG_1_INP		(0) 
#define TSCADC_STEPCONFIG_2_INP		BIT(19) 
#define TSCADC_STEPCONFIG_3_INP		BIT(20) 
#define TSCADC_STEPCONFIG_4_INP		(BIT(19)|BIT(20)) 
#define TSCADC_STEPCONFIG_5_INP		BIT(21) 
#define TSCADC_STEPCONFIG_6_INP		(BIT(19)|BIT(21))  
#define TSCADC_STEPCONFIG_7_INP		(BIT(21)|BIT(20))  
#define TSCADC_STEPCONFIG_8_INP		(BIT(19)|BIT(20)|BIT(21))
#define TSCADC_STEPCONFIG_REFP		(BIT(12)|BIT(13))

#define TSCADC_STEPCONFIG_RFM	((1 << 23)|(1 << 24)) 
#define TSCADC_STEPCONFIG_SINGLE_ENDED_OPER_MODE (0 <<25)
#define TSCADC_STEPCONFIG_MODE   (0)
#define TSCADC_STEPCONFIG_RFP		(1 << 12)
#define TSCADC_STEPCONFIG_INM		(1 << 18)
#define TSCADC_STEPCONFIG_INP_4		(1 << 19)
#define TSCADC_STEPCONFIG_INP		(1 << 20)
#define TSCADC_STEPCONFIG_INP_5		(1 << 21)
#define TSCADC_STEPCONFIG_FIFO1		(1 << 26)
#define TSCADC_STEPCONFIG_IDLE_INP	(1 << 22)
#define TSCADC_STEPCONFIG_OPENDLY	0x0
#define TSCADC_STEPCONFIG_SAMPLEDLY	0x0


#define TSCADC_STEPCONFIG_Z1		(3 << 19)

#define TSCADC_CNTRLREG_TSCSSENB	BIT(0)
#define TSCADC_CNTRLREG_STEPID		BIT(1)
#define TSCADC_CNTRLREG_STEPCONFIGWRT	BIT(2)
#define TSCADC_CNTRLREG_TSCENB		BIT(7)
#define TSCADC_CNTRLREG_4WIRE		(0x1 << 5)
#define TSCADC_CNTRLREG_5WIRE		(0x1 << 6)
#define TSCADC_CNTRLREG_8WIRE		(0x3 << 5)
#define TSCADC_ADCFSM_STEPID		0x10
#define TSCADC_ADCFSM_FSM		BIT(5)

#define ADC_CLK				3000000

#define MAX_12BIT                       ((1 << 12) - 1)

static volatile int ev_adc = 0;

static DECLARE_WAIT_QUEUE_HEAD(adc_waitq);
static int adc_data=0;

#define DEV_NAME "am335x_adc"
static int dev_major 			= 0;
static int dev_minor 			= 0;
static int dev_nr_devs			= 1;			// 字符设备的个数
static const char dev_name[] 	= "am335x_adc";	// 注册的名称



typedef struct {
	struct class *cls;
	struct device *dev;  
	struct cdev	adc;   
	int			irq;
	void __iomem		*adc_base;  

	
}adc_st,*LPST_ADC_ST;

static int adc_major = 0;

struct adc_st *adc_dev = NULL;

static int channel = 4;

module_param(channel, int, S_IRUGO);


static unsigned int adc_readl(struct adc_st *adc, unsigned int reg)
{
	return readl(adc->adc_base + reg);
}

static void adc_writel(struct adc_st *adc , unsigned int reg,
					unsigned int val)
{
	writel(val, adc->adc_base + reg);
}

static void start_adc(struct adc_st *dev)
{
   printk("ADC start!\n");
   int ctrl=0;
   ctrl |= TSCADC_CNTRLREG_STEPCONFIGWRT |			
			TSCADC_CNTRLREG_TSCSSENB;
   adc_writel(dev, TSCADC_REG_CTRL, ctrl); 
   
}

static void stop_adc(struct adc_st *dev)
{
   printk("ADC stop!\n");
   int ctrl=0;
   ctrl |= TSCADC_CNTRLREG_STEPCONFIGWRT;
   adc_writel(dev, TSCADC_REG_CTRL, ctrl); 
   
}



static void adc_step_config(struct adc_st *adc_dev)
{
    	unsigned int	stepconfigx = 0;
	unsigned int	delay;
	int i;

	/* Configure the Step registers */
    stepconfigx =  TSCADC_STEPCONFIG_REFP |  TSCADC_STEPCONFIG_RFM    
                    | TSCADC_STEPCONFIG_SINGLE_ENDED_OPER_MODE |TSCADC_STEPCONFIG_MODE
                    ;
	//Steven: this can choose channel  !!!   
     switch (channel)
	{
	case 0:
		stepconfigx |= TSCADC_STEPCONFIG_1_INP;
		printk("ADC channel choose: 0 \n");
		break;
	case 1:
		stepconfigx |= TSCADC_STEPCONFIG_2_INP;
		printk("ADC channel choose: 1 \n");
		break;
	case 2:
		stepconfigx |= TSCADC_STEPCONFIG_3_INP;
		printk("ADC channel choose: 2 \n");
		break;
	case 3:
		stepconfigx |= TSCADC_STEPCONFIG_4_INP;
		printk("ADC channel choose: 3 \n");
		break;
	case 4:
		stepconfigx |= TSCADC_STEPCONFIG_5_INP;
		printk("ADC channel choose: 4 \n");
		break;
	case 5:
		stepconfigx |= TSCADC_STEPCONFIG_6_INP;
		printk("ADC channel choose: 5 \n");
		break;
	case 6:
		stepconfigx |= TSCADC_STEPCONFIG_7_INP;
		printk("ADC channel choose: 6 \n");
		break;
	case 7:
		stepconfigx |= TSCADC_STEPCONFIG_8_INP;
		printk("ADC channel choose: 7 \n");
		break;
	default:
		stepconfigx |= TSCADC_STEPCONFIG_5_INP;//default for channel 4
		printk("Input wrong!  Channel number should be [0..7] \n ADC channel choose: 4 \n");
		channel = 4;
		break;

	}

	for (i = 1; i < 9; i++) {
           adc_writel(adc_dev, TSCADC_REG_STEPCONFIG(i), stepconfigx);
           }
	delay = TSCADC_STEPCONFIG_SAMPLEDLY | TSCADC_STEPCONFIG_OPENDLY;
	for (i = 1; i < 9; i++) {    
		adc_writel(adc_dev, TSCADC_REG_STEPDELAY(i), delay);
		}
	

	adc_writel(adc_dev, TSCADC_REG_SE, ADC_STPENB_STEPENB); 
	// steven:this affect which step configs can be used!!! Important!!!
}

static void adc_idle_config(struct adc_st *adc_config)
{
  /* Idle mode adc screen config */
	unsigned int	 idleconfig=0;	

	adc_writel(adc_config, TSCADC_REG_IDLECONFIG, idleconfig);
}

static int adc_open(struct inode *inode, struct file *file)
{
    int ret;
    printk("adc_open enter...\n");

    return 0;
}


static ssize_t adc_read(struct file *filp, char *buffer, size_t count, loff_t *ppos)
{
    printk("=========== Now ADC channel:%d working===========\n", channel);
    if(!ev_adc)
    {
        if(filp->f_flags & O_NONBLOCK)
        {
            return -EAGAIN;
        }
        else
        {
            start_adc(adc_dev);

            wait_event_interruptible(adc_waitq, ev_adc);
        }
    }

    ev_adc = 0;

    copy_to_user(buffer, (char *)&adc_data, sizeof(adc_data));

    adc_data=0;
    return sizeof(adc_data);
}

static int adc_release(struct inode *inode, struct file *filp)
{
    
    return 0;
}


// static struct file_operations adc_fops = 
// {
//     .owner    = THIS_MODULE,
//     .open     = adc_open,
//     .read     = adc_read,    
//     .release  = adc_release,
// };


static int adc_setup_cdev(struct adc_st *dev, int index)
{
//     int err, devno = MKDEV(adc_major, index);
//     cdev_init(&dev->adc, &adc_fops);
//     dev->adc.owner = THIS_MODULE;
//     err = cdev_add(&dev->adc,  devno, 1);
//     if(err)
//     {
//        printk("no memory resource defined.\n"); 
//        return err;
//     }
// }
#if 0 
static irqreturn_t adc_interrupt(int irq, void *dev)
{
	

    unsigned int		status, irqclr = 0;
	int			i;
	int		fifo0count = 0;
	unsigned int		readx1 = 0;
    printk("adc_interrupt enter, ev_adc %d...\n", ev_adc);    
    if(!ev_adc)      
    {        
    	status = adc_readl(adc_dev, TSCADC_REG_IRQSTATUS);
         printk("adc_interrupt status %d...\n", status);
    	if (status & TSCADC_IRQENB_FIFO0THRES) {
    		fifo0count =adc_readl(adc_dev, TSCADC_REG_FIFO0CNT);
    		
            printk("ADC_Interrupt FIFO0_Count %d...\n", fifo0count);
    		for (i = 0; i < fifo0count; i++) {
    			readx1 = adc_readl(adc_dev, TSCADC_REG_FIFO0);
                printk("FIFO0_count %d : %d\n", i, readx1);
                adc_data+=readx1;
    			
    		}
            if(fifo0count)
                adc_data = adc_data/fifo0count;
            printk("ADC_DATA: %d \n", adc_data);
    		
    		irqclr |= TSCADC_IRQENB_FIFO0THRES;
    	}
        stop_adc(adc_dev);
        
        //clear TSCADC_IRQENB_FIFO0THRES irq
    	adc_writel(adc_dev, TSCADC_REG_IRQSTATUS, irqclr);


    	/* check pending interrupts */
    	adc_writel(adc_dev, TSCADC_REG_IRQEOI, 0x0);

    	adc_writel(adc_dev, TSCADC_REG_SE, ADC_STPENB_STEPENB);
    	

        ev_adc=1;
        
        //stop_adc();
        
        wake_up_interruptible(&adc_waitq);

        
    }
	return IRQ_HANDLED;
    
}
#endif 	

//文件操作结构体
static const struct file_operations dev_fops = 
{
	.owner 		= THIS_MODULE,
	.read 		= adc_read,
	.open 		= adc_open,
	.release 	= adc_release,
};

/*
* The functions for inserting/removing driver as a module.
*/
#if 0 
static	int __devinit adc_probe(struct platform_device *pdev)
{
	int				err;
	int				clk_value;
	int				clock_rate, irqenable, ctrl;
	struct resource			*res;
	struct clk			*clk;
    	dev_t devno;
    	int ret;
      
       printk("start probe....\n"); 
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "no memory resource defined.\n");
		return -EINVAL;
	}

	/* Allocate memory for device */
	adc_dev = kzalloc(sizeof(struct adc_st), GFP_KERNEL);
	if (!adc_dev) {
		dev_err(&pdev->dev, "failed to allocate memory.\n");
		return -ENOMEM;
	}

     ret = alloc_chrdev_region(&devno, // pointer to where the device number to be stored
        0, // first minor number requested
        1, // number of devices
        "adc"); // device name

    if(ret < 0)
    {
        err  = ret;
        goto err_free_mem;
    }   
    adc_major = MAJOR(devno);
    printk("adc_probe adc_major %d\n", adc_major);
    ret = adc_setup_cdev(adc_dev, 0);
    if(ret)
    {
        err = ret;
        goto err_alloc_chrdev;
    }    

	adc_dev->irq = platform_get_irq(pdev, 0);
	if (adc_dev->irq < 0) {
		dev_err(&pdev->dev, "no irq ID is specified.\n");
		err =  -ENODEV;
        goto err_add_cdev;
	}
	printk("irq ID is : %d\n",adc_dev->irq);
	res =  request_mem_region(res->start, resource_size(res), pdev->name);
	if (!res) {
		dev_err(&pdev->dev, "failed to reserve registers.\n");
		err = -EBUSY;
		goto err_add_cdev;
	}

	adc_dev->adc_base = ioremap(res->start, resource_size(res));
	if (!adc_dev->adc_base) {
		dev_err(&pdev->dev, "failed to map registers.\n");
		err = -ENOMEM;
		goto err_release_mem;
	}

	err = request_irq(adc_dev->irq, adc_interrupt, IRQF_DISABLED,
				pdev->dev.driver->name, adc_dev);
	if (err) {
		dev_err(&pdev->dev, "failed to allocate irq.\n");
		goto err_unmap_regs;
	}

	pm_runtime_enable(&pdev->dev);
	pm_runtime_get_sync(&pdev->dev);

	clk = clk_get(&pdev->dev, "adc_tsc_fck");
	if (IS_ERR(clk)) {
		dev_err(&pdev->dev, "failed to get TSC fck\n");
		err = PTR_ERR(clk);
		goto err_free_irq;
	}
	clock_rate = clk_get_rate(clk);
	clk_value = clock_rate / ADC_CLK;
	if (clk_value < 7) {
		dev_err(&pdev->dev, "clock input less than min clock requirement\n");
		err = -EINVAL;
		goto err_fail;
	}
	/* TSCADC_CLKDIV needs to be configured to the value minus 1 */
	clk_value = clk_value - 1;
	adc_writel(adc_dev, TSCADC_REG_CLKDIV, clk_value);


	/* Set the control register bits */
	ctrl = TSCADC_CNTRLREG_STEPCONFIGWRT |  TSCADC_CNTRLREG_TSCENB ;
	adc_writel(adc_dev, TSCADC_REG_CTRL, ctrl);    

	/* Set register bits for Idel Config Mode */
	adc_idle_config(adc_dev);

    	adc_step_config(adc_dev);

    
	/* IRQ Enable */
    	irqenable = TSCADC_IRQENB_FIFO0THRES;   
    	adc_writel(adc_dev,TSCADC_REG_IRQENABLE, irqenable);
	adc_writel(adc_dev, TSCADC_REG_FIFO0THR, 7);//FIFO THRESHOLD SET 0-7
	//test

	printk("testing..........########\n");
    	int status = adc_readl(adc_dev, TSCADC_REG_IRQSTATUS);
	device_init_wakeup(&pdev->dev, true);
    printk("adc initialized!\n");
	return 0;

err_fail:
	pm_runtime_disable(&pdev->dev);
err_free_irq:
	free_irq(adc_dev->irq, adc_dev);
err_unmap_regs:
	iounmap(adc_dev->adc_base);
err_release_mem:
	release_mem_region(res->start, resource_size(res));	
err_add_cdev:
    cdev_del(&adc_dev->adc); 
err_alloc_chrdev:
    unregister_chrdev_region(MKDEV(adc_major, 0), 1);
err_free_mem:
	kfree(adc_dev);
	return err;
}
#endif 
#if 0 
static int __devexit adc_remove(struct platform_device *pdev)
{
	
	struct resource		*res;

    cdev_del(&adc_dev->adc); 
    unregister_chrdev_region(MKDEV(adc_major, 0), 1);
    
    free_irq(adc_dev->irq, adc_dev);
	

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	iounmap(adc_dev->adc_base);
	release_mem_region(res->start, resource_size(res));

	pm_runtime_disable(&pdev->dev);

	kfree(adc_dev);

	device_init_wakeup(&pdev->dev, 0);
	platform_set_drvdata(pdev, NULL);
	return 0;
}
#endif 

#if 0 
static struct platform_driver ti_adc_driver = {
	.probe	  = adc_probe,
	.remove	 = __devexit_p(adc_remove),
	.driver	 = {
		.name   = "my_tsc",
		.owner  = THIS_MODULE,
	},	
};
#endif 


static int __init adc_init(void)
{
	int err;
	int i;
	dev_t devno = MKDEV(dev_major, 0);
	
	/***********************申请字符设备************************/
	if (dev_major)
	{// 手动申请
		err = register_chrdev_region(devno, dev_nr_devs, DEV_NAME);
	}
	else
	{// 内核自动申请
		err = alloc_chrdev_region(&devno, dev_minor, dev_nr_devs, DEV_NAME);
		dev_major = MAJOR(devno);
	}

	if (err < 0)
	{
		printk("am335x_adc can't get the major %d\n", dev_major);
		return err;
	}
	else
	{
		printk(KERN_DEBUG "am335x_adc success get the major is %d\n", dev_major);
	}
	
	/**************注册字符设备的具体方法**********************/
	adc_dev = kmalloc(sizeof(adc_st), GFP_KERNEL);
	if (adc_dev == NULL)
	{
		err = -ENOMEM;
		//goto fail_malloc;
	}
 	
	memset(adc_dev, 0, sizeof(adc_st));
	cdev_init(&adc_dev->cdev, &dev_fops);	// 与文件操作函数关联起来
	adc_dev->cdev.owner	= THIS_MODULE;


	// add 后所有方法生效 在此之前进一步设置硬件
	// di_setup_pin();
	err = cdev_add(&adc_dev->cdev, devno, 1);
	
	if (err < 0)
	{
		printk("cdev_add fail\n");
		goto fail_cdev_add;
	}
	
	// 为proc文件系统 创建一个设备类
	adc_dev->cls = class_create(THIS_MODULE, dev_name);
	
	// 为该类 创建一个设备
	adc_dev->dev = device_create(adc_dev->cls, NULL, MKDEV(dev_major, 0), NULL,"%s", DEV_NAME);


	return 0;

fail_cdev_add:
	kfree(adc_dev);
fail_malloc:
	unregister_chrdev_region(MKDEV(dev_major, dev_minor),
							dev_nr_devs);
	return err;
}

static void __exit adc_exit(void)
{
	// del_timer(&adc_dev->timer);
	
	cdev_del(&adc_dev->cdev);
	unregister_chrdev_region(MKDEV(dev_major, dev_minor),
							dev_nr_devs);
	device_destroy(adc_dev->cls, MKDEV(dev_major, 0));
	class_destroy(adc_dev->cls);
	
	kfree(adc_dev);

}


// module_init(adc_init);
// module_exit(adc_exit);
module_init(adc_init);
module_exit(adc_exit);
MODULE_DESCRIPTION("TI ADC controller driver");
MODULE_AUTHOR("Ti");
MODULE_LICENSE("GPL");
