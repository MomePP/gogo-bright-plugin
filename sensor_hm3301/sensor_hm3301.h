#ifndef __SENSOR_HM3301_H__
#define __SENSOR_HM3301_H__

#include <string>
using std::string;

#include "driver.h"
#include "device.h"
#include "i2c-dev.h"

#define SENSOR_HM3301_POLLING_MS 1000
#define SENSOR_HM3301_I2C_ADDRESS 0x40
#define SENSOR_HM3301_DATA_SIZE 29

class SENSOR_HM3301 : public Device
{
private:
    uint8_t _address;
    enum
    {
        s_detect,
        s_get_sensor,
        s_error,
        s_wait
    } state;

    TickType_t tickcnt, polling_tickcnt;
    bool first_init;
    bool i2c_bus_flag = 0;

    I2CDev *i2c;
    esp_err_t status_i2c;

    uint8_t data_hm3301[SENSOR_HM3301_DATA_SIZE];
    uint8_t initCmd = 0x88;
    uint16_t dustValue[3] = {0};
    
    bool calChecksum(uint8_t *data);

public:
    SENSOR_HM3301();
    // override
    void init(void);
    void process(Driver *drv);
    int prop_count(void);
    bool prop_name(int index, char *name);
    bool prop_unit(int index, char *unit);
    bool prop_attr(int index, char *attr);
    bool prop_read(int index, char *value);
    bool prop_write(int index, char *value);

    //* ********************* I2C port functions *********************
    int readDust(uint8_t pm_size);
};
#endif
