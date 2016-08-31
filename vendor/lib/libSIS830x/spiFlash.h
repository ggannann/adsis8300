#ifndef SPIFLASH_H_INCLUDED
#define SPIFLASH_H_INCLUDED

#define SIS8300L_FLASHSIZE 8388608 // 8MB
#define SIS8300L_PAGESIZE 256 // 256B
#define SIS8300L_BLOCKSIZE 65536 // 64kB
#define SIS8300L_READBLOCKSIZE 2048 // 2kB

SIS830X_STATUS spiByteExch(PSIS830X_DEVICE device, uint8_t in, uint8_t *out);

SIS830X_STATUS spiFlashRead(PSIS830X_DEVICE device, uint32_t addr, uint8_t *data, uint32_t len);
SIS830X_STATUS spiFlashReadBlock(PSIS830X_DEVICE device, uint32_t addr, uint8_t *data, uint32_t len);
SIS830X_STATUS spiFlashEraseBlock(PSIS830X_DEVICE device, uint32_t addr);
SIS830X_STATUS spiFlashEraseChip(PSIS830X_DEVICE device);
SIS830X_STATUS spiFlashProgramPage(PSIS830X_DEVICE device, uint32_t addr, uint8_t *data, uint32_t len);
SIS830X_STATUS spiFlashUpdateFirmware(PSIS830X_DEVICE device, char *path, void (*cb)(int percent));
SIS830X_STATUS spiFlashWriteEnable(PSIS830X_DEVICE device);
SIS830X_STATUS spiFlashReadBusy(PSIS830X_DEVICE device, bool *busy);
SIS830X_STATUS spiFlashPollBusy(PSIS830X_DEVICE device, int timeout);

#endif // SPIFLASH_H_INCLUDED
