#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "../../lib/libSIS830x/sis830x.h"
#include "register_map_in56.h"

PSIS830X_DEVICE dev_list;


uint8_t *th_com;        //[!Stop|Wait|15|14|13|12|OPT2|OPT1]
void* th_echo(void*);
void th_echo_f(PSIS830X_DEVICE,uint8_t,uint8_t);
void dev_init(PSIS830X_DEVICE,uint8_t,uint8_t);
void dev_in_fifo_reset(PSIS830X_DEVICE,uint8_t,uint8_t);
uint8_t dev_test(PSIS830X_DEVICE,uint8_t,uint8_t);

int main()
{
    int num, i=0;
    uint32_t data;
    SIS830X_STATUS stat;

    pthread_t *th_id;

    SIS830X_STATUS_TEXT stat_text;

    uint8_t OPT_flag = 0, error=0;


/*** OPEN ***/
    printf("\n------------------------   Testing Links   ------------------------\n");

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
    th_id= (pthread_t*)malloc(num*sizeof(pthread_t));
    th_com= (uint8_t*)malloc(num*sizeof(uint8_t));

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
        }
    }

    for(i=0; i < num; i++){
        if(dev_list[i].open && (dev_list[i].type == 2)){        // only if device is a SIS8300L
            printf("* testing default setup for DEV_%d: crystal based clock input... ",i);

            stat = sis830x_si5338a_configDefault(&dev_list[i]);
            if(stat != Stat830xSuccess){
                sis830x_status2str(stat,stat_text);
                printf("failed: %s\n", stat_text);
                sis830x_CloseDevice(&dev_list[i]);
                dev_list[i].open = false;
            }
            else{
                printf("done\n");
            }
        }
    }


//    printf("\n# creating threads...\n");

    for(i=0; i < num; i++){
        if(dev_list[i].open){
//             printf("# creating thread for DEV_%d... ",i);
            if(pthread_create(&th_id[i],NULL,th_echo,(void*)&i)!=0){
//                printf("failed\n");
            }
            else{
                th_com[i]=0x80;
//                printf("done\n");
                usleep(100);
            }
        }
    }


    printf("\n* testing link status...\n");
    for(i=0; i < num; i++){
        if(dev_list[i].open){
             while(th_com[i]&0x80){usleep(100);}             // Auf Threads warten!
            printf("links up DEV_%d: %02X\n",i,th_com[i]);
        }
    }

    // TODO: Was wenn kein Link up!?!

//  Begin der Tests
    printf("\n* starting communication tests...\n");
    for(i=0; i < num; i++){
        if(dev_list[i].open){
            // Thread des Devices pausieren
            th_com[i]|=0x40;

            // Fuer jeden offenen Link das Protokoll initialisieren
            if(th_com[i]&0x01){ // OPT1
       //         dev_in_fifo_reset(&dev_list[i],0x15,0x00);
                dev_init(&dev_list[i],0x15,0x00);
            }
            if(th_com[i]&0x02){ // OPT2
       //         dev_in_fifo_reset(&dev_list[i],0x17,0x00);
                dev_init(&dev_list[i],0x17,0x00);
            }
            if(th_com[i]&0x04){ // PORT12
        //        dev_in_fifo_reset(&dev_list[i],0x15,0x02);
                dev_init(&dev_list[i],0x15,0x02);
            }
            if(th_com[i]&0x08){ // PORT13
        //        dev_in_fifo_reset(&dev_list[i],0x17,0x02);
                dev_init(&dev_list[i],0x17,0x02);
            }
            if(th_com[i]&0x10){ // PORT14
         //       dev_in_fifo_reset(&dev_list[i],0x15,0x03);
                dev_init(&dev_list[i],0x15,0x03);
            }
            if(th_com[i]&0x20){ // PORT15
          //      dev_in_fifo_reset(&dev_list[i],0x17,0x03);
                dev_init(&dev_list[i],0x17,0x03);
            }

            // Thread wieder frei geben
            th_com[i]&=~0x40;
//           printf("# INIT finished DEV_%d\n",i);
        }
    }

    for(i=0; i < num; i++){
        if(dev_list[i].open){
            // Thread des Devices pausieren
            th_com[i]|=0x40;

            // Fuer jeden offenen Link den IN_FIFO loeschen
            if(th_com[i]&0x01){ // OPT1
                dev_in_fifo_reset(&dev_list[i],0x15,0x00);
            }
            if(th_com[i]&0x02){ // OPT2
                dev_in_fifo_reset(&dev_list[i],0x17,0x00);
            }
            if(th_com[i]&0x04){ // PORT12
                dev_in_fifo_reset(&dev_list[i],0x15,0x02);
            }
            if(th_com[i]&0x08){ // PORT13
                dev_in_fifo_reset(&dev_list[i],0x17,0x02);
            }
            if(th_com[i]&0x10){ // PORT14
                dev_in_fifo_reset(&dev_list[i],0x15,0x03);
            }
            if(th_com[i]&0x20){ // PORT15
                dev_in_fifo_reset(&dev_list[i],0x17,0x03);
            }

            // Thread wieder frei geben
            th_com[i]&=~0x40;
        }
    }


    usleep(500);

    for(i=0; i < num; i++){
        if(dev_list[i].open){
            // Thread des Devices pausieren
            th_com[i]|=0x40;

            printf("Dev_%d:\n",i);

            // Fuer jeden offenen Link einen Transfertest
            if(th_com[i]&0x01){ // OPT1

                if(th_com[i]&0x02){ // Direktverbindung OPT1 u. OPT2?
                    sis830x_WriteRegister(&dev_list[i],0x15,0x00000000);
                    sis830x_WriteRegister(&dev_list[i],0x17,0x80000000);
                    sis830x_WriteRegister(&dev_list[i],0x17,0x00000000);
                    sis830x_WriteRegister(&dev_list[i],0x14,0x0BADBEEF);
                    usleep(1000);
                    sis830x_WriteRegister(&dev_list[i],0x17,0x40000000);
                    sis830x_ReadRegister(&dev_list[i],0x16,&data);
                    if(data==0x0BADBEEF){error=dev_test(&dev_list[i],0x15,0x40);OPT_flag=0x40;}
                    else                {error=dev_test(&dev_list[i],0x15,0x00);OPT_flag=0x00;}
                }
                else{error=dev_test(&dev_list[i],0x15,0x00);}
                if(error){printf("OPT1[%2d], ",error);error=0;}
                else{printf("OPT1 [OK], ");}
            }else{printf("OPT1 [NA], ");}

            if(th_com[i]&0x02){ // OPT2
                if(OPT_flag){error=dev_test(&dev_list[i],0x17,0x80);OPT_flag=0x00;}
                else        {error=dev_test(&dev_list[i],0x17,0x00);}
                if(error){printf("OPT2 [%2d]\n",error);error=0;}
                else{printf("OPT2 [OK]\n");}
            }else{printf("OPT2 [NA]\n");}

            if(th_com[i]&0x04){ // PORT12
                error=dev_test(&dev_list[i],0x15,0x02);
                if(error){printf("PORT12 [%2d], ",error);error=0;}
                else{printf("PORT12 [OK], ");}
            }else{printf("PORT12 [NA], ");}

            if(th_com[i]&0x08){ // PORT13
                error=dev_test(&dev_list[i],0x17,0x02);
                if(error){printf("PORT13 [%2d], ",error);error=0;}
                else{printf("PORT13 [OK], ");}
            }else{printf("PORT13 [NA], ");}

            if(th_com[i]&0x10){ // PORT14
                error=dev_test(&dev_list[i],0x15,0x03);
                if(error){printf("PORT14 [%2d], ",error);error=0;}
                else{printf("PORT14 [OK], ");}
            }else{printf("PORT14 [NA], ");}

            if(th_com[i]&0x20){ // PORT15
                error=dev_test(&dev_list[i],0x17,0x03);
                if(error){printf("PORT15 [%2d]\n\n",error);error=0;}
                else{printf("PORT15 [OK]\n\n");}
            }else{printf("PORT15 [NA]\n\n");}

            // Thread wieder frei geben
            th_com[i]&=~0x40;
        }
    }


//printf("\n* closing devices...\n");
    for(i=0; i < num; i++){
        if(dev_list[i].open){
 //           printf("  waiting for DEV_%d... ",i);
            th_com[i]|=0x80;
            pthread_join(th_id[i],NULL);
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
    free(th_id);
    free(th_com);
    return 0;
}

/***  Threads  ***/

void* th_echo(void *attr){
int num = *((int*)attr);
//    printf("[ %d %x ]",num, th_com[num]);
uint32_t data_in=0;

    // Reset GTX OPTn und POERTn
    sis830x_WriteRegister(&dev_list[num],0x15,0x00000000);
    sis830x_WriteRegister(&dev_list[num],0x15,0x00000008);
    usleep(1000);
    sis830x_WriteRegister(&dev_list[num],0x15,0x00000000);
    sis830x_WriteRegister(&dev_list[num],0x17,0x00000002);
    sis830x_WriteRegister(&dev_list[num],0x17,0x0000000A);
    usleep(1000);
    sis830x_WriteRegister(&dev_list[num],0x17,0x00000002);
    //    printf("[ %d go ]",num);
    sleep(1);

    sis830x_ReadRegister(&dev_list[num], 0x15, &data_in);
    th_com[num] |= (data_in&0x02000000)>>25 | (data_in&0x04000000)>>22 | (data_in&0x08000000)>>25;
    sis830x_ReadRegister(&dev_list[num], 0x17, &data_in);
    th_com[num] |= (data_in&0x02000000)>>24 | (data_in&0x04000000)>>21 | (data_in&0x08000000)>>24;

 //   printf("[ %d %02X ]",num, th_com[num]);

    th_com[num]&=~0x80;

    while(!(th_com[num]&0x80)){     // Solange Thread_Stop = 0; wird durch Vater gesetzt
        if(!(th_com[num]&0x40)){    // Thread_break = 0; wird durch Vater gesetzt
            if(th_com[num]&0x01){   // Wenn OPT1 up
                th_echo_f(&dev_list[num],0x15,0x00);
            }
            if(th_com[num]&0x02){   // Wenn OPT2 up
                th_echo_f(&dev_list[num],0x17,0x00);
            }
            if(th_com[num]&0x04){   // Wenn PORT12 up
                th_echo_f(&dev_list[num],0x15,0x02);
            }
            if(th_com[num]&0x08){   // Wenn PORT13 up
                th_echo_f(&dev_list[num],0x17,0x02);
            }
            if(th_com[num]&0x10){   // Wenn PORT14 up
                th_echo_f(&dev_list[num],0x15,0x03);
            }
            if(th_com[num]&0x20){   // Wenn PORT15 up
                th_echo_f(&dev_list[num],0x17,0x03);
            }
        }
        usleep(10);
    }

 //    printf("[ %d ]",num);
    return NULL;
}

void th_echo_f(PSIS830X_DEVICE dev,uint8_t reg,uint8_t index){
uint32_t data=0;
uint16_t i;
                sis830x_WriteRegister(dev,reg,0x00000000|index);  // Link selektieren
                sis830x_ReadRegister(dev, reg, &data);
                i=(data&0x000001FF);
                if(i){                                 // Neue Daten empfangen
                    if((data&0x001FF000)==0x00001000){
                   //     printf(".");  // DEBUG
                        sis830x_WriteRegister(dev,reg-1, 0xBAD0BEEF);
                    }
                    for(;i>0;i--){
                        sis830x_WriteRegister(dev,reg,0x40000000|index);  // Readoption
                        sis830x_ReadRegister(dev, reg-1,&data);   // Daten lesen
                        sis830x_WriteRegister(dev,reg-1, data);
                    }
                }
}

void dev_in_fifo_reset(PSIS830X_DEVICE dev,uint8_t reg,uint8_t index){
    sis830x_WriteRegister(dev,reg,  0x80000000|index);
    sis830x_WriteRegister(dev,reg,  0x00000000|index);
}
void dev_init(PSIS830X_DEVICE dev,uint8_t reg,uint8_t index){
    sis830x_WriteRegister(dev,reg,  0x01000000|index);
    sis830x_WriteRegister(dev,reg-1,0x1C000000);
    sis830x_WriteRegister(dev,reg,  0x00000000|index);
}

uint8_t dev_test(PSIS830X_DEVICE dev,uint8_t reg,uint8_t index_flag){
    uint8_t index = index_flag&0x03;
    uint8_t flag = index_flag&0xC0;
    uint8_t RX_reg, tmp_return = 0;
    uint16_t i = 0;
    uint32_t data;
    uint32_t test_data[8]={
            0x00000000,
            0xFFFFFFFF,
            0x00000000,
            0xFFFFFFFF,
            0x55555555,
            0xAAAAAAAA,
            0x55555555,
            0xAAAAAAAA
    };

        switch(flag){
            case 0x40: RX_reg = 0x17; break;
            case 0x80: RX_reg = 0x15; break;
            default:   RX_reg = reg;
        }
        usleep(10);                         // Warten bis alle alten Daten empfangen
        dev_in_fifo_reset(dev,reg,index);

        for(i=1;i<9;i++){
            sis830x_WriteRegister(dev,reg-1,test_data[i-1]);

            do{
              sis830x_ReadRegister(dev,RX_reg,&data);
            }while(!(data&0x000001FF));

            sis830x_WriteRegister(dev,RX_reg,0x40000000|index);
            sis830x_ReadRegister(dev,RX_reg-1,&data);
            if(data != test_data[i-1]){
    //          printf("DB1: %X = %X\n",data,test_data[i-1]);
                tmp_return|=0x04;   // pattern error
            }
        }

        dev_in_fifo_reset(dev,reg,index);

        if(!tmp_return){    // Wenn bis hierhin noch keie Fehler erkannt, weiter mit Xoff-Test

        tmp_return=0x02;

            for(i=0;i<270;i++){
                sis830x_WriteRegister(dev,reg-1,(i<<16 | i));

                if(flag){
                    sis830x_ReadRegister(dev, reg, &data);
                    if((data&0x001FF000)==0x00001000){sis830x_WriteRegister(dev,reg-1, 0xBAD0BEEF);}
                }else{
                    usleep(10);
                }
            }

            for(i=0;i<270;i++){
                sis830x_WriteRegister(dev,RX_reg,0x40000000|index);
                usleep(10);
                sis830x_ReadRegister(dev,RX_reg-1,&data);
                if(data==0xBAD0BEEF){tmp_return &= ~0x02;i--;}
                else{if(data!=(i<<16 | i)){tmp_return |= 0x01;}}
            }
        }

    dev_in_fifo_reset(dev,reg,index);
    return tmp_return;
}
