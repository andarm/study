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
	{ 0xdc1e1b7d, "device_create" },
	{ 0x4c1d0cd4, "__class_create" },
	{ 0x78a4d9af, "cdev_add" },
	{ 0x796d56bd, "__init_rwsem" },
	{ 0x307ac37b, "clk_get_rate" },
	{ 0x21e7c5cc, "clk_get" },
	{ 0x40a6f522, "__arm_ioremap" },
	{ 0x93d01c67, "cdev_init" },
	{ 0xfa2a45e, "__memzero" },
	{ 0x8836fca7, "kmem_cache_alloc" },
	{ 0xc5ae0182, "malloc_sizes" },
	{ 0x29537c9e, "alloc_chrdev_region" },
	{ 0xd8e484f0, "register_chrdev_region" },
	{ 0x67c2fa54, "__copy_to_user" },
	{ 0x58db3f5, "up_read" },
	{ 0x4e748dcd, "down_read" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0x37a0cba, "kfree" },
	{ 0xc7ef0ce2, "class_destroy" },
	{ 0xcfad398d, "device_destroy" },
	{ 0x7485e15e, "unregister_chrdev_region" },
	{ 0x43aa22c0, "cdev_del" },
	{ 0x27e1a049, "printk" },
	{ 0x2196324, "__aeabi_idiv" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "7805EADD008E4816EE0F50C");
