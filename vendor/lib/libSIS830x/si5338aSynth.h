#ifndef SI5338ASYNTH_H_INCLUDED
#define SI5338ASYNTH_H_INCLUDED

#define SI5338A_ADR 0xE0 //shifted one left (device addr 0x70)

SIS830X_STATUS si5338a_regRead(PSIS830X_DEVICE device, uint8_t addr, uint8_t *data);
SIS830X_STATUS si5338a_regWrite(PSIS830X_DEVICE device, uint8_t addr, uint8_t data);

SIS830X_STATUS si5338a_configDefault(PSIS830X_DEVICE device);
SIS830X_STATUS si5338a_configUser(PSIS830X_DEVICE device, si5338a_Reg_Data *config, int len);

#endif // SI5338ASYNTH_H_INCLUDED
