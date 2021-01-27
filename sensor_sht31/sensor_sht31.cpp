#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "kidbright32.h"
#include "sensor_sht31.h"

SENSOR_SHT31::SENSOR_SHT31()
{
	channel = 0;
	address = SENSOR_SHT31_I2C_ADDRESS;
	polling_ms = SENSOR_SHT31_POLLING_MS;
}

void SENSOR_SHT31::init(void)
{
	temperature = 0;
	humidity = 0;
	state = s_detect;

	i2c_set_timeout(I2C_NUM_1, 40000);
}

int SENSOR_SHT31::prop_count(void)
{
	return 2;
}

bool SENSOR_SHT31::prop_name(int index, char *name)
{
	if (index == 0)
	{
		snprintf(name, DEVICE_PROP_NAME_LEN_MAX, "%s", "Temperature value");
		return true;
	}
	else if (index == 1)
	{
		snprintf(name, DEVICE_PROP_NAME_LEN_MAX, "%s", "Relative Humidity value");
		return true;
	}

	// not supported
	return false;
}

bool SENSOR_SHT31::prop_unit(int index, char *unit)
{
	if (index == 0)
	{
		snprintf(unit, DEVICE_PROP_UNIT_LEN_MAX, "%s", "Celsius");
		return true;
	}
	else if (index == 1)
	{
		snprintf(name, DEVICE_PROP_UNIT_LEN_MAX, "%s", "Percent");
		return true;
	}

	// not supported
	return false;
}

bool SENSOR_SHT31::prop_attr(int index, char *attr)
{
	if ((index == 0) || (index == 1))
	{
		get_attr_str(attr, PROP_ATTR_READ_FLAG | PROP_ATTR_TYPE_NUMBER_FLAG); // read only, number
		return true;
	}

	// not supported
	return false;
}

bool SENSOR_SHT31::prop_read(int index, char *value)
{
	if (index == 0)
	{
		snprintf(value, DEVICE_PROP_VALUE_LEN_MAX, "%f", temperature);
		return true;
	}
	else if (index == 1)
	{
		snprintf(value, DEVICE_PROP_VALUE_LEN_MAX, "%f", humidity);
		return true;
	}

	// not supported
	return false;
}

bool SENSOR_SHT31::prop_write(int index, char *value)
{
	// not supported
	return false;
}

uint8_t SENSOR_SHT31::crc8(const uint8_t *data, int size)
{
	// CRC-8 formula from page 14 of SHT spec pdf
	// test data 0xbe, 0xef => 0x92
	// initialization data 0xff
	// polynomial = 0x31 (x8 + x5 +x4 +1)
	// final xor = 0x00

	uint8_t crc = 0xff;

	for (int j = size; j; --j)
	{
		crc ^= *data++;

		for (int i = 8; i; --i)
		{
			crc = (crc & 0x80) ? (crc << 1) ^ 0x31 : (crc << 1);
		}
	}

	return crc;
}

void SENSOR_SHT31::process(Driver *drv)
{
	// I2CDev *i2c = (I2CDev *)drv;
	i2c = (I2CDev *)drv;
	uint8_t data[6];
	uint16_t temp, humid;
	double dtemp, dhumid;

	switch (state)
	{
	case s_detect:
		// stamp polling tickcnt
		polling_tickcnt = get_tickcnt();

		// detect i2c device
		if (i2c->detect(channel, address) == ESP_OK)
		{
			// periodic high repetitive measurement
			data[0] = SENSOR_SHT31_HIGHREP >> 8;
			data[1] = SENSOR_SHT31_HIGHREP & 0xff;
			if (i2c->write(channel, address, data, 2) == ESP_OK)
			{
				// clear error flag
				error = false;
				// get current tickcnt
				tickcnt = get_tickcnt();
				state = s_get_sensor;
			}
			else
			{
				state = s_error;
			}
		}
		else
		{
			state = s_error;
		}
		break;

	case s_get_sensor:
		// read delay for 40ms
		if (is_tickcnt_elapsed(tickcnt, 40))
		{
			if (i2c->read(channel, address, NULL, 0, data, 6) == ESP_OK)
			{
				temp = (data[0] << 8) | data[1];
				if (data[2] == crc8(data, 2))
				{
					dtemp = temp;
					dtemp *= 175;
					dtemp /= 0xffff;
					dtemp = -45 + dtemp;
					temperature = dtemp;
				}

				humid = (data[3] << 8) | data[4];
				if (data[5] == crc8(data + 3, 2))
				{
					dhumid = humid;
					dhumid *= 100;
					dhumid /= 0xFFFF;
					humidity = dhumid;
				}

				// set initialized flag
				initialized = true;
				// load polling tickcnt
				tickcnt = polling_tickcnt;
				// goto wait and retry with detect state
				state = s_wait;
			}
			else
			{
				state = s_error;
			}
		}
		break;

	case s_error:
		// set error flag
		error = true;
		// clear initialized flag
		initialized = false;
		// get current tickcnt
		tickcnt = get_tickcnt();
		// goto wait and retry with detect state
		state = s_wait;
		// set flag to switch to comm on other i2c bus
		i2c_bus_flag = !i2c_bus_flag;
		break;

	case s_wait:
		// wait polling_ms timeout
		if (is_tickcnt_elapsed(tickcnt, polling_ms))
		{
			state = s_detect;
		}
		break;
	}
}

//* ************* Input port functions *************

double SENSOR_SHT31::readTemp(void)
{
	return temperature;
}

double SENSOR_SHT31::readHumi(void)
{
	return humidity;
}
