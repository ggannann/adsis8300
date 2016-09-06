#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <linux/fs.h>
#include <stdint.h>
#include "../../driver/sis8300_defs.h"

int main(int argc, char **argv){

	int fp;
	sis8300_reg myReg;

	if(argc != 4){
		printf("usage: %s [device] [offset] [data]\n", argv[0]);
		return -1;
	}

	myReg.offset = (unsigned int)strtoul(argv[2], NULL, 16);
	myReg.data = (unsigned int)strtoul(argv[3], NULL, 16);

	fp = open(argv[1], O_RDWR);
	if(fp < 0){
		printf("can't open device\n");
		return -1;
	}

	ioctl(fp, SIS8300_REG_WRITE, &myReg);

	close(fp);

	return 0;
}
