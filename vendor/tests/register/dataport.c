#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <linux/fs.h>
#include <stdint.h>
#include "../../driver/sis8300_defs.h"

int main(int argc, char **argv){

  int fp;
  int module;
  sis8300_reg myReg;
  int result;
  char string[1];
   
if(argc != 2){
	printf("usage: %s [device]\n", argv[0]);
  return -1;
  }
  
  myReg.offset = (unsigned int)strtol("0x000", NULL, 16);
	           

	fp = open(argv[1], O_RDWR);
	
  if(fp < 0){
		printf("can't open device\n");
		return -1;
	}
  ioctl(fp, SIS8300_REG_READ, &myReg);
	printf("Identifier: %X\n", myReg.data);
  
  if((myReg.data&0xFFFF0000)==0x83000000){
  
  printf("\n\nYou are testing with SIS8300V2\n\n"); 
  
  printf("\f");// system("clear")
	printf( "++++++++++++SIS8900 RTM DATA-PORT checker ++++++++++++ \n\n");
	printf( "\nConnect Port A and C with LVDS wire ");
	//result = scanf( "%x", &module );
	gets(string);
	
  myReg.offset = (unsigned int)strtol("0x12F", NULL, 16);
	myReg.data = (unsigned int)strtol("0x3F080000", NULL, 16);
	ioctl(fp, SIS8300_REG_WRITE, &myReg);
	sleep(1);
	ioctl(fp, SIS8300_REG_READ, &myReg);
	
	if((myReg.data&2)==2){
	  printf("Bit 1 = 1  ok    -  ");}
	  else
	    {
	    printf("ERROR setting Bit 1 ! ");
	  }
	  printf("reg = 0x%08X\n", myReg.data);
	
	myReg.data = (unsigned int)strtol("0x3F000000", NULL, 16);             
	ioctl(fp, SIS8300_REG_WRITE, &myReg);
	sleep(1);
	ioctl(fp, SIS8300_REG_READ, &myReg);
	
	if((myReg.data&2) == 0){
	  printf("Bit 1 = 0  ok    -  ");}
	  else
    	{
	   printf("ERROR clearing Bit 1 ! ");
	  }
	printf("reg = 0x%08X\n", myReg.data);
    	
// Checking Port A and D
	
	printf( "\n\nConnect Port A and D with LVDS wire ");
	//result = scanf( "%x", &module );
	gets(string);
	
	myReg.data = (unsigned int)strtol("0x3F080000", NULL, 16);             

	ioctl(fp, SIS8300_REG_WRITE, &myReg);
	sleep(1);
	ioctl(fp, SIS8300_REG_READ, &myReg);
	
	if((myReg.data&1)==1){
	  printf("Bit 0 = 1  ok    -  ");}
	  else
    	{
	   printf("ERROR setting Bit 0 ! ");
	  }	
        printf("reg = 0x%08X\n", myReg.data);                
  	
	myReg.data = (unsigned int)strtol("0x3F000000", NULL, 16);             
	ioctl(fp, SIS8300_REG_WRITE, &myReg);
	sleep(1);
	ioctl(fp, SIS8300_REG_READ, &myReg);
	
	if((myReg.data&1) == 0){
	  printf("Bit 0 = 0  ok    -  ");}
	  else
    	{
	   printf("ERROR clearing Bit 0 ! ");
	  }
	printf("reg = 0x%08X\n", myReg.data);
}
else{
printf("\n\nYou are testing with SIS8300L\n\n"); 
  
  printf("\f");
	printf( "++++++++++++SIS8900 RTM DATA-PORT checker ++++++++++++ \n\n");
	printf( "\n\nConnect Port D and A with LVDS wire ");
	//result = scanf( "%x", &module );
	gets(string);
	
	myReg.offset = (unsigned int)strtol("0x12F", NULL, 16);
  myReg.data = (unsigned int)strtol("0x07010000", NULL, 16);
	ioctl(fp, SIS8300_REG_WRITE, &myReg);
	sleep(1);
	ioctl(fp, SIS8300_REG_READ, &myReg);
	
	if((myReg.data&8)==8){
	  printf("Bit 3 = 1  ok    -  ");}
	  else
    	{
	    printf("ERROR setting Bit 1 ! ");}
	printf("reg = 0x%08X\n", myReg.data);
	
	
	myReg.data = (unsigned int)strtol("0x07000000", NULL, 16);             
	ioctl(fp, SIS8300_REG_WRITE, &myReg);
	sleep(1);
	ioctl(fp, SIS8300_REG_READ, &myReg);
	
	if((myReg.data&8) == 0){
	  printf("Bit 3 = 0  ok    -  ");}
	  else
    {
	   printf("ERROR clearing Bit 3 ! ");}
	printf("reg = 0x%08X\n", myReg.data);
	 
    	
// Checking Port C and A
	
	printf( "\n\nConnect Port C and A with LVDS wire ");
	//result = scanf( "%x", &module );
	gets(string);
	
	myReg.data = (unsigned int)strtol("0x07020000", NULL, 16);             

	ioctl(fp, SIS8300_REG_WRITE, &myReg);
	sleep(1);
	ioctl(fp, SIS8300_REG_READ, &myReg);
	
	if((myReg.data&8)==8){
	  printf("Bit 3 = 1  ok    -  ");}
	else
    	{
	printf("ERROR setting Bit 3 ! ");}
	printf("reg = 0x%08X\n", myReg.data);
	                        
  	
	myReg.data = (unsigned int)strtol("0x07000000", NULL, 16);             
	ioctl(fp, SIS8300_REG_WRITE, &myReg);
	sleep(1);
	ioctl(fp, SIS8300_REG_READ, &myReg);
	
	if((myReg.data&8) == 0){
	  printf("Bit 3 = 0  ok    -  ");}
	  else
	  {
	   printf("ERROR clearing Bit 3 ! ");}
	printf("reg = 0x%08X\n", myReg.data);
	  


}	 	
	close(fp);

	return 0;
}
