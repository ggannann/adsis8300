#ifndef SIS830X_H_INCLUDED
#define SIS830X_H_INCLUDED

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "sis830xStat.h"
#include "sis830xType.h"
#include "sis830xReg.h"

#define SIS830X_MAX_CARDS 16

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// version
SIS830X_STATUS sis830x_Version(PSIS830X_VERSION version);

// device handling
SIS830X_STATUS sis830x_GetNumberOfDevices(int *num);
SIS830X_STATUS sis830x_OpenDeviceOnIdx(int idx, PSIS830X_DEVICE device);
SIS830X_STATUS sis830x_OpenDeviceOnPath(char *path, PSIS830X_DEVICE device);
SIS830X_STATUS sis830x_CloseDevice(PSIS830X_DEVICE device);

// general purpose register i/o
SIS830X_STATUS sis830x_ReadRegister(PSIS830X_DEVICE device, uint32_t addr, uint32_t *data);
SIS830X_STATUS sis830x_WriteRegister(PSIS830X_DEVICE device, uint32_t addr, uint32_t data);

// sample memory i/o
SIS830X_STATUS sis830x_ReadMemory(PSIS830X_DEVICE device, uint32_t addr, uint32_t *data, uint32_t len);
SIS830X_STATUS sis830x_WriteMemory(PSIS830X_DEVICE device, uint32_t addr, uint32_t *data, uint32_t len);

// spi flash access
SIS830X_STATUS sis830x_spiFlashRead(PSIS830X_DEVICE device, uint32_t addr, uint8_t *data, uint32_t len);
SIS830X_STATUS sis830x_spiFlashEraseBlock(PSIS830X_DEVICE device, uint32_t addr);
SIS830X_STATUS sis830x_spiFlashEraseChip(PSIS830X_DEVICE device);
SIS830X_STATUS sis830x_spiFlashProgramPage(PSIS830X_DEVICE device, uint32_t addr, uint8_t *data, uint32_t len);
SIS830X_STATUS sis830x_spiFlashUpdateFirmware(PSIS830X_DEVICE device, char *path, void (*cb)(int percent));

// general mgt synth register access
SIS830X_STATUS sis830x_si5338a_regRead(PSIS830X_DEVICE device, uint8_t addr, uint8_t *data);
SIS830X_STATUS sis830x_si5338a_regWrite(PSIS830X_DEVICE device, uint8_t addr, uint8_t data);
// si5338a mgt clksynth config
SIS830X_STATUS sis830x_si5338a_configDefault(PSIS830X_DEVICE device);
SIS830X_STATUS sis830x_si5338a_configUser(PSIS830X_DEVICE device, si5338a_Reg_Data *config, int len);

// general rtm i2c register access
SIS830X_STATUS sis830x_rtm_i2c_regRead(PSIS830X_DEVICE device, uint8_t i2c_device_addr, uint8_t reg_addr, uint8_t *data, uint8_t read_nof_byte);
SIS830X_STATUS sis830x_rtm_i2c_regWrite(PSIS830X_DEVICE device, uint8_t i2c_device_addr, uint8_t reg_addr, uint8_t *data, uint8_t write_nof_byte);

SIS830X_STATUS sis830x_rtm_pca9535_Read(PSIS830X_DEVICE device, uint8_t i2c_device_addr,uint8_t reg_addr, uint8_t *data);
SIS830X_STATUS sis830x_rtm_pca9535_Write(PSIS830X_DEVICE device, uint8_t i2c_device_addr, uint8_t reg_addr, uint8_t data);

SIS830X_STATUS sis830x_rtm_ltc2493_Read(PSIS830X_DEVICE device, uint8_t i2c_device_addr, uint32_t *data);
SIS830X_STATUS sis830x_rtm_ltc2493_Write(PSIS830X_DEVICE device, uint8_t i2c_device_addr, uint16_t *data);

SIS830X_STATUS sis830x_adc_tempSensor_Read(PSIS830X_DEVICE device, uint8_t i2c_device_addr, uint32_t *data);
SIS830X_STATUS sis830x_adc_tempSensor_Write(PSIS830X_DEVICE device, uint8_t i2c_device_addr, uint16_t *data);

// ad9510 divider setup
SIS830X_STATUS sis830x_AD9510_SPI_Setup(PSIS830X_DEVICE device, unsigned int* ch_divider_configuration_array, unsigned int ad9510_synch_cmd);

// error handling
void sis830x_status2str(SIS830X_STATUS status_in, char *text_out);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SIS830X_H_INCLUDED
