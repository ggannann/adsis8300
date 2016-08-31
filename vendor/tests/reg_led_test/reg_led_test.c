#include "reg_led_test.h"

uint16_t reg_led_test(uint16_t i, PSIS830X_DEVICE dev){
    uint32_t data;

        printf("\n* testing modul %d:\n",i+1);

            sis830x_WriteRegister(dev,0x04,0x00000002);
            sis830x_ReadRegister(dev,0x00,&data);
            printf("Firmware:\t0x%08X\n",data);
            sis830x_ReadRegister(dev,0x01,&data);
            printf("Serialnumber:\t0x%08X\n\nCheck User LED: ",data);

            for(i=0; i < 8;i++){
                sis830x_WriteRegister(dev,0x04,0x00000001);
                printf("on ");fflush(stdout);
                usleep(500000);
                sis830x_WriteRegister(dev,0x04,0x00010000);
                printf("off ");fflush(stdout);
                usleep(500000);
            }

        sis830x_WriteRegister(dev,0x04,0x00020000);

        printf("... done\n");


    return EXIT_SUCCESS;
}
