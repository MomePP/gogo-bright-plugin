#ifndef __SENSOR_SHT31_H__
#define __SENSOR_SHT31_H__

#include "driver.h"
#include "device.h"
#include "i2c-dev.h"

#define SENSOR_SHT31_POLLING_MS 1000
#define SENSOR_SHT31_I2C_ADDRESS 0x44
#define SENSOR_SHT31_HIGHREP 0x2400

class SENSOR_SHT31 : public Device
{
private:
	enum
	{
		s_detect,
		s_get_sensor,
		s_error,
		s_wait
	} state;

	TickType_t tickcnt, polling_tickcnt;
	bool i2c_bus_flag = 0;

	I2CDev *i2c;

	double temperature;
	double humidity;

	uint8_t crc8(const uint8_t *data, int size);

public:
	// constructor
	SENSOR_SHT31(void);
	// override
	void init(void);
	void process(Driver *drv);
	int prop_count(void);
	bool prop_name(int index, char *name);
	bool prop_unit(int index, char *unit);
	bool prop_attr(int index, char *attr);
	bool prop_read(int index, char *value);
	bool prop_write(int index, char *value);
	// method
	double readTemp(void);
	double readHumi(void);
};

#endif
