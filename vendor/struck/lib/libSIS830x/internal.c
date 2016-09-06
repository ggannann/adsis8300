#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "sis830x.h"
#include "internal.h"
#include "sis8300_defs.h"

SIS830X_STATUS intRegRead(PSIS830X_DEVICE device, uint32_t addr, uint32_t *data)
{
    int ret;
    sis8300_reg reg;

    reg.offset = addr;

    ret = ioctl(device->fp, SIS8300_REG_READ, &reg);
    if(ret < 0)
        return Stat830xIoctlError;

    *data = reg.data;
    return Stat830xSuccess;
}

SIS830X_STATUS intRegWrite(PSIS830X_DEVICE device, uint32_t addr, uint32_t data)
{
    int ret;
    sis8300_reg reg;

    reg.offset = addr;
    reg.data = data;

    ret = ioctl(device->fp, SIS8300_REG_WRITE, &reg);
    if(ret < 0)
        return Stat830xIoctlError;

    return Stat830xSuccess;
}

SIS830X_STATUS intReadMemory(PSIS830X_DEVICE device, uint32_t addr, uint32_t *data, uint32_t len)
{
    ssize_t ret;
    SIS830X_STATUS stat;

    stat = intRegWrite(device, 0x202, addr & 0xFFFFFFE0); // 32byte boundary
    if(stat != Stat830xSuccess){
        return stat;
    }

    ret = read(device->fp, data, len);
    if(ret < 0){
        return Stat830xReadCallError;
    }
    if(ret != len){
        return Stat830xReadLenError;
    }

    return Stat830xSuccess;
}

SIS830X_STATUS intWriteMemory(PSIS830X_DEVICE device, uint32_t addr, uint32_t *data, uint32_t len)
{
    ssize_t ret;
    SIS830X_STATUS stat;

    stat = intRegWrite(device, 0x230, 1);
    if(stat != Stat830xSuccess){
        return stat;
    }

    stat = intRegWrite(device, 0x212, addr & 0xFFFFFFE0); // 32byte boundary
    if(stat != Stat830xSuccess){
        return stat;
    }

    ret = write(device->fp, data, len);
    intRegWrite(device, 0x230, 0);
    if(ret < 0){
        return Stat830xWriteCallError;
    }
    if(ret != len){
        return Stat830xWriteLenError;
    }

    return Stat830xSuccess;
}
// i2c bus handling
SIS830X_STATUS inline i2cStart(PSIS830X_DEVICE device, uint32_t interfaceRegister)
{
    SIS830X_STATUS ret;

    ret = intRegWrite(device, interfaceRegister, 1<<I2C_START);
    if(ret != Stat830xSuccess){
        // try to free bus
        intRegWrite(device, interfaceRegister, 1<<I2C_STOP);
        return ret;
    }

    uint32_t reg;
    int loops = 1000;
    do{
        ret = intRegRead(device, interfaceRegister, &reg);
        if(ret != Stat830xSuccess){
            // try to free bus
            intRegWrite(device, interfaceRegister, 1<<I2C_STOP);
            return ret;
        }
        usleep(1);
        loops--;
    }while((loops > 0) && (reg & 1<<I2C_BUSY));
    if(loops == 0){
        // try to free bus
        intRegWrite(device, interfaceRegister, 1<<I2C_STOP);
        return Stat830xI2cBusy;
    }

    return Stat830xSuccess;
}

SIS830X_STATUS inline i2cStop(PSIS830X_DEVICE device, uint32_t interfaceRegister)
{
    SIS830X_STATUS ret;

    ret = intRegWrite(device, interfaceRegister, 1<<I2C_STOP);
    if(ret != Stat830xSuccess){
        return ret;
    }

    uint32_t reg;
    int loops = 1000;
    do{
        ret = intRegRead(device, interfaceRegister, &reg);
        if(ret != Stat830xSuccess){
            return ret;
        }
        usleep(1);
        loops--;
    }while((loops > 0) && (reg & 1<<I2C_BUSY));
    if(loops == 0){
        return Stat830xI2cBusy;
    }

    return Stat830xSuccess;
}

SIS830X_STATUS inline i2cWrite(PSIS830X_DEVICE device, uint32_t interfaceRegister, uint8_t data, bool *ack)
{
    SIS830X_STATUS ret;

    ret = intRegWrite(device, interfaceRegister, 1<<I2C_WRITE_CYCLE | data);
    if(ret != Stat830xSuccess){
        // try to free bus
        intRegWrite(device, interfaceRegister, 1<<I2C_STOP);
        return ret;
    }

    uint32_t reg;
    int loops = 1000;
    do{
        ret = intRegRead(device, interfaceRegister, &reg);
        if(ret != Stat830xSuccess){
            // try to free bus
            intRegWrite(device, interfaceRegister, 1<<I2C_STOP);
            return ret;
        }
        usleep(1);
        loops--;
    }while((loops > 0) && (reg & 1<<I2C_BUSY));
    if(loops == 0){
        // try to free bus
        intRegWrite(device, interfaceRegister, 1<<I2C_STOP);
        return Stat830xI2cBusy;
    }

    *ack = false;
    if(reg & 1<<I2C_ACK)
        *ack = true;

    return Stat830xSuccess;
}

SIS830X_STATUS inline i2cRead(PSIS830X_DEVICE device, uint32_t interfaceRegister, uint8_t *data, bool ack)
{
    SIS830X_STATUS ret;
    uint32_t reg = ack ? (1<<I2C_ACK) : 0;

    ret = intRegWrite(device, interfaceRegister, 1<<I2C_READ_CYCLE | reg);
    if(ret != Stat830xSuccess){
        // try to free bus
        intRegWrite(device, interfaceRegister, 1<<I2C_STOP);
        return ret;
    }

    int loops = 1000;
    do{
        ret = intRegRead(device, interfaceRegister, &reg);
        if(ret != Stat830xSuccess){
            // try to free bus
            intRegWrite(device, interfaceRegister, 1<<I2C_STOP);
            return ret;
        }
        usleep(1);
        loops--;
    }while((loops > 0) && (reg & 1<<I2C_BUSY));
    if(loops == 0){
        // try to free bus
        intRegWrite(device, interfaceRegister, 1<<I2C_STOP);
        return Stat830xI2cBusy;
    }

    *data = (uint8_t)reg;

    return Stat830xSuccess;
}


// i2c bus handling
SIS830X_STATUS intI2cStart(PSIS830X_DEVICE device)
{
    /*
    SIS830X_STATUS ret;

    ret = intRegWrite(device, SIS830X_MGTCLK_SYNTH_I2C_REG, 1<<I2C_START);
    if(ret != Stat830xSuccess){
        // try to free bus
        intRegWrite(device, SIS830X_MGTCLK_SYNTH_I2C_REG, 1<<I2C_STOP);
        return ret;
    }

    uint32_t reg;
    int loops = 1000;
    do{
        ret = intRegRead(device, SIS830X_MGTCLK_SYNTH_I2C_REG, &reg);
        if(ret != Stat830xSuccess){
            // try to free bus
            intRegWrite(device, SIS830X_MGTCLK_SYNTH_I2C_REG, 1<<I2C_STOP);
            return ret;
        }
        usleep(1);
        loops--;
    }while((loops > 0) && (reg & 1<<I2C_BUSY));
    if(loops == 0){
        // try to free bus
        intRegWrite(device, SIS830X_MGTCLK_SYNTH_I2C_REG, 1<<I2C_STOP);
        return Stat830xI2cBusy;
    }

    return Stat830xSuccess;*/

    return i2cStart(device, SIS830X_MGTCLK_SYNTH_I2C_REG);
}

SIS830X_STATUS intI2cStop(PSIS830X_DEVICE device)
{
    /*
    SIS830X_STATUS ret;

    ret = intRegWrite(device, SIS830X_MGTCLK_SYNTH_I2C_REG, 1<<I2C_STOP);
    if(ret != Stat830xSuccess){
        return ret;
    }

    uint32_t reg;
    int loops = 1000;
    do{
        ret = intRegRead(device, SIS830X_MGTCLK_SYNTH_I2C_REG, &reg);
        if(ret != Stat830xSuccess){
            return ret;
        }
        usleep(1);
        loops--;
    }while((loops > 0) && (reg & 1<<I2C_BUSY));
    if(loops == 0){
        return Stat830xI2cBusy;
    }

    return Stat830xSuccess;*/

    return i2cStop(device, SIS830X_MGTCLK_SYNTH_I2C_REG);

}

SIS830X_STATUS intI2cWrite(PSIS830X_DEVICE device, uint8_t data, bool *ack)
{
    /*
    SIS830X_STATUS ret;

    ret = intRegWrite(device, SIS830X_MGTCLK_SYNTH_I2C_REG, 1<<I2C_WRITE_CYCLE | data);
    if(ret != Stat830xSuccess){
        // try to free bus
        intRegWrite(device, SIS830X_MGTCLK_SYNTH_I2C_REG, 1<<I2C_STOP);
        return ret;
    }

    uint32_t reg;
    int loops = 1000;
    do{
        ret = intRegRead(device, SIS830X_MGTCLK_SYNTH_I2C_REG, &reg);
        if(ret != Stat830xSuccess){
            // try to free bus
            intRegWrite(device, SIS830X_MGTCLK_SYNTH_I2C_REG, 1<<I2C_STOP);
            return ret;
        }
        usleep(1);
        loops--;
    }while((loops > 0) && (reg & 1<<I2C_BUSY));
    if(loops == 0){
        // try to free bus
        intRegWrite(device, SIS830X_MGTCLK_SYNTH_I2C_REG, 1<<I2C_STOP);
        return Stat830xI2cBusy;
    }

    *ack = false;
    if(reg & 1<<I2C_ACK)
        *ack = true;

    return Stat830xSuccess;*/

    return i2cWrite(device,SIS830X_MGTCLK_SYNTH_I2C_REG, data, ack);
}

SIS830X_STATUS intI2cRead(PSIS830X_DEVICE device, uint8_t *data, bool ack)
{ /*
    SIS830X_STATUS ret;
    uint32_t reg = ack ? (1<<I2C_ACK) : 0;

    ret = intRegWrite(device, SIS830X_MGTCLK_SYNTH_I2C_REG, 1<<I2C_READ_CYCLE | reg);
    if(ret != Stat830xSuccess){
        // try to free bus
        intRegWrite(device, SIS830X_MGTCLK_SYNTH_I2C_REG, 1<<I2C_STOP);
        return ret;
    }

    int loops = 1000;
    do{
        ret = intRegRead(device, SIS830X_MGTCLK_SYNTH_I2C_REG, &reg);
        if(ret != Stat830xSuccess){
            // try to free bus
            intRegWrite(device, SIS830X_MGTCLK_SYNTH_I2C_REG, 1<<I2C_STOP);
            return ret;
        }
        usleep(1);
        loops--;
    }while((loops > 0) && (reg & 1<<I2C_BUSY));
    if(loops == 0){
        // try to free bus
        intRegWrite(device, SIS830X_MGTCLK_SYNTH_I2C_REG, 1<<I2C_STOP);
        return Stat830xI2cBusy;
    }

    *data = (uint8_t)reg;

    return Stat830xSuccess;*/

    return i2cRead(device, SIS830X_MGTCLK_SYNTH_I2C_REG, data, ack);
}


// RTM i2c bus handling
SIS830X_STATUS rtmI2cStart(PSIS830X_DEVICE device)
{
    return i2cStart(device, SIS830X_RTM_I2C_REG);
}

SIS830X_STATUS rtmI2cStop(PSIS830X_DEVICE device)
{
    return i2cStop(device, SIS830X_RTM_I2C_REG);
}

SIS830X_STATUS rtmI2cWrite(PSIS830X_DEVICE device, uint8_t data, bool *ack)
{
    return i2cWrite(device,SIS830X_RTM_I2C_REG, data, ack);
}

SIS830X_STATUS rtmI2cRead(PSIS830X_DEVICE device, uint8_t *data, bool ack)
{
    return i2cRead(device, SIS830X_RTM_I2C_REG, data, ack);
}


// AMC temperatur i2c bus handling
SIS830X_STATUS amcI2cStart(PSIS830X_DEVICE device)
{
    return i2cStart(device, SIS830X_ADC_TEMP_I2C_REG);
}

SIS830X_STATUS amcI2cStop(PSIS830X_DEVICE device)
{
    return i2cStop(device, SIS830X_ADC_TEMP_I2C_REG);
}

SIS830X_STATUS amcI2cWrite(PSIS830X_DEVICE device, uint8_t data, bool *ack)
{
    return i2cWrite(device,SIS830X_ADC_TEMP_I2C_REG, data, ack);
}

SIS830X_STATUS amcI2cRead(PSIS830X_DEVICE device, uint8_t *data, bool ack)
{
    return i2cRead(device, SIS830X_ADC_TEMP_I2C_REG, data, ack);
}
