#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sis830x.h"
#include "internal.h"
#include "spiFlash.h"

SIS830X_STATUS spiByteExch(PSIS830X_DEVICE device, uint8_t in, uint8_t *out)
{
    SIS830X_STATUS ret;
    int maxBusyLoops = 1000;

    // start transfer
    ret = intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_EXCH) | in);
    if(ret != Stat830xSuccess){
        return ret;
    }

    // poll fsm busy
    uint32_t reg;
    bool busy = true;
    while(busy == true && maxBusyLoops > 0){
        ret = intRegRead(device, SIS830X_SPI_FLASH_CSR, &reg);
        if(ret != Stat830xSuccess){
            return ret;
        }

        if(!(reg & (1<<SPI_FLASH_BUSY))){
            busy = false;
        }

        usleep(1);
        maxBusyLoops--;
    }
    if(maxBusyLoops == 0){
        return Stat830xSpiBusy;
    }

    if(out){
        *out = (uint8_t)reg;
    }

    return Stat830xSuccess;
}

SIS830X_STATUS spiFlashRead(PSIS830X_DEVICE device, uint32_t addr, uint8_t *data, uint32_t len)
{
    SIS830X_STATUS ret;
    int i;

    // gain control of spi lines
    ret = intRegWrite(device, SIS830X_SPI_FLASH_CSR, 1<<SPI_FLASH_MUX_EN);
    if(ret != Stat830xSuccess){
        return ret;
    }

    // select flash
    ret = intRegWrite(device, SIS830X_SPI_FLASH_CSR, 1<<SPI_FLASH_CS);
    if(ret != Stat830xSuccess){
        // try to disable flash mux back to mmc access
        intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_MUX_EN)<<16);
        return ret;
    }

    // load command, READ DATA = 0x03
    ret = spiByteExch(device, 0x03, NULL);
    if(ret != Stat830xSuccess){
        // try to disable flash mux back to mmc access
        intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_CS)<<16);
        intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_MUX_EN)<<16);
        return ret;
    }

    // load address, 24bit
    for(i = 2;i >= 0;i--){
        ret = spiByteExch(device, (addr >> (8 * i)) & 0xFF, NULL);
        if(ret != Stat830xSuccess){
            // try to disable flash mux back to mmc access
            intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_CS)<<16);
            intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_MUX_EN)<<16);
            return ret;
        }
    }

    // read data out
    for(i = 0;i < len;i++){
        ret = spiByteExch(device, 0, data+i);
    }

    // deselect flash and return mux to mmc access
    ret = intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_CS)<<16);
    ret = intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_MUX_EN)<<16);
    if(ret != Stat830xSuccess){
        return ret;
    }

    return Stat830xSuccess;
}

SIS830X_STATUS spiFlashReadBlock(PSIS830X_DEVICE device, uint32_t addr, uint8_t *data, uint32_t len)
{
    SIS830X_STATUS ret;
    int i;

    // gain control of spi lines
    ret = intRegWrite(device, SIS830X_SPI_FLASH_CSR, 1<<SPI_FLASH_MUX_EN);
    if(ret != Stat830xSuccess){
        return ret;
    }

    // select flash
    ret = intRegWrite(device, SIS830X_SPI_FLASH_CSR, 1<<SPI_FLASH_CS);
    if(ret != Stat830xSuccess){
        // try to disable flash mux back to mmc access
        intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_MUX_EN)<<16);
        return ret;
    }

    // load command, READ DATA = 0x03
    ret = spiByteExch(device, 0x03, NULL);
    if(ret != Stat830xSuccess){
        // try to disable flash mux back to mmc access
        intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_CS)<<16);
        intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_MUX_EN)<<16);
        return ret;
    }

    // load address, 24bit
    for(i = 2;i >= 0;i--){
        ret = spiByteExch(device, (addr >> (8 * i)) & 0xFF, NULL);
        if(ret != Stat830xSuccess){
            // try to disable flash mux back to mmc access
            intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_CS)<<16);
            intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_MUX_EN)<<16);
            return ret;
        }
    }

    // read data out, select blockmode
    ret = intRegWrite(device, SIS830X_SPI_FLASH_CSR, 1<<SPI_FLASH_RD_BLK_EN);
    if(ret != Stat830xSuccess){
        // try to disable flash mux back to mmc access
        intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_CS)<<16);
        intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_MUX_EN)<<16);
        return ret;
    }
    usleep(1);

    // readout fifo
    uint32_t reg;
    for(i = 0;i < len;i++){
        ret = intRegRead(device, SIS830X_SPI_FLASH_CSR, &reg);
        if(ret != Stat830xSuccess){
            // try to disable flash mux back to mmc access
            intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_RD_BLK_EN)<<16);
            intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_CS)<<16);
            intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_MUX_EN)<<16);
            return ret;
        }

        *(data+i) = (uint8_t)reg;
        // advance fifo
        ret = intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_RD_BLK_FIFO));
        if(ret != Stat830xSuccess){
            // try to disable flash mux back to mmc access
            intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_RD_BLK_EN)<<16);
            intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_CS)<<16);
            intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_MUX_EN)<<16);
            return ret;
        }
    }

    // read data out, deselect blockmode
    ret = intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_RD_BLK_EN)<<16);
    if(ret != Stat830xSuccess){
        // try to disable flash mux back to mmc access
        intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_CS)<<16);
        intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_MUX_EN)<<16);
        return ret;
    }

    // deselect flash and return mux to mmc access
    ret = intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_CS)<<16);
    ret = intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_MUX_EN)<<16);
    if(ret != Stat830xSuccess){
        return ret;
    }

    return Stat830xSuccess;
}

SIS830X_STATUS spiFlashEraseBlock(PSIS830X_DEVICE device, uint32_t addr)
{
    SIS830X_STATUS ret;
    int i;

    ret = spiFlashWriteEnable(device);
    if(ret != Stat830xSuccess){
        return ret;
    }

    // gain control of spi lines
    ret = intRegWrite(device, SIS830X_SPI_FLASH_CSR, 1<<SPI_FLASH_MUX_EN);
    if(ret != Stat830xSuccess){
        return ret;
    }

    // select flash
    ret = intRegWrite(device, SIS830X_SPI_FLASH_CSR, 1<<SPI_FLASH_CS);
    if(ret != Stat830xSuccess){
        // try to disable flash mux back to mmc access
        intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_MUX_EN)<<16);
        return ret;
    }

    // load command, BLOCK ERASE 64kB = 0xD8
    ret = spiByteExch(device, 0xD8, NULL);
    if(ret != Stat830xSuccess){
        // try to disable flash mux back to mmc access
        intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_CS)<<16);
        intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_MUX_EN)<<16);
        return ret;
    }

    // load address, 24bit
    for(i = 2;i >= 0;i--){
        ret = spiByteExch(device, (addr >> (8 * i)) & 0xFF, NULL);
        if(ret != Stat830xSuccess){
            // try to disable flash mux back to mmc access
            intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_CS)<<16);
            intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_MUX_EN)<<16);
            return ret;
        }
    }

    // deselect flash and return mux to mmc access
    ret = intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_CS)<<16);
    ret = intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_MUX_EN)<<16);
    if(ret != Stat830xSuccess){
        return ret;
    }

    // poll busy, for a maximum of 2 seconds
    // datasheet: blockerase max: 1000ms
    ret = spiFlashPollBusy(device, 2000);
    if(ret != Stat830xSuccess){
        return ret;
    }

    return Stat830xSuccess;
}

SIS830X_STATUS spiFlashEraseChip(PSIS830X_DEVICE device)
{
    SIS830X_STATUS ret;

    ret = spiFlashWriteEnable(device);
    if(ret != Stat830xSuccess){
        return ret;
    }

    // gain control of spi lines
    ret = intRegWrite(device, SIS830X_SPI_FLASH_CSR, 1<<SPI_FLASH_MUX_EN);
    if(ret != Stat830xSuccess){
        return ret;
    }

    // select flash
    ret = intRegWrite(device, SIS830X_SPI_FLASH_CSR, 1<<SPI_FLASH_CS);
    if(ret != Stat830xSuccess){
        // try to disable flash mux back to mmc access
        intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_MUX_EN)<<16);
        return ret;
    }

    // load command, CHIP ERASE = 0xC7
    ret = spiByteExch(device, 0xC7, NULL);
    if(ret != Stat830xSuccess){
        // try to disable flash mux back to mmc access
        intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_CS)<<16);
        intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_MUX_EN)<<16);
        return ret;
    }

    // deselect flash and return mux to mmc access
    ret = intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_CS)<<16);
    ret = intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_MUX_EN)<<16);
    if(ret != Stat830xSuccess){
        return ret;
    }

    // poll busy, for a maximum of 45 seconds
    // datasheet: chiperase max: 30s
    ret = spiFlashPollBusy(device, 45000);
    if(ret != Stat830xSuccess){
        return ret;
    }

    return Stat830xSuccess;
}

SIS830X_STATUS spiFlashProgramPage(PSIS830X_DEVICE device, uint32_t addr, uint8_t *data, uint32_t len)
{
    SIS830X_STATUS ret;
    int i;

    ret = spiFlashWriteEnable(device);
    if(ret != Stat830xSuccess){
        return ret;
    }

    // gain control of spi lines
    ret = intRegWrite(device, SIS830X_SPI_FLASH_CSR, 1<<SPI_FLASH_MUX_EN);
    if(ret != Stat830xSuccess){
        return ret;
    }

    // select flash
    ret = intRegWrite(device, SIS830X_SPI_FLASH_CSR, 1<<SPI_FLASH_CS);
    if(ret != Stat830xSuccess){
        // try to disable flash mux back to mmc access
        intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_MUX_EN)<<16);
        return ret;
    }

    // load command, PROGRAM PAGE = 0x02
    ret = spiByteExch(device, 0x02, NULL);
    if(ret != Stat830xSuccess){
        // try to disable flash mux back to mmc access
        intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_CS)<<16);
        intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_MUX_EN)<<16);
        return ret;
    }

    // load address, 24bit
    for(i = 2;i >= 0;i--){
        ret = spiByteExch(device, (addr >> (8 * i)) & 0xFF, NULL);
        if(ret != Stat830xSuccess){
            // try to disable flash mux back to mmc access
            intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_CS)<<16);
            intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_MUX_EN)<<16);
            return ret;
        }
    }

    // enable block mode
    ret = intRegWrite(device, SIS830X_SPI_FLASH_CSR, 1<<SPI_FLASH_WR_BLK_EN);
    if(ret != Stat830xSuccess){
        // try to disable flash mux back to mmc access
        intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_CS)<<16);
        intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_MUX_EN)<<16);
        return ret;
    }


    // shift data in
    for(i = 0;i < len;i++){
        ret = intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_WR_BLK_FILL) | *(data+i));
        if(ret != Stat830xSuccess){
            // try to disable flash mux back to mmc access
            intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_WR_BLK_EN)<<16);
            intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_CS)<<16);
            intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_MUX_EN)<<16);
            return ret;
        }
    }

    // poll fsm busy
    uint32_t reg;
    do{
        ret = intRegRead(device, SIS830X_SPI_FLASH_CSR, &reg);
        if(ret != Stat830xSuccess){
            // try to disable flash mux back to mmc access
            intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_WR_BLK_EN)<<16);
            intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_CS)<<16);
            intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_MUX_EN)<<16);
            return ret;
        }
    }while(reg & (1<<SPI_FLASH_BUSY));

    // disable block mode
    ret = intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_WR_BLK_EN)<<16);
    if(ret != Stat830xSuccess){
        // try to disable flash mux back to mmc access
        intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_CS)<<16);
        intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_MUX_EN)<<16);
        return ret;
    }

    // deselect flash and return mux to mmc access
    ret = intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_CS)<<16);
    ret = intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_MUX_EN)<<16);
    if(ret != Stat830xSuccess){
        return ret;
    }

    // poll busy, for a maximum of 5 ms
    // datasheet: page program max: 3ms
    ret = spiFlashPollBusy(device, 5);
    if(ret != Stat830xSuccess){
        return ret;
    }

    return Stat830xSuccess;
}

SIS830X_STATUS spiFlashUpdateFirmware(PSIS830X_DEVICE device, char *path, void (*cb)(int percent))
{
    SIS830X_STATUS ret;
    int percent = 0, percentOld = 0;

    // read image into buffer
    FILE *fp = fopen(path, "rb");
    if(!fp){
        return Stat830xInvalidPath;
    }

    if(strstr(path,".bin") == NULL){
        fclose(fp);
        return Stat830xNotSupported;
    }

    fseek(fp, 0, SEEK_END);
    int fileSize = ftell(fp);
    rewind(fp);

    if(fileSize > SIS8300L_FLASHSIZE){
        fclose(fp);
        return Stat830xFileTooLarge;
    }

    uint8_t *image = (uint8_t *)malloc(fileSize * sizeof(uint8_t));
    if(image == NULL){
        fclose(fp);
        return Stat830xMemAlloc;
    }

    fread(image, sizeof(uint8_t), fileSize, fp);
    fclose(fp);

    // invoke callback
    if(cb)
        cb(percent);

    // erase blocks, write pages
    uint8_t *verify = (uint8_t *)malloc(SIS8300L_PAGESIZE * sizeof(uint8_t));
    if(verify == NULL){
        free(image);
        return Stat830xMemAlloc;
    }

    uint32_t written = 0;
    uint32_t pageProgramSize = 0;
    while(written < fileSize){
        // erase current block, 64kB
        if((written & (SIS8300L_BLOCKSIZE - 1)) == 0){
            ret = spiFlashEraseBlock(device, written);
            if(ret != Stat830xSuccess){
                return ret;
            }
        }

        // program page, 256B
        if(fileSize >= (written + SIS8300L_PAGESIZE)){
            pageProgramSize = SIS8300L_PAGESIZE;
        }else{
            pageProgramSize = fileSize - written;
        }

        ret = spiFlashProgramPage(device, written, image + written, pageProgramSize);
        if(ret != Stat830xSuccess){
            return ret;
        }

        // verify page
        ret = spiFlashReadBlock(device, written, verify, pageProgramSize);
        if(ret != Stat830xSuccess){
            return ret;
        }

        if(memcmp(image + written, verify, pageProgramSize)){
            int i;
            printf("soll\n");
            for(i = 0;i < 256;i++){
                if((i&7)==0) printf("\n[%04X]\t", written + i);

                printf("%02X ", *(image + written + i));
            }

            printf("ist\n");
            for(i = 0;i < 256;i++){
                if((i&7)==0) printf("\n[%04X]\t", written + i);

                printf("%02X ", *(verify + i));
            }


            free(image);
            free(verify);
            return Stat830xVerifyError;
        }

        written += SIS8300L_PAGESIZE;

        if(cb){
            percent = written * 100 / fileSize;
            if(percent != percentOld){
                (cb)(percent);
                percentOld = percent;
            }
        }
    }

    free(image);
    free(verify);

    return Stat830xSuccess;
}

SIS830X_STATUS spiFlashWriteEnable(PSIS830X_DEVICE device)
{
    SIS830X_STATUS ret;

    // gain control of spi lines
    ret = intRegWrite(device, SIS830X_SPI_FLASH_CSR, 1<<SPI_FLASH_MUX_EN);
    if(ret != Stat830xSuccess){
        return ret;
    }

    // select flash
    ret = intRegWrite(device, SIS830X_SPI_FLASH_CSR, 1<<SPI_FLASH_CS);
    if(ret != Stat830xSuccess){
        // try to disable flash mux back to mmc access
        intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_MUX_EN)<<16);
        return ret;
    }

    // load command, WRITE ENABLE = 0x06
    ret = spiByteExch(device, 0x06, NULL);
    if(ret != Stat830xSuccess){
        // try to disable flash mux back to mmc access
        intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_CS)<<16);
        intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_MUX_EN)<<16);
        return ret;
    }

    // deselect flash and return mux to mmc access
    ret = intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_CS)<<16);
    ret = intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_MUX_EN)<<16);
    if(ret != Stat830xSuccess){
        return ret;
    }

    return Stat830xSuccess;
}

SIS830X_STATUS spiFlashPollBusy(PSIS830X_DEVICE device, int timeout)
{
    SIS830X_STATUS ret;
    bool busy = true;
    bool wait100ms = timeout > 100 ? true : false;
    int maxWaitLoops = timeout;

    if(wait100ms){
        maxWaitLoops = (timeout + 50) / 100; // roundup to nearest 100ms
    }

    while((busy == true) && (maxWaitLoops > 0)){
        ret = spiFlashReadBusy(device, &busy);
        if(ret != Stat830xSuccess){
            return ret;
        }

        if(busy){
            if(wait100ms){
                usleep(100000); // 100 msec
            }else{
                usleep(1000); // 1 msec
            }

            maxWaitLoops--;
        }

    }
    if(maxWaitLoops == 0){
        return Stat830xFlashBusy;
    }

    return Stat830xSuccess;
}

SIS830X_STATUS spiFlashReadBusy(PSIS830X_DEVICE device, bool *busy)
{
    SIS830X_STATUS ret;

    // gain control of spi lines
    ret = intRegWrite(device, SIS830X_SPI_FLASH_CSR, 1<<SPI_FLASH_MUX_EN);
    if(ret != Stat830xSuccess){
        return ret;
    }

    // select flash
    ret = intRegWrite(device, SIS830X_SPI_FLASH_CSR, 1<<SPI_FLASH_CS);
    if(ret != Stat830xSuccess){
        // try to disable flash mux back to mmc access
        intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_MUX_EN)<<16);
        return ret;
    }

    // load command, READ STATUS REGISTER-1 = 0x05
    ret = spiByteExch(device, 0x05, NULL);
    if(ret != Stat830xSuccess){
        // try to disable flash mux back to mmc access
        intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_CS)<<16);
        intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_MUX_EN)<<16);
        return ret;
    }

    // exchange byte
    uint8_t reg;
    ret = spiByteExch(device, 0, &reg);
    if(ret != Stat830xSuccess){
        // try to disable flash mux back to mmc access
        intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_CS)<<16);
        intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_MUX_EN)<<16);
        return ret;
    }

    *busy = false;
    if(reg & 1){
        *busy = true;
    }

    // deselect flash and return mux to mmc access
    ret = intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_CS)<<16);
    ret = intRegWrite(device, SIS830X_SPI_FLASH_CSR, (1<<SPI_FLASH_MUX_EN)<<16);
    if(ret != Stat830xSuccess){
        return ret;
    }

    return Stat830xSuccess;
}
