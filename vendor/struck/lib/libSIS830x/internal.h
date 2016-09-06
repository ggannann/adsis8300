#ifndef INTERNAL_H_INCLUDED
#define INTERNAL_H_INCLUDED

// generic register i/o
SIS830X_STATUS intRegRead(PSIS830X_DEVICE device, uint32_t addr, uint32_t *data);
SIS830X_STATUS intRegWrite(PSIS830X_DEVICE device, uint32_t addr, uint32_t data);

// sample memory i/o
SIS830X_STATUS intReadMemory(PSIS830X_DEVICE device, uint32_t addr, uint32_t *data, uint32_t len);
SIS830X_STATUS intWriteMemory(PSIS830X_DEVICE device, uint32_t addr, uint32_t *data, uint32_t len);

// i2c bus handling
SIS830X_STATUS i2cStart(PSIS830X_DEVICE device, uint32_t interfaceRegister);
SIS830X_STATUS i2cStop(PSIS830X_DEVICE device, uint32_t interfaceRegister);
SIS830X_STATUS i2cWrite(PSIS830X_DEVICE device, uint32_t interfaceRegister , uint8_t data, bool *ack);
SIS830X_STATUS i2cRead(PSIS830X_DEVICE device, uint32_t interfaceRegister, uint8_t *data, bool ack);

// i2c bus handling
SIS830X_STATUS intI2cStart(PSIS830X_DEVICE device);
SIS830X_STATUS intI2cStop(PSIS830X_DEVICE device);
SIS830X_STATUS intI2cWrite(PSIS830X_DEVICE device, uint8_t data, bool *ack);
SIS830X_STATUS intI2cRead(PSIS830X_DEVICE device, uint8_t *data, bool ack);

// rtm i2c bus handling
SIS830X_STATUS rtmI2cStart(PSIS830X_DEVICE device);
SIS830X_STATUS rtmI2cStop(PSIS830X_DEVICE device);
SIS830X_STATUS rtmI2cWrite(PSIS830X_DEVICE device, uint8_t data, bool *ack);
SIS830X_STATUS rtmI2cRead(PSIS830X_DEVICE device, uint8_t *data, bool ack);

// amc i2c bus handling
SIS830X_STATUS amcI2cStart(PSIS830X_DEVICE device);
SIS830X_STATUS amcI2cStop(PSIS830X_DEVICE device);
SIS830X_STATUS amcI2cWrite(PSIS830X_DEVICE device, uint8_t data, bool *ack);
SIS830X_STATUS amcI2cRead(PSIS830X_DEVICE device, uint8_t *data, bool ack);
#endif // INTERNAL_H_INCLUDED
