#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <linux/fs.h>
#include "../../driver/sis8300_defs.h"

int main(int argc, char **argv){

	unsigned int i;
	int fp;
	sis8300_reg myReg;

	if(argc != 3){
		printf("usage: %s [device] [offset]\n", argv[0]);
		return -1;
	}

	fp = open(argv[1], O_RDWR);
	if(fp < 0){
		printf("can't open device\n");
		return -1;
	}

	myReg.offset = (unsigned int)strtoul(argv[2], NULL, 16);

	ioctl(fp, SIS8300_REG_READ, &myReg);

	printf("reg = 0x%08X\n", myReg.data);
	
	close(fp);

	return 0;
}