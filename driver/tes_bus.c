
/** 注册bus type相关 */

#include <linux/module.h>
#include <linux/device.h>
#include "tes.h"

static int tes_device_match(struct device *dev, struct device_driver *drv)
{
	return 1;
}

static int tes_uevent(struct device *dev, struct kobj_uevent_env *env)
{
	//在为用户空间产生热插拔事件之前，这个方法允许总线添加环境变量。
	/** add_uevent_var(env, "PRODUCT=%x/%x/%x",
			   le16_to_cpu(usb_dev->descriptor.idVendor),
			   le16_to_cpu(usb_dev->descriptor.idProduct),
			   le16_to_cpu(usb_dev->descriptor.bcdDevice))
	*/
	return 0;
}

static int tes_bus_probe(struct device *dev)
{
	/** 当match()函数配对成功后，会执行。*/
	printk("[ tes_bus_probe: ] device name: %s: driver name: %s \n", dev_name(dev), dev->driver->name);

	dev->driver->probe(dev);

	return 0;
}

static ssize_t version_show(struct bus_type *bus, char *buf)
{
	return snprintf(buf, 9, "ver0.01\n\r");	//  \n\r用于回车换行
}
static BUS_ATTR_RO(version);	//会创建一个 bus_attribute 结构体，名字叫："bus_attr_version";表征一个属性文件，要实现这个结构体的读写函数
//struct bus_attribute bus_arrt_version;


int tes_bus_init(void)
{
	/** --- /sys/bus/目录下增加tes子目录
 	 * tes目录下有device和driver子目录
 	 * tes目录下有默认的drivers_autoprobe[rw]，drivers_probe[w]，uevent[w]三个文件
 	 * drivers_autoprobe读出来是1
	 */
	bus_register(&tes_bus_type);
	bus_create_file(&tes_bus_type, &bus_attr_version);
	
	return 0;
}

EXPORT_SYMBOL_GPL(tes_bus_init);


static ssize_t name_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%s\n\r", dev_name(dev));
}
static DEVICE_ATTR_RO(name);	//会创建一个 device_attribute 结构体，名字叫："dev_attr_name";

static ssize_t id_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n\r", dev->id);
}

static DEVICE_ATTR_RO(id);

struct attribute *tes_dev_attrs[] = {
	&dev_attr_name.attr,
	&dev_attr_id.attr,
	NULL,
};

ATTRIBUTE_GROUPS(tes_dev);

struct bus_type tes_bus_type = {
	.name 	= "tes",
	.match 	= tes_device_match,
	.uevent = tes_uevent,
	.probe 	= tes_bus_probe,
	.dev_groups = tes_dev_groups,
};

