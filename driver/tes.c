
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/wait.h>

#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/kthread.h>
#include <linux/async.h>
#include <linux/pm_runtime.h>
#include <linux/pinctrl/devinfo.h>

#include "tes.h"

extern pid_t tes_ktherad_create(void);
extern void tes_kthread_destroy(void);
extern int tes_bus_init(void);

struct tes_device *tesdev;
struct tes_driver *tesdrv;


int tes_device_register(struct tes_device *tesdev)
{
	tesdev->device.bus = &tes_bus_type;
	return device_register(&tesdev->device);
}

void tes_device_unregister(struct tes_device *tesdev)
{
	device_unregister(&tesdev->device);
}

//--------------------------------------------------

static ssize_t value_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct tes_device *tes_dev = to_tes_device(dev);
	return sprintf(buf, "%ld\n\r", tes_dev->value);
}
static ssize_t value_store(struct device *dev, struct device_attribute *attr,
		 const char *buf, size_t count)
{
	struct tes_device *tes_dev = to_tes_device(dev);
	tes_dev->value = simple_strtoul(buf, NULL, 0);

	/** poll或者epoll可用
	 *  poll使用 POLLPRI 事件; epoll使用 EPOLLWAKEUP 事件
	 *  调用poll(); epoll_wait() 之前要调用read()
	 */
	sysfs_notify(&dev->kobj, NULL, "sleep");	
	return count;
}
static DEVICE_ATTR_RW(value);


static ssize_t sleep_show(struct device *dev,
				       struct device_attribute *attr, char *buf)
{
	struct tes_device *tes_dev = to_tes_device(dev);
	return sprintf(buf, "%ld\n\r", tes_dev->value);
}
static DEVICE_ATTR_RO(sleep);


static int tes_driver_probe(struct device *dev)
{
	printk("[ tes_driver_probe: ] drivers probe dev:%s\n", dev_name(dev));
	device_create_file(dev, &dev_attr_value);		//创建设备的属性文件
	device_create_file(dev, &dev_attr_sleep);


	return 0;
}

static int tes_driver_remove(struct device *dev)
{
	/** echo "device" > unbind 时会执行此函数 */
	printk("[ tes_driver_remove: ] driver remove dev:%s\n", dev_name(dev));

	return 0;
}

static void tes_driver_shutdown(struct device *dev)
{
	printk("[ tes_driver_shutdown: ] driver shutdown dev:%s\n", dev_name(dev));
}

int tes_driver_register(struct tes_driver *tesdrv)
{
	tesdrv->driver.owner = THIS_MODULE;
	tesdrv->driver.bus = &tes_bus_type;
	tesdrv->driver.probe = tes_driver_probe;
	tesdrv->driver.remove = tes_driver_remove;
	tesdrv->driver.shutdown = tes_driver_shutdown; //暂时不清楚何时调用这玩意儿

	return driver_register(&tesdrv->driver);
}

void tes_driver_unregister(struct tes_driver *tesdrv)
{
	driver_unregister(&tesdrv->driver);
}


static void tesdev_device_release(struct device *dev)
{
}

static int __init tes_init(void)
{
	int ret;

	printk("[ ==> start of all: ] register tes_bus_type.. \n");

	tes_ktherad_create();

	tes_bus_init();


	tesdev = kzalloc(sizeof(struct tes_device), GFP_KERNEL);
	tesdev->device.init_name = "TesDev@000";
	tesdev->device.release = tesdev_device_release;	//release函数必须要有，否则退出时报错
	ret = tes_device_register(tesdev);
	if (ret < 0) {
		printk("tes_device_register failed!\n");
	}
	printk("device id is %d\n", tesdev->device.id);	//id从0开始


	tesdrv = kzalloc(sizeof(struct tes_driver), GFP_KERNEL);
	tesdrv->driver.name = "TesDrv@000";
	ret = tes_driver_register(tesdrv);
	if (ret < 0) {
		printk("tes_driver_register failed!\n");
	}

	return 0;
}

static void __exit tes_exit(void)
{
	tes_driver_unregister(tesdrv);
	tes_device_unregister(tesdev);
	bus_unregister(&tes_bus_type);
//	kfree(tesdrv);						//是否需要显式释放内存？
//	kfree(tesdrv);
	tes_kthread_destroy();
	printk("goodbye tes_exit!\n");
}

module_init(tes_init);
module_exit(tes_exit);
MODULE_LICENSE("GPL");


