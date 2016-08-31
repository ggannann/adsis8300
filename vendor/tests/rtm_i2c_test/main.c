#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sis830x.h"
#include "register_map_in56.h"

#define PCA9535ADDR     0x20
#define LTC2493ADDR     0x34

//#define TEST_LTC2493
#ifdef TEST_LTC2493
//#define TEMP_ON
//#define PWR
#define TEMP
#endif // TEST_LTC2493

//#define TEST_PCA9553
#ifdef TEST_PCA9553

#endif // TEST_PCA9553

//#define TEST_ATT
#ifdef TEST_ATT

#define ATT_LE0     0
#define ATT_LE1     1
#define ATT_LE2     2
#define ATT_LE3     3
#define ATT_LE4     4
#define ATT_LE5     5
#define ATT_LE6     6
#define ATT_LE7     7
#define ATT_LE8     8
#define ATT_LE9     9

#endif // TEST_ATT

#define TEST_AD5624
#ifdef TEST_AD5624
#define INITCMDAC 0x352
#endif // TEST_AD5624

PSIS830X_DEVICE dev_list;

SIS830X_STATUS setAttenuator(uint8_t addr, uint8_t data);
SIS830X_STATUS initCommonModeDAC(uint16_t initValue);
void convert_data(uint32_t *data);
float temp(uint32_t *data);
float pwr_vol(uint32_t *data);
float temp_ext(uint32_t *data);

int main()
{
    int num, i=0;

//    uint8_t bus_addr, reg_addr;
    SIS830X_STATUS stat;

    float temp_rtm;

    uint8_t PCA9535_addr = PCA9535ADDR;
    uint8_t PCA9535_reg;
    uint8_t PCA9535_data;

    uint8_t LTC2493_addr = LTC2493ADDR;
    uint16_t LTC2493_writeData;
    uint32_t LTC2493_readData, readData;

    struct truthTableElement{
        uint8_t value;
        char discript[10];
    };

    struct truthTableElement truthTable[]={
        {0x3F, "0dB"},
        {0x3E, "-0,5dB"},
        {0x3D, "-1dB"},
        {0x3B, "-2dB"},
        {0x37, "-4dB"},
        {0x2F, "-8dB"},
        {0x1F, "-16dB"},
        {0x00, "-31,5dB"}
    };

    char errorstr[20];

 // SIS830X_STATUS_TEXT stat_text;



/*** OPEN ***/
    printf("\n------------------------   Testing RTM I²C   ------------------------\n");

    stat = sis830x_GetNumberOfDevices(&num);
    if(stat != Stat830xSuccess){
        printf("get number of devices failed.\n");
        return EXIT_FAILURE;
    }

 //   printf("devs: %d\n", num);
    if(num == 0){
        printf("no cards installed.\n");
        return EXIT_FAILURE;
    }

    dev_list = (PSIS830X_DEVICE)malloc(num*sizeof(SIS830X_DEVICE));

    if(dev_list == NULL){
        printf("open virtual RAM failed\n");
        return EXIT_FAILURE;
    }

    for(i=0; i < num; i++){
        dev_list[i].open = false;

        stat = sis830x_OpenDeviceOnIdx(i, &dev_list[i]);
 //       printf("open DEV_%d... ",i);
        if(stat != Stat830xSuccess){
            printf("open DEV_%d failed (%d)\n",i,stat);
        }
        else{
  //          printf("done\n");
        dev_list[i].type = 2;
        }
    }

    sis830x_ReadRegister(&dev_list[0], 0x0, &readData);
    if((readData&0xF000) != 0x2000){
        printf("Wrong FW version: 0x%X (expected: 0x2xxx)\n",readData&0xFFFF);
        sis830x_CloseDevice(&dev_list[i]);
        return -1;
    }

#ifdef TEMP_ON
    LTC2493_writeData = 0xC0B0;
#endif // TEMP_ON
#ifdef PWR
    LTC2493_writeData = 0x80B0;
#endif // PWR
#ifdef TEMP
    LTC2493_writeData = 0x80B8;
#endif // TEMP

#ifdef TEST_LTC2493
    stat = sis830x_rtm_ltc2493_Write(&dev_list[0], LTC2493_addr, &LTC2493_writeData);
    if(stat != Stat830xSuccess){
        sis830x_status2str(stat, errorstr);
        printf("Write I²C failed (%d): %s\n",stat, errorstr);
    }
    sleep(1);
#endif // TEST_LTC2493

#ifdef TEST_PCA9553
    // Set Pin IO0_0 - IO0-7 and IO1_0 - IO1_3 to output
    PCA9535_reg = 6;
    PCA9535_data = 0x00;
    stat = sis830x_rtm_pca9535_Write(&dev_list[0], PCA9535_addr, PCA9535_reg, PCA9535_data);
    if(stat != Stat830xSuccess){
        sis830x_status2str(stat, errorstr);
        printf("Write I²C failed (%d): %s\n",stat, errorstr);
    }

    PCA9535_reg = 7;
    PCA9535_data = 0x00;
    stat = sis830x_rtm_pca9535_Write(&dev_list[0], PCA9535_addr, PCA9535_reg, PCA9535_data);
    if(stat != Stat830xSuccess){
        sis830x_status2str(stat, errorstr);
        printf("Write I²C failed (%d): %s\n",stat, errorstr);
    }

    PCA9535_data = 0xAA;
#endif // TEST_PCA9553

#ifdef TEST_AD5624
    stat = initCommonModeDAC(INITCMDAC);
    if(stat != Stat830xSuccess){
        sis830x_status2str(stat, errorstr);
        printf("Init CommonModeDAC failed (%d): %s\n",stat, errorstr);
    }
#endif // TEST_AD5624

i=0;
do{

#ifdef TEST_LTC2493
    stat = sis830x_rtm_ltc2493_Read(&dev_list[0], LTC2493_addr, &LTC2493_readData);
    if(stat != Stat830xSuccess){
        sis830x_status2str(stat, errorstr);
        printf("Read I²C failed (%d): %s\n",stat, errorstr);
    }
    else{
        convert_data(&LTC2493_readData);

        #ifdef TEMP_ON
         printf("Read I²C-Bus: Device: LTC, Data: 0x%X -> %f C\n", LTC2493_readData, temp(&LTC2493_readData));
        #endif // TEMP_ON
        #ifdef PWR
        printf("Read I²C-Bus: Device: LO-PWR, Data: 0x%X -> %f V\n", LTC2493_readData, pwr_vol(&LTC2493_readData));
        #endif // PWR
        #ifdef TEMP
        printf("Read I²C-Bus: Device: ADC, Data: 0x%X -> %f C\n", LTC2493_readData, temp_ext(&LTC2493_readData));
        #endif // TEMP

    }
#endif // TEST_LTC2493

#ifdef TEST_PCA9553
    // Set Pin IO0_0 - IO0-7 and IO1_0 - IO1_3
    PCA9535_reg = 2;
    PCA9535_data ^= 0xFF;
    stat = sis830x_rtm_pca9535_Write(&dev_list[0], PCA9535_addr, PCA9535_reg, PCA9535_data);
    if(stat != Stat830xSuccess){
        sis830x_status2str(stat, errorstr);
        printf("Write I²C failed (%d): %s\n",stat, errorstr);
    }
    else{
        printf("Set REG: %d to 0x%02X\n", PCA9535_reg, PCA9535_data);
    }

    PCA9535_reg = 3;
//    PCA9535_data = 0x0F;
    stat = sis830x_rtm_pca9535_Write(&dev_list[0], PCA9535_addr, PCA9535_reg, PCA9535_data);
    if(stat != Stat830xSuccess){
        sis830x_status2str(stat, errorstr);
        printf("Write I²C failed (%d): %s\n",stat, errorstr);
    }
    else
    {
        printf("Set REG: %d to 0x%02X\n", PCA9535_reg, PCA9535_data);
    }
#endif // TEST_PCA9553

#ifdef TEST_ATT
    stat = setAttenuator(ATT_LE0, truthTable[i].value);
    if(stat != Stat830xSuccess){
        sis830x_status2str(stat, errorstr);
        printf("'setAttenuator' failed (%d): %s\n",stat, errorstr);
    }
    else{
        printf("Set Attenuator %d to %s\n", ATT_LE0, truthTable[i].discript);
    }
    i++;
#endif // TEST_ATT

}
while(getchar() != 'q');
    usleep(500);



//printf("\n* closing devices...\n");
    for(i=0; i < num; i++){
        if(dev_list[i].open){
            stat = sis830x_CloseDevice(&dev_list[i]);
            if(stat != Stat830xSuccess){
                printf("closing failed\n");
            }
            else{
 //               printf("closed\n");
            }
        }
    }
printf("-- test finished --\n");
    free(dev_list);
    return 0;
}



void convert_data(uint32_t *data)
{

    *data = ((*data & 0xFF)<<24) | ((*data & 0xFF00)<<8) | ((*data & 0xFF0000)>>8) | ((*data & 0xFF000000)>>24);

}

float temp(uint32_t *data)
{

    uint32_t tmp;

    tmp = *data & ~0x80000000;
    tmp>>=7;

    return ((tmp * 3.3)/1570) - 273.15;
}

float pwr_vol(uint32_t *data)
{

    uint32_t tmp;

    tmp = *data & ~0x80000000;
    tmp>>=6;

    return tmp*(3.3/0x1000000); // (Vref/2^25Bit)*2
}

float temp_ext(uint32_t *data)
{

    uint32_t tmp;

    tmp = *data & ~0x80000000;
    tmp>>=6;


    return ((tmp*(3.3/0x1000000))-1.275)/0.005; // {(Vref/2^25Bit)*2}-1,275 / 0.005
}

// Set the attenuator
// @param addr attenuator number 1 - 8
// @param data
SIS830X_STATUS setAttenuator(uint8_t addr, uint8_t data)
{
    SIS830X_STATUS stat;

    static uint8_t initPCA9535 = false;
    uint8_t i, mask = 0x20, data_tmp=0;

    if(!initPCA9535){

        // Set PORT0 to output
        stat = sis830x_rtm_pca9535_Write(&dev_list[0], PCA9535ADDR, 6, 0x00);
        if(stat != Stat830xSuccess) return stat;

        // Set PORT1 to output
        stat = sis830x_rtm_pca9535_Write(&dev_list[0], PCA9535ADDR, 7, 0x00);
        if(stat != Stat830xSuccess) return stat;

        initPCA9535 = true;
    }

    stat = sis830x_rtm_pca9535_Write(&dev_list[0], PCA9535ADDR, 2, data_tmp);
    if(stat != Stat830xSuccess) return stat;

    stat = sis830x_rtm_pca9535_Write(&dev_list[0], PCA9535ADDR, 3, 0x8);       // VCM_DAC_CS to high!
    if(stat != Stat830xSuccess) return stat;

    for(i=0; i<6; i++){
        if(mask & data){
            data_tmp |= 0x02;
        }
        else{
            data_tmp &= ~0x02;
        }

        stat = sis830x_rtm_pca9535_Write(&dev_list[0], PCA9535ADDR, 2, data_tmp);
        if(stat != Stat830xSuccess) return stat;
        mask>>=1;

        stat = sis830x_rtm_pca9535_Write(&dev_list[0], PCA9535ADDR, 2, data_tmp | 1);
        stat = sis830x_rtm_pca9535_Write(&dev_list[0], PCA9535ADDR, 2, data_tmp);
        if(stat != Stat830xSuccess) return stat;
    }

    stat = sis830x_rtm_pca9535_Write(&dev_list[0], PCA9535ADDR, 2, 0);

    if(addr < 6){
        stat = sis830x_rtm_pca9535_Write(&dev_list[0], PCA9535ADDR, 2, 1<<(addr+2));
        stat = sis830x_rtm_pca9535_Write(&dev_list[0], PCA9535ADDR, 2, 0);
    }
    else{
        stat = sis830x_rtm_pca9535_Write(&dev_list[0], PCA9535ADDR, 3, 0x8 | 1<<(addr-6));  // VCM_DAC_CS const to high!
        stat = sis830x_rtm_pca9535_Write(&dev_list[0], PCA9535ADDR, 3, 0x8);
    }

    return stat;
}


// Init common mode DAC
// @param data init data
SIS830X_STATUS initCommonModeDAC(uint16_t initValue)
{
    SIS830X_STATUS stat;

    static uint8_t initPCA9535 = false;
    uint32_t mask = 0x800000, send_data=0;
    uint8_t i, data_tmp=0;

    if(!initPCA9535){

        // Set PORT0 to output
        stat = sis830x_rtm_pca9535_Write(&dev_list[0], PCA9535ADDR, 6, 0x00);
        if(stat != Stat830xSuccess) return stat;

        // Set PORT1 to output
        stat = sis830x_rtm_pca9535_Write(&dev_list[0], PCA9535ADDR, 7, 0x00);
        if(stat != Stat830xSuccess) return stat;

        initPCA9535 = true;
    }

    send_data = (initValue << 4);

    // Command Definition :
    // C2 - C0:     3h     (Write to and update DAC)
    // Address Command:
    // A2 - A0:     7h     (All DACs)

    // CmdAddr [x x C2 C1 C0 A2 A1 A0]:     0x1F
    send_data |= (0x1F << 16);

    stat = sis830x_rtm_pca9535_Write(&dev_list[0], PCA9535ADDR, 2, data_tmp);
    if(stat != Stat830xSuccess) return stat;

    stat = sis830x_rtm_pca9535_Write(&dev_list[0], PCA9535ADDR, 3, 0x8);       // VCM_DAC_CS to high!
    stat = sis830x_rtm_pca9535_Write(&dev_list[0], PCA9535ADDR, 3, 0x0);       // VCM_DAC_CS to low!

    for(i=0; i<24; i++){
        if(mask & send_data){
            data_tmp |= 0x02;
        }
        else{
            data_tmp &= ~0x02;
        }

        stat = sis830x_rtm_pca9535_Write(&dev_list[0], PCA9535ADDR, 2, data_tmp);
        if(stat != Stat830xSuccess) return stat;
        mask>>=1;

        stat = sis830x_rtm_pca9535_Write(&dev_list[0], PCA9535ADDR, 2, data_tmp | 1);
        stat = sis830x_rtm_pca9535_Write(&dev_list[0], PCA9535ADDR, 2, data_tmp);
        if(stat != Stat830xSuccess) return stat;
    }

    stat = sis830x_rtm_pca9535_Write(&dev_list[0], PCA9535ADDR, 2, 0);

    stat = sis830x_rtm_pca9535_Write(&dev_list[0], PCA9535ADDR, 3, 0x8);       // VCM_DAC_CS to high!

    return stat;
}
