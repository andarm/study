/***********************************************************************************  
 * Copyright Saftop Technologies Co., Ltd. 1998-2015. All rights reserved.  
 *  File name:     saftop_in.c
 *  Author:        linjinye
 *  Version:      
 *  Date:          2015-10-12
 *  Dicription:    干接点输入驱动
 *  History:
 *                                                 
 *          *****************************************************************************/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
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


#include <linux/spi/spi.h>
#include <linux/spi/flash.h>
#include <asm/gpio.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/delay.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/clk.h>
#include <linux/interrupt.h>


#include "ai.h"

#define BAK_ADC_1 240 //这是ADC比例放大系数. 原始采集值*BAK_ADC 对应为实际值.
#define BAK_ADC_2 202
#define BAK_ADC_3 203
#define BAK_30_44  2000
#define BAK_45_82  4095


static int read_ad_val(u16 ch);

#define AD_MAJOR			   0	// 定义主设备号 0--自动分配 非零手动分配
#define AD_MINOR			   0	// 定义次设备号 一般由0开始

#define AD_TEMP_NUM     20  // 采集多少个缓存数据
#define AD_CH           3  // 采集多少个通道数据
#define ABANDON_NUM     1  // 丢弃最大和最小值各ABANDON_NUM个，AD_TEMP_NUM > 2 * ABANDON_NUM

static int ad_major 		= AD_MAJOR;	//主设备号
static int ad_minor 		= AD_MINOR;	//次设备号
static int ad_nr_devs		= 1;		// 字符设备的个数

static const char ad_dev_name[] = "am335x_adc";	// 注册的名称

struct tscadc {
        int         irq;
    void __iomem		*tsc_base;
};

typedef struct
{
    struct cdev cdev;		// 字符设备结构体
    struct class *cls;
    struct device *dev;
    struct rw_semaphore rw_sem;
    struct timer_list timer;
    struct work_struct adc_calc_work;

    u16 ch_index;
    u16 ch_temp_pt;
    u16 temp_ai_data[AD_TEMP_NUM];			    // 采集缓冲buff
    u16 ai_data[AD_CH];					        // 2路采集通道数据
    u16 is_allow_read;					        // 是否允许读取
}ST_AD_DRV, *LPST_AD_DRV;

struct tscadc *ts_dev =NULL;

ST_AD_DRV *pst_ad_drv = NULL;
static int channel = 3 ;
static volatile int ev_adc = 0;
static DECLARE_WAIT_QUEUE_HEAD(adc_waitq);
static u16 adc_data=0;



static unsigned int tscadc_readl(struct tscadc *ts, unsigned int reg)
{
    return readl(ts->tsc_base + reg);
}

static void tscadc_writel(struct tscadc *tsc, unsigned int reg,unsigned int val)
{
    writel(val, tsc->tsc_base + reg);
}

static void start_adc(struct tscadc *dev)
{
   printk("ADC start!\n");
   int ctrl=0;
   ctrl |= TSCADC_CNTRLREG_STEPCONFIGWRT |          
            TSCADC_CNTRLREG_TSCSSENB;
   tscadc_writel(dev, TSCADC_REG_CTRL, ctrl); 
   
}

static void stop_adc(struct tscadc *dev)
{
   printk("ADC stop!\n");
   int ctrl=0;
   ctrl |= TSCADC_CNTRLREG_STEPCONFIGWRT;
   tscadc_writel(dev, TSCADC_REG_CTRL, ctrl); 
   
}

static void adc_step_config(struct tscadc *adc_dev,int ch)
{
        unsigned int    stepconfigx = 0;
    unsigned int    delay;
    int i;
#if  1
    /* Configure the Step registers */
    stepconfigx =  TSCADC_STEPCONFIG_REFP |  TSCADC_STEPCONFIG_RFM    
                    | TSCADC_STEPCONFIG_SINGLE_ENDED_OPER_MODE |TSCADC_STEPCONFIG_MODE
                    ;
    //Steven: this can choose channel  !!!   
     switch (ch)
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
           tscadc_writel(adc_dev, TSCADC_REG_STEPCONFIG(i), stepconfigx);
           }
    delay = TSCADC_STEPCONFIG_SAMPLEDLY | TSCADC_STEPCONFIG_OPENDLY;
    for (i = 1; i < 9; i++) {    
        tscadc_writel(adc_dev, TSCADC_REG_STEPDELAY(i), delay);
        }
    

    tscadc_writel(adc_dev, TSCADC_REG_SE, ADC_STPENB_STEPENB); 
    // steven:this affect which step configs can be used!!! Important!!!
    pr_debug("################finish config val################\n");
        /* IRQ Enable */
  #if 0
    int irqenable;
    irqenable = TSCADC_IRQENB_FIFO0THRES;   
    tscadc_writel(ts_dev,TSCADC_REG_IRQENABLE, irqenable);
    tscadc_writel(ts_dev, TSCADC_REG_FIFO0THR, 7);//FIFO THRESHOLD SET 0-7
   #endif 

#endif 
}

static int read_ch_val(int ch)
{
	    unsigned int        status, irqclr = 0;
    int         i;
    int     fifo0count = 0;
    unsigned int        ret_val = 0;
    pr_debug("read val  ch : %d\n",ch);    
   
    adc_step_config(ts_dev,ch);
    start_adc(ts_dev);
    mdelay(1);
	 status = tscadc_readl(ts_dev, TSCADC_REG_IRQSTATUS);
     pr_debug("adc_interrupt status %d...\n", status);
        //if (status & TSCADC_IRQENB_FIFO0THRES) 
        {
            fifo0count =tscadc_readl(ts_dev, TSCADC_REG_FIFO0CNT);
            
            pr_debug("ADC_Interrupt FIFO0_Count %d...\n", fifo0count);
            for (i = 0; i < fifo0count; i++) {
                pst_ad_drv->temp_ai_data[i] = tscadc_readl(ts_dev, TSCADC_REG_FIFO0);
                pr_debug("FIFO0_count %d : %d\n", i, pst_ad_drv->temp_ai_data[i] );
            //    adc_data+=readx1;
                
            }

           	ret_val = ad_filter(pst_ad_drv->temp_ai_data,fifo0count,0);
        //    if(fifo0count)
         //       adc_data = adc_data/fifo0count;
            pr_debug("ADC_DATA: %d \n", ret_val);
            
            irqclr |= TSCADC_IRQENB_FIFO0THRES;
        }
        stop_adc(ts_dev);
        
        //clear TSCADC_IRQENB_FIFO0THRES irq
        tscadc_writel(ts_dev, TSCADC_REG_IRQSTATUS, irqclr);

        /* check pending interrupts */
        tscadc_writel(ts_dev, TSCADC_REG_IRQEOI, 0x0);
        tscadc_writel(ts_dev, TSCADC_REG_SE, ADC_STPENB_STEPENB);
        return  ret_val;
}



////////////////////////////
static void tsc_idle_config(struct tscadc *ts_config)
{
    /* Idle mode touch screen config */
    unsigned int	 idleconfig;

    idleconfig = TSCADC_STEPCONFIG_YNN  ;
    // if (ts_config->analog_input == 0)
        idleconfig |= TSCADC_STEPCONFIG_XNN;
    // else
        // idleconfig |= TSCADC_STEPCONFIG_YPN;

    tscadc_writel(ts_config,TSCADC_REG_IDLECONFIG, idleconfig);
}


/************************************************************************
函数功能：GPIO初始化
输入参数：无
输出参数：
返回值  ：
说明    ：
************************************************************************/
static int ad_setup_reg(void)
{
    int			clk_value;
    struct clk	*clk;
    int		    clock_rate,  ctrl;
	printk("ad_setup_register.........1\n");
    /* Allocate memory for device */
    ts_dev = kzalloc(sizeof(struct tscadc), GFP_KERNEL);
    
    if (!ts_dev) {
        printk("failed to allocate memory.\n");
        return -ENOMEM;
    }

    ts_dev->tsc_base = ioremap(AM33XX_TSC_BASE, 10*1024*1024); //0x3200
    if (!ts_dev->tsc_base) {
        printk("failed to map registers.\n");
        return -1;
    }
#if  1
    clk = clk_get(pst_ad_drv->dev,"adc_tsc_fck");
   
    if (IS_ERR(clk)) {
        printk("failed to get TSC fck\n");
    }

    clock_rate = clk_get_rate(clk);
    clk_value = clock_rate / ADC_CLK;
    if (clk_value < 7) {
       printk( "clock input less than min clock requirement\n");
    }
	
    clk_value = clk_value - 1;
    tscadc_writel(ts_dev, TSCADC_REG_CLKDIV, clk_value);
#endif      
    ctrl = TSCADC_CNTRLREG_STEPCONFIGWRT |
            TSCADC_CNTRLREG_STEPID|TSCADC_CNTRLREG_TSCSSENB;
    tscadc_writel(ts_dev, TSCADC_REG_CTRL, ctrl);

    tsc_idle_config(ts_dev);
    //ADC相关寄存器配置
    // tsc_step_config(ts_dev);


    return 0 ;
}

/************************************************************************
函数功能：	对输入的数值进行滤波（16位）
输入参数：	u16 *data		  -- 数据首地址
            u16 uc_Num		-- 数据个数
            u16 filt_mod	-- 滤波模式（0：均值滤波；1：中值滤波）
返回值：	数据经滤波后的值
说明：
************************************************************************/
u16 ad_filter(u16 *data, u16 uc_Num, u16 filt_mod)
{
    u16 temp;
    u16 ui_temp;
    u16 i,j;

    for (j = 1; j <= uc_Num; j++) 	// 排序
    {
        for (i = 0; i < uc_Num-j; i++)
        {
            if ((*(data+i)) > (*(data +i+1)))
            {
                 temp			= *(data+i);
                *(data+i)		= *(data+i+1);
                *(data+i+1)		= temp;
            }
        }
    }

    ui_temp = 0;
    if (filt_mod == 0)
    {
        for (i = ABANDON_NUM; i < uc_Num-ABANDON_NUM; i++)	// 均值
        {
            ui_temp += *(data+i);
        }
        ui_temp = ui_temp / (uc_Num-(ABANDON_NUM*2));
    }
    else
    {
        ui_temp = *(data+(uc_Num >> 1));	// 取中值
    }
    return ui_temp;
}


/************************************************************************
函数功能：	补偿值处理，获得最终结果
输入参数：
说明：
************************************************************************/

static char calc_result(u16 *chVal,int ch)
{
	u16 ad_val[AD_CH];
	u16 temp_val=*chVal;
	int i ;

	if(ch<=0 ||ch > AD_CH)
	{
		return 1;
	}

	*chVal = (temp_val-BAK_ADC_1);
	return 0 ;
	
}


static ssize_t ad_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos)
{
	int ch1,ch2,ch3;

    if (size != AD_CH*sizeof(u16))
    {
    return -EINVAL;
    }
	// printk("read ad..\n");
	
    pr_debug("=========== Now ADC channel:%d working===========\n", channel);

    down_read(&pst_ad_drv->rw_sem);
    // if (pst_ad_drv->is_allow_read == 0)
    // {// 数据没有准备好
    //     up_read(&pst_ad_drv->rw_sem);
    //     return -EBUSY;
    // }
	
 //   if(!ev_adc)
    {
        if(filp->f_flags & O_NONBLOCK)
        {
            return -EAGAIN;
        }
        else
        {
          
            ch1 = read_ch_val(1);
            ch2 = read_ch_val(2);
            ch3 = read_ch_val(3);
            calc_result(&ch1,1);
            calc_result(&ch2,2);
 			calc_result(&ch3,3);

            pr_debug("ch1=%d,ch2=%d,ch3=%d\n",ch1,ch2,ch3);
            pst_ad_drv->ai_data[0]=ch1;
            pst_ad_drv->ai_data[1]=ch2;
            pst_ad_drv->ai_data[2]=ch3;
          //  wait_event_interruptible(adc_waitq, ev_adc);
        }
    }

     if (copy_to_user(buf, pst_ad_drv->ai_data, AD_CH*sizeof(u16)))
     {

     	up_read(&pst_ad_drv->rw_sem);
		return -EBUSY;
     }
	else
	{
		// pst_ad_drv->is_allow_read = 0;
		up_read(&pst_ad_drv->rw_sem);
		return AD_CH*sizeof(u16);
	}

     return AD_CH*sizeof(u16);
#if 0 
    
//    printk("ad0 =%d,ad1=%d\n",pst_ad_drv->ai_data[0],pst_ad_drv->ai_data[1]);
	if( pst_ad_drv->is_allow_read ==1)
	{	
		calc_result(pst_ad_drv->ai_data,AD_CH);
		if (copy_to_user(buf, pst_ad_drv->ai_data, AD_CH*sizeof(u16)))
		{
		
			up_read(&pst_ad_drv->rw_sem);
			return -EBUSY;
		}
	}
    else
    {
        // pst_ad_drv->is_allow_read = 0;
        up_read(&pst_ad_drv->rw_sem);
        return AD_CH*sizeof(u16);
    }
#endif 
	return 0 ;
}

static int ad_open(struct inode *inode, struct file *filp)
{


    return 0;
}

static int ad_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static const struct file_operations ad_fops =
{
    .owner 		= THIS_MODULE,
    .open 		= ad_open,
    .read 		= ad_read,
    .release 	= ad_release,
};

static int __init ad_init(void)
{
    int err;

    dev_t devno = MKDEV(ad_major, 0);
    pr_debug("***************************Data:%s,time:%s**************************\n",__DATE__,__TIME__);

    /***********************申请字符设备************************/
    if (ad_major)
    {// 手动申请
        err = register_chrdev_region(devno, ad_nr_devs, ad_dev_name);
    }
    else
    {// 内核自动申请
        err = alloc_chrdev_region(&devno, ad_minor, ad_nr_devs, ad_dev_name);
        ad_major = MAJOR(devno);
    }

    if (err < 0)
    {
        printk("am335x_ad can't get the major %d\n", ad_major);
        return err;
    }
    else
    {
        printk(KERN_DEBUG "am335x_ad success get the major is %d\n", ad_major);
    }
   	printk("ad_major = %d\n",ad_major);

    /**************注册字符设备的具体方法**********************/
    pst_ad_drv = kmalloc(sizeof(ST_AD_DRV), GFP_KERNEL);
    if (pst_ad_drv == NULL)
    {
        err = -ENOMEM;
        goto fail_malloc;
    }

    memset(pst_ad_drv, 0, sizeof(ST_AD_DRV));
    cdev_init(&pst_ad_drv->cdev, &ad_fops);	// 与文件操作函数关联起来
    pst_ad_drv->cdev.owner	= THIS_MODULE;
    
    //ad_setup_reg();
#define lin_test 1 
#if lin_test 
    ad_setup_reg();

    init_rwsem(&pst_ad_drv->rw_sem);
#if 0 
    INIT_WORK(&pst_ad_drv->adc_calc_work, adc_calc_handle);

    init_timer(&pst_ad_drv->timer);
#endif 
    pst_ad_drv->ch_index = 0;
    pst_ad_drv->ch_temp_pt = 0;
    pst_ad_drv->is_allow_read = 0;
#endif

    //add 后所有方法生效 在此之前进一步设置硬件

    err = cdev_add(&pst_ad_drv->cdev, devno, 1);
    if (err < 0)
    {
        printk("cdev_add fail\n");
        goto fail_cdev_add;
    }

    // 为proc文件系统 创建一个设备类
    pst_ad_drv->cls = class_create(THIS_MODULE, ad_dev_name);

    // 为该类 创建一个设备
    pst_ad_drv->dev = device_create(pst_ad_drv->cls, NULL, MKDEV(ad_major, 0), NULL, "%s", "ai_dev");
	
    //ad_setup_reg();
    //tscadc_readl(ts_dev, TSCADC_REG_CTRL);
    #if 0 
    //申请中断
    err = request_irq(ts_dev->irq, adc_interrupt, IRQF_DISABLED,
                ad_dev_name, ts_dev);
    if (err) {
        pr_debug( "failed to allocate irq.\n");
        goto fail_cdev_add;
    }
    #endif 
    adc_step_config(ts_dev,1);
   
#if 0// lin_test
    pst_ad_drv->timer.function	= timer_handler;
    pst_ad_drv->timer.data	= (unsigned long)pst_ad_drv;
    pst_ad_drv->timer.expires	= jiffies + (500 * HZ / 1000);	// 8ms为单位	add_timer(&pst_di_drv->timer);
    add_timer(&pst_ad_drv->timer);
#endif
    
     return 0 ;

fail_cdev_add:
    kfree(pst_ad_drv);
fail_malloc:
    unregister_chrdev_region(MKDEV(ad_major, ad_minor), ad_nr_devs);

    return err;
}

static void __exit ad_exit(void)
{
	printk("EXIT ADC\n");
	//del_timer(&pst_ad_drv->timer);
   // free_irq(ts_dev->irq,ts_dev);

	cdev_del(&pst_ad_drv->cdev);
	unregister_chrdev_region(MKDEV(ad_major, ad_minor), ad_nr_devs);
	device_destroy(pst_ad_drv->cls, MKDEV(ad_major, 0));
	class_destroy(pst_ad_drv->cls);
	// iounmap(ts_dev->tsc_base);
	// kfree(ts_dev);
	kfree(pst_ad_drv);
	printk("Close ADC\n");

}

module_init(ad_init);
module_exit(ad_exit);
MODULE_LICENSE("GPL");
