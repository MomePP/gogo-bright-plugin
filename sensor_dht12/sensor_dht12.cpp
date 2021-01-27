#include <stdio.h>
#include <math.h>
#include <locale>
#include "esp_system.h"
#include "kidbright32.h"
#include "sensor_dht12.h"

SENSOR_DHT12::SENSOR_DHT12()
{
	channel = 0; //bus_ch;
	address = SENSOR_DHT12_I2C_ADDRESS;
	polling_ms = SENSOR_DHT12_POLLING_MS;
}

void SENSOR_DHT12::init(void)
{
	first_read = true;
	temperature = 0;
	humidity = 0;
	state = s_detect;

    i2c_set_timeout(I2C_NUM_1, 40000);
}

int SENSOR_DHT12::prop_count(void)
{
	return 2;
}

bool SENSOR_DHT12::prop_name(int index, char *name)
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

bool SENSOR_DHT12::prop_unit(int index, char *unit)
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

bool SENSOR_DHT12::prop_attr(int index, char *attr)
{
	if (index == 0)
	{
		get_attr_str(attr, PROP_ATTR_READ_FLAG | PROP_ATTR_TYPE_NUMBER_FLAG); // read only, number
		return true;
	}

	// not supported
	return false;
}

bool SENSOR_DHT12::prop_read(int index, char *value)
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

	return false;
}

bool SENSOR_DHT12::prop_write(int index, char *value)
{
	// not supported
	return false;
}

void SENSOR_DHT12::process(Driver *drv)
{
	// I2CDev *i2c = (I2CDev *)drv;
	i2c = (I2CDev *)drv;
	uint8_t byte, data[2];

	switch (state)
	{
	case s_detect:
		// stamp polling tickcnt
		polling_tickcnt = get_tickcnt();

		// detect i2c device
		if (i2c->detect(channel, address) == ESP_OK)
		{
			initialized = true;
			// clear error flag
			error = false;
			// get current tickcnt
			tickcnt = get_tickcnt();
			state = s_get_sensor;
		}
		else //* try on other i2c bus
		{
			state = s_error;
		}
		break;
	case s_get_sensor:
		// read delay for 40ms
		if (is_tickcnt_elapsed(tickcnt, 40))
		{
            i2c->write(channel, address, &initCmd, 1);
            i2c->read(channel, address, NULL, 0, raw_data, 5);

			// ****** Checksum ******
			if (raw_data[0] + raw_data[1] + raw_data[2] + raw_data[3] != raw_data[4])
			{
				state = s_error;
				break;
			}

			// ****** Calc Temp ******
			temperature = (float)raw_data[2] * 10 + raw_data[3];

			// ****** Calc Humi ******
			humidity = (float)raw_data[0] * 10 + raw_data[1];

			initialized = true;
			// load polling tickcnt
			tickcnt = polling_tickcnt;
			// goto wait and retry with detect state
			state = s_wait;
		}
		break;
	case s_error:
		// set error flag
		error = true;
		// clear initialized flag
		initialized = false;
		// set first read flag
		first_read = true;
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

float SENSOR_DHT12::readTemp(void)
{
	return temperature;
}

float SENSOR_DHT12::readHumi(void)
{
	return humidity;
}
