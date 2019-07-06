#ifndef __WDG_H
#define __WDG_H
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

#define GPIO_TO_PIN(bank, gpio) (32 * (bank) + (gpio))




/* am335x的GPIO接口 */
extern int  gpio_request(unsigned gpio,const char *tag);
extern int  gpio_direction_input(unsigned gpio);
extern int  gpio_direction_output(unsigned gpio, int value);
extern int  gpio_get_value(unsigned gpio);
extern void gpio_set_value(unsigned gpio, int value);
extern int  gpio_cansleep(unsigned gpio);

static int dev_open(struct inode *inode, struct file *filp);
static int dev_release(struct inode *inode, struct file *filp);
static ssize_t dev_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos);
static ssize_t dev_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos);

static inline void do_setup_pin(void);
static void start_watchdog(void);
static void stop_watchdog(void);
static void stop_feed_watchdog(void);
static void timer_handler(unsigned long data);

#if 1
	#define pr_debug printk
#else
#define pr_debug(fmt, ...) \
      printk(KERN_DEBUG fmt, ##__VA_ARGS__)
#endif 

#endif 
