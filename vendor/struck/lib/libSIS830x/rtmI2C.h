#ifndef RTMI2C_H_INCLUDED
#define RTMI2C_H_INCLUDED

SIS830X_STATUS rtm_i2c_regRead(PSIS830X_DEVICE device, uint8_t i2c_device_addr, uint8_t reg_addr, uint8_t *data, uint8_t read_nof_byte);
SIS830X_STATUS rtm_i2c_regWrite(PSIS830X_DEVICE device, uint8_t i2c_device_addr, uint8_t reg_addr, uint8_t *data, uint8_t write_nof_byte);

SIS830X_STATUS rtm_pca9535_Read(PSIS830X_DEVICE device, uint8_t i2c_device_addr, uint8_t reg_addr, uint8_t *data);
SIS830X_STATUS rtm_pca9535_Write(PSIS830X_DEVICE device, uint8_t i2c_device_addr, uint8_t reg_addr, uint8_t *data);

SIS830X_STATUS rtm_ltc2493_Read(PSIS830X_DEVICE device, uint8_t i2c_device_addr, uint8_t *data);
SIS830X_STATUS rtm_ltc2493_Write(PSIS830X_DEVICE device, uint8_t i2c_device_addr, uint8_t *data);


// Only for tests!!
SIS830X_STATUS amc_ltc2493_Read(PSIS830X_DEVICE device, uint8_t i2c_device_addr, uint8_t *data);
SIS830X_STATUS amc_ltc2493_Write(PSIS830X_DEVICE device, uint8_t i2c_device_addr, uint8_t *data);

#endif // RTMI2C_H_INCLUDED
