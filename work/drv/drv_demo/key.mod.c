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
	{ 0x796d56bd, "__init_rwsem" },
	{ 0x78a4d9af, "cdev_add" },
	{ 0x65d6d0f0, "gpio_direction_input" },
	{ 0x47229b5c, "gpio_request" },
	{ 0xfe990052, "gpio_free" },
	{ 0x93d01c67, "cdev_init" },
	{ 0xfa2a45e, "__memzero" },
	{ 0x8836fca7, "kmem_cache_alloc" },
	{ 0xc5ae0182, "malloc_sizes" },
	{ 0x29537c9e, "alloc_chrdev_region" },
	{ 0xd8e484f0, "register_chrdev_region" },
	{ 0x37a0cba, "kfree" },
	{ 0xc7ef0ce2, "class_destroy" },
	{ 0xcfad398d, "device_destroy" },
	{ 0x7485e15e, "unregister_chrdev_region" },
	{ 0x43aa22c0, "cdev_del" },
	{ 0x27e1a049, "printk" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "F1D647F260D5123F5234607");