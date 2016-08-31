#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "sis830x.h"
#include "sis830xVer.h"
#include "internal.h"
#include "spiFlash.h"
#include "si5338aSynth.h"
#include "ad9510.h"
#include "rtmI2C.h"

// version
SIS830X_STATUS sis830x_Version(PSIS830X_VERSION version)
{
    // sanity checks
    if(version == NULL){
        return Stat830xNullArgument;
    }

    version->major = API_VERSION_MAJOR;
    version->minor = API_VERSION_MINOR;

    return Stat830xSuccess;
}

// device handling
SIS830X_STATUS sis830x_GetNumberOfDevices(int *num)
{
    // sanity checks
    if(num == NULL){
        return Stat830xNullArgument;
    }

    char devNode[32];
    *num = 0;
    // poll device nodes
    int i;
    for(i = 0;i < SIS830X_MAX_CARDS;i++){
        sprintf(devNode, "/dev/sis8300-%d", i);
        int fp = open(devNode, O_RDWR);
        if(fp >= 0){
            (*num)++;
            close(fp);
        }
    }

    return Stat830xSuccess;
}

SIS830X_STATUS sis830x_OpenDeviceOnIdx(int idx, PSIS830X_DEVICE device)
{
    // sanity checks
    if(device == NULL || idx >= SIS830X_MAX_CARDS){
        return Stat830xNullArgument;
    }
    if(device->open == true){
        return Stat830xAlreadyOpen;
    }

    char devNode[32];

    sprintf(devNode, "/dev/sis8300-%d", idx);
    int fp = open(devNode, O_RDWR);
    if(fp < 0){
        return Stat830xInvalidDeviceIndex;
    }

    device->fp = fp;
    device->open = true;
    device->type = UNKNOWN;

    uint32_t reg;
    intRegRead(device, SIS830X_MODULE_VERSION_REG, &reg);

    if(reg & 0x00010000){
        device->type = SIS8300L;
    }else{
        device->type = SIS8300;
    }

    return Stat830xSuccess;
}

SIS830X_STATUS sis830x_OpenDeviceOnPath(char *path, PSIS830X_DEVICE device)
{
    // sanity checks
    if(device == NULL || path == NULL){
        return Stat830xNullArgument;
    }
    if(device->open == true){
        return Stat830xAlreadyOpen;
    }

    int fp = open(path, O_RDWR);
    if(fp < 0){
        return Stat830xInvalidDeviceIndex;
    }

    device->fp = fp;
    device->open = true;
    device->type = UNKNOWN;

    uint32_t reg;
    intRegRead(device, SIS830X_MODULE_VERSION_REG, &reg);

    if(reg & 0x00010000){
        device->type = SIS8300L;
    }else{
        device->type = SIS8300;
    }

    return Stat830xSuccess;
}

SIS830X_STATUS sis830x_CloseDevice(PSIS830X_DEVICE device)
{
    // sanity checks
    if(device == NULL){
        return Stat830xNullArgument;
    }
    if(device->open == false){
        return Stat830xNotOpen;
    }

    device->type = UNKNOWN;
    device->open = false;
    close(device->fp);

    return Stat830xSuccess;
}

// general purpose register i/o
SIS830X_STATUS sis830x_ReadRegister(PSIS830X_DEVICE device, uint32_t addr, uint32_t *data)
{
    // sanity checks
    if(device == NULL || data == NULL){
        return Stat830xNullArgument;
    }
    if(device->open == false){
        return Stat830xNotOpen;
    }

    return intRegRead(device, addr, data);
}

SIS830X_STATUS sis830x_WriteRegister(PSIS830X_DEVICE device, uint32_t addr, uint32_t data)
{
    // sanity checks
    if(device == NULL){
        return Stat830xNullArgument;
    }
    if(device->open == false){
        return Stat830xNotOpen;
    }

    return intRegWrite(device, addr, data);
}

// sample memory i/o
SIS830X_STATUS sis830x_ReadMemory(PSIS830X_DEVICE device, uint32_t addr, uint32_t *data, uint32_t len)
{
    // sanity check
    if(device == NULL || data == NULL){
        return Stat830xNullArgument;
    }
    if(device->open == false){
        return Stat830xNotOpen;
    }

    return intReadMemory(device, addr, data, len);
}

SIS830X_STATUS sis830x_WriteMemory(PSIS830X_DEVICE device, uint32_t addr, uint32_t *data, uint32_t len)
{
    // sanity check
    if(device == NULL || data == NULL){
        return Stat830xNullArgument;
    }
    if(device->open == false){
        return Stat830xNotOpen;
    }

    return intWriteMemory(device, addr, data, len);
}

// spi flash access
SIS830X_STATUS sis830x_spiFlashRead(PSIS830X_DEVICE device, uint32_t addr, uint8_t *data, uint32_t len)
{
    // sanity checks
    if(device == NULL || data == NULL){
        return Stat830xNullArgument;
    }
    if(device->open == false){
        return Stat830xNotOpen;
    }
    if(device->type != SIS8300L){
        return Stat830xNotSupported;
    }

    return spiFlashRead(device, addr, data, len);
}

SIS830X_STATUS sis830x_spiFlashEraseBlock(PSIS830X_DEVICE device, uint32_t addr)
{
    // sanity checks
    if(device == NULL){
        return Stat830xNullArgument;
    }
    if(device->open == false){
        return Stat830xNotOpen;
    }
    if(device->type != SIS8300L){
        return Stat830xNotSupported;
    }

    return spiFlashEraseBlock(device, addr);
}

SIS830X_STATUS sis830x_spiFlashEraseChip(PSIS830X_DEVICE device)
{
    // sanity checks
    if(device == NULL){
        return Stat830xNullArgument;
    }
    if(device->open == false){
        return Stat830xNotOpen;
    }
    if(device->type != SIS8300L){
        return Stat830xNotSupported;
    }

    return spiFlashEraseChip(device);
}

SIS830X_STATUS sis830x_spiFlashProgramPage(PSIS830X_DEVICE device, uint32_t addr, uint8_t *data, uint32_t len)
{
    // sanity checks
    if(device == NULL || data == NULL){
        return Stat830xNullArgument;
    }
    if(device->open == false){
        return Stat830xNotOpen;
    }
    if(device->type != SIS8300L){
        return Stat830xNotSupported;
    }

    return spiFlashProgramPage(device, addr, data, len);
}

SIS830X_STATUS sis830x_spiFlashUpdateFirmware(PSIS830X_DEVICE device, char *path, void (*cb)(int percent))
{
    // sanity check
    if(device == NULL || path == NULL){
        return Stat830xNullArgument;
    }
    if(device->open == false){
        return Stat830xNotOpen;
    }
    if(device->type != SIS8300L){
        return Stat830xNotSupported;
    }

    return spiFlashUpdateFirmware(device, path, cb);
}

// general mgt synth register access
SIS830X_STATUS sis830x_si5338a_regRead(PSIS830X_DEVICE device, uint8_t addr, uint8_t *data)
{
    // sanity check
    if(device == NULL || data == NULL){
        return Stat830xNullArgument;
    }
    if(device->open == false){
        return Stat830xNotOpen;
    }
    if(device->type != SIS8300L){
        return Stat830xNotSupported;
    }

    return si5338a_regRead(device, addr, data);
}

SIS830X_STATUS sis830x_si5338a_regWrite(PSIS830X_DEVICE device, uint8_t addr, uint8_t data)
{
    // sanity check
    if(device == NULL){
        return Stat830xNullArgument;
    }
    if(device->open == false){
        return Stat830xNotOpen;
    }
    if(device->type != SIS8300L){
        return Stat830xNotSupported;
    }

    return si5338a_regWrite(device, addr, data);
}

// mgt synth config setup
SIS830X_STATUS sis830x_si5338a_configDefault(PSIS830X_DEVICE device)
{
    // sanity check
    if(device == NULL){
        return Stat830xNullArgument;
    }
    if(device->open == false){
        return Stat830xNotOpen;
    }
    if(device->type != SIS8300L){
        return Stat830xNotSupported;
    }

    return si5338a_configDefault(device);
}

SIS830X_STATUS sis830x_si5338a_configUser(PSIS830X_DEVICE device, si5338a_Reg_Data *config, int len)
{
    // sanity check
    if(device == NULL){
        return Stat830xNullArgument;
    }
    if(device->open == false){
        return Stat830xNotOpen;
    }
    if(device->type != SIS8300L){
        return Stat830xNotSupported;
    }

    return si5338a_configUser(device, config, len);
}

// general rtm i2c register access
SIS830X_STATUS sis830x_rtm_i2c_regRead(PSIS830X_DEVICE device, uint8_t bus_addr, uint8_t reg_addr, uint8_t *data)
{
    // sanity check
    if(device == NULL || data == NULL){
        return Stat830xNullArgument;
    }
    if(device->open == false){
        return Stat830xNotOpen;
    }
    if(device->type != SIS8300L){
        return Stat830xNotSupported;
    }

    return rtm_i2c_regRead(device, bus_addr, reg_addr, data);
}

SIS830X_STATUS sis830x_rtm_i2c_regWrite(PSIS830X_DEVICE device, uint8_t bus_addr, uint8_t reg_addr, uint8_t data)
{
    // sanity check
    if(device == NULL){
        return Stat830xNullArgument;
    }
    if(device->open == false){
        return Stat830xNotOpen;
    }
    if(device->type != SIS8300L){
        return Stat830xNotSupported;
    }

    return rtm_i2c_regWrite(device, bus_addr, reg_addr, data);
}

// ad9510 divider setup
SIS830X_STATUS sis830x_AD9510_SPI_Setup(PSIS830X_DEVICE device, unsigned int* ch_divider_configuration_array, unsigned int ad9510_synch_cmd)
{
    // sanity check
    if(device == NULL || ch_divider_configuration_array == NULL){
        return Stat830xNullArgument;
    }
    if(device->open == false){
        return Stat830xNotOpen;
    }

    return AD9510_SPI_Setup(device, ch_divider_configuration_array, ad9510_synch_cmd);
}

// error handling
void sis830x_status2str(SIS830X_STATUS status_in, char *text_out){

    switch(status_in){
        case 0:
            strcpy(text_out, "Nix");
        break;
        case 1:

        break;
        case 2:

        break;
        case 3:

        break;
        case 4:

        break;
        case 5:

        break;
        case 6:

        break;
        case 7:

        break;
        case 8:

        break;
        default:

        break;
    }

}
