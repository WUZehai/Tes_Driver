#ifndef _TES_H
#define _TES_H

#include <linux/device.h>

extern struct bus_type tes_bus_type;

struct tes_device {
	struct device device;
	unsigned long value;
};

struct tes_driver{
	struct device_driver driver;
};

#define to_tes_device(d) container_of(d, struct tes_device, device);
#define to_tes_driver(d) container_of(d, struct tes_driver, driver);


#endif
