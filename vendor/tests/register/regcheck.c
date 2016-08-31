#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <linux/fs.h>
#include <stdint.h>
#include "../../driver/sis8300_defs.h"


int sis8300_ReadRegister(int device, unsigned int address, unsigned int* data){

    sis8300_reg myReg;
    myReg.offset = address;
    ioctl(device, SIS8300_REG_READ, &myReg);
    *data = myReg.data ;
    return 0;
}

int sis8300_WriteRegister(int device, unsigned int address, unsigned int data){

    sis8300_reg myReg;
    // enable ddr2 test write interface
    myReg.offset = address;
    myReg.data = data;
    ioctl(device, SIS8300_REG_WRITE, &myReg);
    return 0;
}

int sis8300_CheckRegister(int device, unsigned int address, unsigned int data){

    unsigned int temp;

  //  sis8300_WriteRegister(device, address, data);
    sis8300_ReadRegister(device, address, &temp);

    if(data != temp)
        return 1;
    return 0;
}

int main(int argc, char **argv){

	int fp,i, elements;

	uint32_t test_data;

    unsigned int addr_data_array[][2] = {
        {0x100,0x00},
        {0x101,0x01},
        {0x102,0x02},
        {0x103,0x03},
        {0x104,0x04},
        {0x105,0x05},
        {0x106,0x06},
        {0x107,0x07},
        {0x108,0x08},
        {0x109,0x09},
        {0x110,0x00},
        {0x111,0x01},
        {0x112,0x02},
        {0x113,0x03},
        {0x114,0x04},
        {0x115,0x05},
        {0x116,0x06},
        {0x117,0x07},
        {0x118,0x08},
        {0x119,0x09},
    };

    elements = sizeof(addr_data_array)/(sizeof(unsigned int)*2);
	if(argc != 2){
		printf("usage: %s [device]\n", argv[0],argc);
		return -1;
	}

	fp = open(argv[1], O_RDWR);
	if(fp < 0){
		printf("can't open device\n");
		return -1;
	}

    for(i=0; i<elements; i++){
        sis8300_WriteRegister(fp, addr_data_array[i][0], addr_data_array[i][1]);
    }
    for(i=0; i<elements; i++){
        if(sis8300_CheckRegister(fp,addr_data_array[i][0],addr_data_array[i][1]))
            printf("> %d\n", i);
    }

	close(fp);

	return 0;
}
