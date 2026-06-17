#include <linux/spinlock.h>
#include <linux/module.h>
#include <linux/idr.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/parser.h>
#include <linux/vmalloc.h>
#include <linux/uio_driver.h>
#include <linux/stringify.h>
#include <linux/bitops.h>
#include <net/genetlink.h>
#include <scsi/scsi_common.h>
#include <scsi/scsi_proto.h>

static struct device *parent_device;
struct uio_info *g_info;

static int __init uio_led_init(void)
{
	int ret;
	struct uio_info *info;
	
	/* 1. 分配/设置/注册uio_info */
	parent_device = root_device_register("uio_led_parent");
	if (IS_ERR(parent_device)) {
		ret = PTR_ERR(parent_device);
		return -ENOMEM;
	}

	g_info = info = kmalloc(sizeof(*info), GFP_KERNEL);
	if (!info) {
		root_device_unregister(parent_device);
		return -ENOMEM;
	}

	memset(info, 0, sizeof(*info));

	info->name = "uio_led_info";
	info->version = "v1.0";

	info->mem[0].name = "first_mem";
	info->mem[0].addr = (phys_addr_t)0x02290000;
	info->mem[0].size = 4096;
	info->mem[0].memtype = UIO_MEM_PHYS;

	info->mem[1].name = "second_mem";
	info->mem[1].addr = (phys_addr_t)0x020AC000;
	info->mem[1].size = 4096;
	info->mem[1].memtype = UIO_MEM_PHYS;

	ret = uio_register_device(parent_device, info);
	if (ret) {
		kfree(info);
		root_device_unregister(parent_device);
	}
	return ret;
}

static void __exit uio_led_exit(void)
{
	uio_unregister_device(g_info);
	root_device_unregister(parent_device);
	kfree(g_info);
}


module_init(uio_led_init);
module_exit(uio_led_exit);

MODULE_LICENSE("GPL");



