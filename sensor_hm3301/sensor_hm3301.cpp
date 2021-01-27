#include <stdio.h>
#include <math.h>
#include <locale>
#include <string.h>
#include "esp_system.h"
#include "kidbright32.h"
#include "sensor_hm3301.h"

SENSOR_HM3301::SENSOR_HM3301()
{
    channel = 0; //bus_ch;
    address = SENSOR_HM3301_I2C_ADDRESS;
    polling_ms = SENSOR_HM3301_POLLING_MS;
}

void SENSOR_HM3301::init(void)
{
    first_init = true;
    error = false;
    initialized = true;
    state = s_detect;
    
    i2c_set_timeout(I2C_NUM_1, 40000);
}

int SENSOR_HM3301::prop_count(void)
{
    return 0;
}

bool SENSOR_HM3301::prop_name(int index, char *name)
{
    // not supported
    return false;
}

bool SENSOR_HM3301::prop_unit(int index, char *unit)
{
    // not supported
    return false;
}

bool SENSOR_HM3301::prop_attr(int index, char *attr)
{
    // not supported
    return false;
}

bool SENSOR_HM3301::prop_read(int index, char *value)
{
    // not supported
    return false;
}

bool SENSOR_HM3301::prop_write(int index, char *value)
{
    // not supported
    return false;
}

bool SENSOR_HM3301::calChecksum(uint8_t *data)
{
    uint8_t sum = 0;
    for (int i = 0; i < SENSOR_HM3301_DATA_SIZE - 1; i++) //? fixed size data checksum 28 bytes
    {
        sum += data[i];
    }

    if (sum == data[28])
        return true;
    return false;
}

void SENSOR_HM3301::process(Driver *drv)
{
    i2c = (I2CDev *)drv;
    switch (state)
    {
    case s_detect:
        // stamp polling tickcnt
        polling_tickcnt = get_tickcnt();

        // detect i2c device
        if (i2c->detect(channel, SENSOR_HM3301_I2C_ADDRESS) == ESP_OK)
        {
            if (first_init)
            {
                if(i2c->write(channel, SENSOR_HM3301_I2C_ADDRESS, &initCmd, 1) == ESP_OK)
                {
                    first_init = false;
                }
                else
                {
                    state = s_error;
                }
            }
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
            if (i2c->read(channel, SENSOR_HM3301_I2C_ADDRESS, NULL, 0, data_hm3301, SENSOR_HM3301_DATA_SIZE) == ESP_OK)
            {
                //? calculate checksum
                if (calChecksum(data_hm3301))
                {
                    for (int i = 2; i < 5; i++)
                    {
                        dustValue[i - 2] = (uint16_t)data_hm3301[i * 2] << 8 | data_hm3301[i * 2 + 1];
                    }
                }
                else
                {
                    // reset the old value
                    memset(dustValue, 0, sizeof(dustValue));
                }

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
        // clear first time init flag
        first_init = false;
        // get current tickcnt
        tickcnt = get_tickcnt();
        // goto wait and retry with detect state
        state = s_wait;
        break;

    case s_wait:
        // wait polling_ms timeout
        if (is_tickcnt_elapsed(tickcnt, polling_ms)) // polling_ms
        {
            state = s_detect;
        }
        break;
    }
}

//* ************* Input port functions *************

int SENSOR_HM3301::readDust(uint8_t pm_size)
{
    return dustValue[pm_size];
}
