#include <unistd.h>

#include "sis830x.h"
#include "internal.h"
#include "si5338aSynth.h"
#include "si5338aSynthDefault.h"

SIS830X_STATUS si5338a_regRead(PSIS830X_DEVICE device, uint8_t addr, uint8_t *data)
{
    SIS830X_STATUS ret;

    //intI2cStop(device);

    ret = intI2cStart(device);
    if(ret != Stat830xSuccess){
        return ret;
    }

    // address slave
    bool ack = false;
    ret = intI2cWrite(device, SI5338A_ADR, &ack);
    if(ret != Stat830xSuccess){
        return ret;
    }
    if(!ack){
        intI2cStop(device);
        return Stat830xI2cNack;
    }

    // write register address
    ret = intI2cWrite(device, addr, &ack);
    if(ret != Stat830xSuccess){
        return ret;
    }
    if(!ack){
        intI2cStop(device);
        return Stat830xI2cNack;
    }

    // stop-start
    ret = intI2cStop(device);
    if(ret != Stat830xSuccess){
        return ret;
    }
    ret = intI2cStart(device);
    if(ret != Stat830xSuccess){
        return ret;
    }

    // readdress
    ret = intI2cWrite(device, SI5338A_ADR | 1, &ack);
    if(ret != Stat830xSuccess){
        return ret;
    }
    if(!ack){
        intI2cStop(device);
        return Stat830xI2cNack;
    }

    // read data
    ret = intI2cRead(device, data, false);
    if(ret != Stat830xSuccess){
        return ret;
    }

    // stop
    ret = intI2cStop(device);
    if(ret != Stat830xSuccess){
        return ret;
    }

    return Stat830xSuccess;
}

SIS830X_STATUS si5338a_regWrite(PSIS830X_DEVICE device, uint8_t addr, uint8_t data)
{
    SIS830X_STATUS ret;

    //intI2cStop(device);

    ret = intI2cStart(device);
    if(ret != Stat830xSuccess){
        return ret;
    }

    // address slave
    bool ack = false;
    ret = intI2cWrite(device, SI5338A_ADR, &ack);
    if(ret != Stat830xSuccess){
        return ret;
    }
    if(!ack){
        intI2cStop(device);
        return Stat830xI2cNack;
    }

    // write register address
    ret = intI2cWrite(device, addr, &ack);
    if(ret != Stat830xSuccess){
        return ret;
    }
    if(!ack){
        intI2cStop(device);
        return Stat830xI2cNack;
    }

    // write data
    ret = intI2cWrite(device, data, &ack);
    if(ret != Stat830xSuccess){
        return ret;
    }
    if(!ack){
        intI2cStop(device);
        return Stat830xI2cNack;
    }

    // stop
    ret = intI2cStop(device);
    if(ret != Stat830xSuccess){
        return ret;
    }

    return Stat830xSuccess;
}

SIS830X_STATUS si5338a_configDefault(PSIS830X_DEVICE device)
{
    return si5338a_configUser(device, Reg_Store_in12_25MHz, NUM_REGS_MAX);
}

SIS830X_STATUS si5338a_configUser(PSIS830X_DEVICE device, si5338a_Reg_Data *config, int len)
{
    // config sequence taken from SiLabs 'SI5338.pdf' Rev 1.3 Figure 9, page 22
    SIS830X_STATUS ret;

    // disable all outputs, OEB_ALL = 1, register 230
    ret = si5338a_regWrite(device, 230, 0x10);
    if(ret != Stat830xSuccess){
        return ret;
    }
    // pause loss of lock, DIS_LOL = 1, register 241
    ret = si5338a_regWrite(device, 241, 0x80);
    if(ret != Stat830xSuccess){
        return ret;
    }

    // write user config table
    int i;
    uint8_t reg;
    for(i = 0;i < len;i++){
        // check mask
        if(config[i].Reg_Mask != 0x00){
            // register needs changing
            if(config[i].Reg_Mask == 0xFF){
                // simple write of all bits
                ret = si5338a_regWrite(device, config[i].Reg_Addr, config[i].Reg_Val);
                if(ret != Stat830xSuccess){
                    return ret;
                }
            }else{
                // read-modify-write with supplied mask
                ret = si5338a_regRead(device, config[i].Reg_Addr, &reg);
                if(ret != Stat830xSuccess){
                    return ret;
                }
                // mask
                reg &= ~(config[i].Reg_Mask);
                reg |= config[i].Reg_Val & config[i].Reg_Mask;
                // write back
                ret = si5338a_regWrite(device, config[i].Reg_Addr, reg);
                if(ret != Stat830xSuccess){
                    return ret;
                }
            }
        }
    }

    // validate input clock
    /*
    ret = si5338a_regRead(device, 218, &reg);
    if(ret != Stat830xSuccess){
        return ret;
    }
    */

    // configure pll for locking, FCAL_OVRD_EN = 0, register 49
    // r-m-w
    ret = si5338a_regRead(device, 49, &reg);
    if(ret != Stat830xSuccess){
        return ret;
    }
    reg &= ~(0x80);
    ret = si5338a_regWrite(device, 49, reg);
    if(ret != Stat830xSuccess){
        return ret;
    }

    // initiate locking of pll, SOFT_RESET = 1, register 246
    ret = si5338a_regWrite(device, 246, 0x02);
    if(ret != Stat830xSuccess){
        return ret;
    }

    // wait 25ms for pll lock
    usleep(25000);

    // restart lol, DIS_LOL = 0 + fixed 0x65, register 241
    ret = si5338a_regWrite(device, 241, 0);
    if(ret != Stat830xSuccess){
        return ret;
    }
    ret = si5338a_regWrite(device, 241, 0x65);
    if(ret != Stat830xSuccess){
        return ret;
    }

    // confirm PLL lock status
    int loops = 1000;
    do{
        ret = si5338a_regRead(device, 218, &reg);
        if(ret != Stat830xSuccess){
            return ret;
        }
        loops--;
        usleep(1000);
        reg &= 0x11; // mask PLL_LOL + SYS_CAL
    }while((reg != 0) && (loops > 0));
    if(loops == 0){
        return Stat830xSynthNoLock;
    }

    // copy FCAL registers to using set
    // 235      -> 45
    // 236      -> 46
    // 237[1:0] -> 47[1:0] + pattern
    ret = si5338a_regRead(device, 235, &reg);
    if(ret != Stat830xSuccess){
        return ret;
    }
    ret = si5338a_regWrite(device, 45, reg);
    if(ret != Stat830xSuccess){
        return ret;
    }

    ret = si5338a_regRead(device, 236, &reg);
    if(ret != Stat830xSuccess){
        return ret;
    }
    ret = si5338a_regWrite(device, 46, reg);
    if(ret != Stat830xSuccess){
        return ret;
    }

    ret = si5338a_regRead(device, 237, &reg);
    if(ret != Stat830xSuccess){
        return ret;
    }
    reg &= ~(0x03);
    reg |= 0x14;
    ret = si5338a_regWrite(device, 47, reg);
    if(ret != Stat830xSuccess){
        return ret;
    }

    // set pll to use FCAL values
    // r-m-w
    ret = si5338a_regRead(device, 49, &reg);
    if(ret != Stat830xSuccess){
        return ret;
    }
    reg |= 0x80;
    ret = si5338a_regWrite(device, 49, reg);
    if(ret != Stat830xSuccess){
        return ret;
    }

    // enable outputs
    ret = si5338a_regWrite(device, 230, 0);
    if(ret != Stat830xSuccess){
        return ret;
    }

    return Stat830xSuccess;
}
