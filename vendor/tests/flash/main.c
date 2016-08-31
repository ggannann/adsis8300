#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../lib/libSIS830x/sis830x.h"

void callback(int percent);

int main(int argc, char *argv[])
{
    int num;
    SIS830X_STATUS stat;
    SIS830X_STATUS_TEXT status_text;
    SIS830X_DEVICE dev = {.open = false};
    char *moduleTypes[] = {"UNKNOWN", "SIS8300", "SIS8300L"};

    if(argc != 2){
        printf("\nusage: %s [path to *.bin file]\n", argv[0]);
        return -1;
    }

    printf("Flash Update!\n");

    stat = sis830x_GetNumberOfDevices(&num);
    if(stat != Stat830xSuccess){
        printf("'sis830x_GetNumberOfDevices' failed.\n");
        return -1;
    }

    printf("devs: %d\n", num);
    if(num == 0){
        printf("no cards installed.\n");
        return -1;
    }

    int card;
    for(card = 0;card < num;card++){
        stat = sis830x_OpenDeviceOnIdx(card, &dev);
        if(stat != Stat830xSuccess){
            printf("'sis830x_OpenDevice' failed: %d\n", stat);
            return -1;
        }

        printf("card opened. type: '%s'\n", moduleTypes[dev.type]);

        if(dev.type != SIS8300L){
            printf("card not compatible with this update tool, skipping...\n");
            continue;
        }

        uint32_t ver;
        stat = sis830x_ReadRegister(&dev, SIS830X_MODULE_VERSION_REG, &ver);
        if(stat != Stat830xSuccess){
            printf("'sis830x_ReadRegister' failed: %d\n", stat);
            return -1;
        }
        ver &= 0xFFFF;

        printf("firmware version on module is: %04X\n", ver);
        char c;
        do{
            printf("udpate? [y/n]: ");
            fflush(stdout);
            fflush(stdin);
            c = getchar();
            fflush(stdin);
        }while(c != 'y' && c != 'n');
        if(c == 'n'){
            continue;
        }

        printf("attempting flash update, standby.....\n");

        printf("file: %s\n", argv[1]);

        stat = sis830x_spiFlashUpdateFirmware(&dev, argv[1], callback);
        printf("\n");
        if(stat != Stat830xSuccess){

            sis830x_status2str(stat, status_text);

            printf("Firmware update failed: %s\n", status_text);
            return -1;
        }

        printf("flashing done\n");

        stat = sis830x_CloseDevice(&dev);
        if(stat != Stat830xSuccess){
            printf("'sis830x_CloseDevice' failed.\n");
            return -1;
        }
    }

    printf("finished flashing all present/selected cards\n");

    return 0;
}

void callback(int percent){+
    printf("\rprogress: %d%%", percent);
    fflush(stdout);
}
