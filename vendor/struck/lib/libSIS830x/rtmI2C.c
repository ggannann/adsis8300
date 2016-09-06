#include <unistd.h>

#include "sis830x.h"
#include "internal.h"
#include "rtmI2C.h"

SIS830X_STATUS rtm_i2c_regRead(PSIS830X_DEVICE device, uint8_t i2c_device_addr, uint8_t reg_addr, uint8_t *data, uint8_t read_nof_byte)
{
    SIS830X_STATUS ret;

    //intI2cStop(device);

    int8_t bus_addr = i2c_device_addr << 1;
    uint8_t i;

    ret = rtmI2cStart(device);
    if(ret != Stat830xSuccess){
        return ret;
    }

    // address slave
    bool ack = false;
    ret = rtmI2cWrite(device, bus_addr, &ack);
    if(ret != Stat830xSuccess){
        return ret;
    }
    if(!ack){
        rtmI2cStop(device);
        return Stat830xI2cNack;
    }

    // write register address
    ret = rtmI2cWrite(device, reg_addr, &ack);
    if(ret != Stat830xSuccess){
        return ret;
    }
    if(!ack){
        rtmI2cStop(device);
        return Stat830xI2cNack;
    }

    // stop-start
    ret = rtmI2cStop(device);
    if(ret != Stat830xSuccess){
        return ret;
    }
    ret = rtmI2cStart(device);
    if(ret != Stat830xSuccess){
        return ret;
    }

    // readdress
    ret = rtmI2cWrite(device, bus_addr | 1, &ack);
    if(ret != Stat830xSuccess){
        return ret;
    }
    if(!ack){
        rtmI2cStop(device);
        return Stat830xI2cNack;
    }

    for(i=0; i < read_nof_byte; i++){
        // read data
        ret = rtmI2cRead(device, data+i, false);
        if(ret != Stat830xSuccess){
            return ret;
        }
    }

    // stop
    ret = rtmI2cStop(device);
    if(ret != Stat830xSuccess){
        return ret;
    }

    return Stat830xSuccess;
}

SIS830X_STATUS rtm_i2c_regWrite(PSIS830X_DEVICE device, uint8_t i2c_device_addr, uint8_t reg_addr, uint8_t *data, uint8_t write_nof_byte)
{
    SIS830X_STATUS ret;

    //intI2cStop(device);

    uint8_t bus_addr = i2c_device_addr << 1;
    uint8_t i;

    ret = rtmI2cStart(device);
    if(ret != Stat830xSuccess){
        return ret;
    }

    // address slave
    bool ack = false;
    ret = rtmI2cWrite(device, bus_addr, &ack);
    if(ret != Stat830xSuccess){
        return ret;
    }
    if(!ack){
        rtmI2cStop(device);
        return Stat830xI2cNack;
    }

    // write register address
    ret = rtmI2cWrite(device, reg_addr, &ack);
    if(ret != Stat830xSuccess){
        return ret;
    }
    if(!ack){
        rtmI2cStop(device);
        return Stat830xI2cNack;
    }

    for(i=0; i < write_nof_byte; i++){
        // write data
        ret = rtmI2cWrite(device, *(data+i), &ack);
        if(ret != Stat830xSuccess){
            return ret;
        }
        if(!ack){
            rtmI2cStop(device);
            return Stat830xI2cNack;
        }
    }
    // stop
    ret = rtmI2cStop(device);
    if(ret != Stat830xSuccess){
        return ret;
    }

    return Stat830xSuccess;
}

SIS830X_STATUS rtm_pca9535_Read(PSIS830X_DEVICE device, uint8_t i2c_device_addr, uint8_t reg_addr, uint8_t *data)
{
    SIS830X_STATUS ret;
    uint8_t read_nof_byte = 1;

    //intI2cStop(device);

    int8_t bus_addr = i2c_device_addr << 1;
    uint8_t i;

    ret = rtmI2cStart(device);
    if(ret != Stat830xSuccess){
        return ret;
    }

    // address slave
    bool ack = false;
    ret = rtmI2cWrite(device, bus_addr, &ack);
    if(ret != Stat830xSuccess){
        return ret;
    }
    if(!ack){
        rtmI2cStop(device);
        return Stat830xI2cNack;
    }

    // write register address
    ret = rtmI2cWrite(device, reg_addr, &ack);
    if(ret != Stat830xSuccess){
        return ret;
    }
    if(!ack){
        rtmI2cStop(device);
        return Stat830xI2cNack;
    }

    // stop-start
    ret = rtmI2cStop(device);
    if(ret != Stat830xSuccess){
        return ret;
    }
    ret = rtmI2cStart(device);
    if(ret != Stat830xSuccess){
        return ret;
    }

    // readdress
    ret = rtmI2cWrite(device, bus_addr | 1, &ack);
    if(ret != Stat830xSuccess){
        return ret;
    }
    if(!ack){
        rtmI2cStop(device);
        return Stat830xI2cNack;
    }

    for(i=0; i < read_nof_byte; i++){
        // read data
        ret = rtmI2cRead(device, data+i, false);
        if(ret != Stat830xSuccess){
            return ret;
        }
    }

    // stop
    ret = rtmI2cStop(device);
    if(ret != Stat830xSuccess){
        return ret;
    }

    return Stat830xSuccess;
}

SIS830X_STATUS rtm_pca9535_Write(PSIS830X_DEVICE device, uint8_t i2c_device_addr, uint8_t reg_addr, uint8_t *data)
{
    SIS830X_STATUS ret;
    uint8_t write_nof_byte = 1;

    //intI2cStop(device);

    uint8_t bus_addr = i2c_device_addr << 1;
    uint8_t i;

    ret = rtmI2cStart(device);
    if(ret != Stat830xSuccess){
        return ret;
    }

    // address slave
    bool ack = false;
    ret = rtmI2cWrite(device, bus_addr, &ack);
    if(ret != Stat830xSuccess){
        return ret;
    }
    if(!ack){
        rtmI2cStop(device);
        return Stat830xI2cNack;
    }

    // write register address
    ret = rtmI2cWrite(device, reg_addr, &ack);
    if(ret != Stat830xSuccess){
        return ret;
    }
    if(!ack){
        rtmI2cStop(device);
        return Stat830xI2cNack;
    }

    for(i=0; i < write_nof_byte; i++){
        // write data
        ret = rtmI2cWrite(device, *(data+i), &ack);
        if(ret != Stat830xSuccess){
            return ret;
        }
        if(!ack){
            rtmI2cStop(device);
            return Stat830xI2cNack;
        }
    }
    // stop
    ret = rtmI2cStop(device);
    if(ret != Stat830xSuccess){
        return ret;
    }

    return Stat830xSuccess;
}

SIS830X_STATUS rtm_ltc2493_Read(PSIS830X_DEVICE device, uint8_t i2c_device_addr, uint8_t *data)
{
    SIS830X_STATUS ret;

    //intI2cStop(device);

    int8_t bus_addr = i2c_device_addr << 1;
    uint8_t i;
    bool ack = false;

    ret = rtmI2cStart(device);
    if(ret != Stat830xSuccess){
        return ret;
    }

    // readdress
    ret = rtmI2cWrite(device, bus_addr | 1, &ack);
    if(ret != Stat830xSuccess){
        return ret;
    }
    if(!ack){
        rtmI2cStop(device);
        return Stat830xI2cNack;
    }

    for(i=0; i < 4; i++){
        // read data
        ret = rtmI2cRead(device, data+i, false);
        if(ret != Stat830xSuccess){
            return ret;
        }
    }

    // stop
    ret = rtmI2cStop(device);
    if(ret != Stat830xSuccess){
        return ret;
    }

    return Stat830xSuccess;
}

SIS830X_STATUS rtm_ltc2493_Write(PSIS830X_DEVICE device, uint8_t i2c_device_addr, uint8_t *data)
{
    SIS830X_STATUS ret;

    //intI2cStop(device);

    uint8_t bus_addr = i2c_device_addr << 1;
    uint8_t i;

    ret = rtmI2cStart(device);
    if(ret != Stat830xSuccess){
        return ret;
    }

    // address slave
    bool ack = false;
    ret = rtmI2cWrite(device, bus_addr, &ack);
    if(ret != Stat830xSuccess){
        return ret;
    }
    if(!ack){
        rtmI2cStop(device);
        return Stat830xI2cNack;
    }

    for(i=0; i < 2; i++){
        // write data
        ret = rtmI2cWrite(device, *(data+i), &ack);
        if(ret != Stat830xSuccess){
            return ret;
        }
        if(!ack){
            rtmI2cStop(device);
            return Stat830xI2cNack;
        }
    }
    // stop
    ret = rtmI2cStop(device);
    if(ret != Stat830xSuccess){
        return ret;
    }

    return Stat830xSuccess;

}

SIS830X_STATUS amc_ltc2493_Read(PSIS830X_DEVICE device, uint8_t i2c_device_addr, uint8_t *data)
{
    SIS830X_STATUS ret;

    //intI2cStop(device);

    int8_t bus_addr = i2c_device_addr << 1;
    uint8_t i;
    bool ack = false;

    ret = amcI2cStart(device);
    if(ret != Stat830xSuccess){
        return ret;
    }

    // readdress
    ret = amcI2cWrite(device, bus_addr | 1, &ack);
    if(ret != Stat830xSuccess){
        return ret;
    }
    if(!ack){
        amcI2cStop(device);
        return Stat830xI2cNack;
    }

    for(i=0; i < 4; i++){
        // read data
        ret = amcI2cRead(device, data+i, false);
        if(ret != Stat830xSuccess){
            return ret;
        }
    }

    // stop
    ret = amcI2cStop(device);
    if(ret != Stat830xSuccess){
        return ret;
    }

    return Stat830xSuccess;
}

SIS830X_STATUS amc_ltc2493_Write(PSIS830X_DEVICE device, uint8_t i2c_device_addr, uint8_t *data)
{
    SIS830X_STATUS ret;

    //intI2cStop(device);

    uint8_t bus_addr = i2c_device_addr << 1;
    uint8_t i;

    ret = amcI2cStart(device);
    if(ret != Stat830xSuccess){
        return ret;
    }

    // address slave
    bool ack = false;
    ret = amcI2cWrite(device, bus_addr, &ack);
    if(ret != Stat830xSuccess){
        return ret;
    }
    if(!ack){
        amcI2cStop(device);
        return Stat830xI2cNack;
    }

    for(i=0; i < 2; i++){
        // write data
        ret = amcI2cWrite(device, *(data+i), &ack);
        if(ret != Stat830xSuccess){
            return ret;
        }
        if(!ack){
            amcI2cStop(device);
            return Stat830xI2cNack;
        }
    }
    // stop
    ret = amcI2cStop(device);
    if(ret != Stat830xSuccess){
        return ret;
    }

    return Stat830xSuccess;

}

