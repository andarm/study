#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x372d36bb, "module_layout" },
	{ 0x3ec8886f, "param_ops_int" },
	{ 0x68403ddd, "platform_driver_register" },
	{ 0xb9e52429, "__wake_up" },
	{ 0x307ac37b, "clk_get_rate" },
	{ 0x21e7c5cc, "clk_get" },
	{ 0x8cb1c8da, "__pm_runtime_resume" },
	{ 0xb87e8f83, "pm_runtime_enable" },
	{ 0xd6b8e852, "request_threaded_irq" },
	{ 0x40a6f522, "__arm_ioremap" },
	{ 0xadf42bd5, "__request_region" },
	{ 0x461b2c07, "platform_get_irq" },
	{ 0x78a4d9af, "cdev_add" },
	{ 0x93d01c67, "cdev_init" },
	{ 0x29537c9e, "alloc_chrdev_region" },
	{ 0x8836fca7, "kmem_cache_alloc" },
	{ 0xc5ae0182, "malloc_sizes" },
	{ 0x3c6ddbbb, "dev_err" },
	{ 0x2196324, "__aeabi_idiv" },
	{ 0x8893fa5d, "finish_wait" },
	{ 0x75a17bed, "prepare_to_wait" },
	{ 0x1000e51, "schedule" },
	{ 0xc8b57c27, "autoremove_wake_function" },
	{ 0x5f754e5a, "memset" },
	{ 0x67c2fa54, "__copy_to_user" },
	{ 0x6369f12b, "dev_set_drvdata" },
	{ 0xbad037c0, "device_init_wakeup" },
	{ 0x37a0cba, "kfree" },
	{ 0x27e53c50, "__pm_runtime_disable" },
	{ 0x9bce482f, "__release_region" },
	{ 0x788fe103, "iomem_resource" },
	{ 0x45a55ec8, "__iounmap" },
	{ 0x22184d7f, "platform_get_resource" },
	{ 0xf20dabd8, "free_irq" },
	{ 0x7485e15e, "unregister_chrdev_region" },
	{ 0x43aa22c0, "cdev_del" },
	{ 0x215e56dd, "platform_driver_unregister" },
	{ 0x27e1a049, "printk" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "63CBADC27EB8483292DE06A");
