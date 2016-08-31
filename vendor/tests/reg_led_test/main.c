#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "reg_led_test.h"

//#include "../../lib/libSIS830x/sis830x.h"
#include "register_map_in56.h"


int main( int argc, char **argv )
{
    PSIS830X_DEVICE dev_list;

    SIS830X_STATUS stat;
    int num;

    uint8_t numofdef=0;

    uint16_t i;

    printf("\n------------------------   Register Read/Write Test   ------------------------\n");

    if( argc != 2 ){
      printf("usage: %s [num of nodule]\n", argv[0]);
      return -1;
    }

    numofdef = strtoul(argv[1],NULL,16);

    stat=sis830x_GetNumberOfDevices(&num);
    if(stat != Stat830xSuccess){
        printf("get number of modules failed\n");
        return EXIT_FAILURE;
    }

    if(num != numofdef){
        printf("number of modules not o.k., found: %d\n", num);
        return EXIT_FAILURE;
    }

 //   printf("devs: %d\n",num);
    if(num == 0){
        printf("no modules installed\n");
        return EXIT_FAILURE;
    }

    dev_list = (PSIS830X_DEVICE)malloc(num*sizeof(SIS830X_DEVICE));
    if(dev_list == NULL){
        printf("open virtual RAM failed\n");
        return EXIT_FAILURE;
    }

    for(i=0; i<num; i++){
        dev_list[i].open = false;

   //     printf("open DEV_%d ",i+1);

        stat = sis830x_OpenDeviceOnIdx(i,&dev_list[i]);
        if(stat != Stat830xSuccess){
            printf("open modul %d failed\n",i+1);
            return EXIT_FAILURE;
        }else{
 //           printf("done\n");
        }
    }


    for(i=0; i<num; i++){
        if(dev_list[i].open == true){
            reg_led_test(i, &dev_list[i]);
        }
    }


/*** CLOSING ***/

    for(i=0; i<num; i++){
        if(dev_list[i].open == true){
            stat = sis830x_CloseDevice(&dev_list[i]);
            if(stat != Stat830xSuccess){
                printf("close modul %d failed\n",i+1);
            }
        }
    }

    free(dev_list);
    printf("\n-- test finished --\n");
    return EXIT_SUCCESS;
}
