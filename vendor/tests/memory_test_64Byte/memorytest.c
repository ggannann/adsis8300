/***************************************************************************/
/*                                                                         */
/*  Filename: memorytest.c                                                 */
/*                                                                         */
/*  Hardware: SIS8300_V2                                                   */
/*  Firmware: V_1400                                                       */
/*                                                                         */
/*  Funktion: SIS8300 Memory Test                                          */
/*                                                                         */
/*  Autor:                TH                                               */
/*  date:                 10.01.2012                                       */
/*  last modification:    12.01.2012                                       */
/*                                                                         */
/* ----------------------------------------------------------------------- */
/*                                                                         */
/*  SIS  Struck Innovative Systeme GmbH                                    */
/*                                                                         */
/*  Harksheider Str. 102A                                                  */
/*  22399 Hamburg                                                          */
/*                                                                         */
/*  Tel. +49 (0)40 60 87 305 0                                             */
/*  Fax  +49 (0)40 60 87 305 20                                            */
/*                                                                         */
/*  http://www.struck.de                                                   */
/*                                                                         */
/*  � 2012                                                                 */
/*                                                                         */
/***************************************************************************/
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <linux/fs.h>
#include "../../driver/sis8300_defs.h"
#include "../../driver/sis8300_reg.h"

#define MEMORY_ON_SIS8300_512MBYTE 0x20000000 // 512MB
#define MEMORY_ON_SIS8300_1GBYTE 0x40000000 // 1GB
#define MEMORY_ON_SIS8300L_2GBYTE 0x80000000 // 2GB
#define MEMORYTEST_CHUNK 0x2000000 // 32MB

//#define AMOUNT_OF_CHUNKS (MEMORY_ON_SIS8300 / MEMORYTEST_CHUNK)


int writeIncrementPattern(int fp, uint32_t *data,  unsigned int  chunk_len, unsigned int nof_chunks);
int readVerifyIncrementPattern(int fp, uint32_t *data, uint32_t *dataVfy,  unsigned int  chunk_len, unsigned int nof_chunks);

void buildTestData(uint32_t *, uint32_t *, int, int, int);
int writeReadVerify(int, uint32_t *, uint32_t *, int, int);
void errToFile(FILE *, int, uint32_t *);

int main(int argc, char **argv){

  int fp, i, i_chunk, error, loops, end_loops, chunk_no, pattern;
  uint32_t *data, *dataVfy;
  sis8300_reg myReg;
  FILE *errLog;

  uint32_t SX50_1GByte_Flag;
  uint32_t version_register;
  uint32_t option_register;

  uint32_t uint_MEMORYTEST_CHUNK;
  uint32_t uint_amount_of_chunks;

//  char *patternStrings[5] = {"increment       ", "walking 1s      ", "walking 0s      ", "alternating AA55", "random          "};
  char *patternStrings[4] = {"walking 1s      ", "walking 0s      ", "alternating AA55", "random          "};

      printf("\n------------------------   Memory Test   -------------------------------------\n");

  if(argc != 3){
    printf("usage: %s [device] [loops]\n", argv[0]);
    return -1;
  }

  fp = open(argv[1], O_RDWR);
  if(fp < 0){
    printf("can't open device\n");
    return -1;
  }



  printf("\n");
  myReg.offset = SIS8300_IDENTIFIER_VERSION_REG;
  ioctl(fp, SIS8300_REG_READ, &myReg);
  version_register = myReg.data ;
  printf("SIS8300_IDENTIFIER_VERSION_REG  = 0x%08X\n", version_register);

  myReg.offset = SIS8300_FIRMWARE_OPTIONS_REG;
  ioctl(fp, SIS8300_REG_READ, &myReg);
  option_register = myReg.data ;
  printf("SIS8300_FIRMWARE_OPTIONS_REG    = 0x%08X\n", option_register);

  myReg.offset = SIS8300_SERIAL_NUMBER_REG;
  ioctl(fp, SIS8300_REG_READ, &myReg);
  printf("SIS8300_SERIAL_NUMBER_REG       = 0x%08X\n", myReg.data);
  printf("\n");

  if ((version_register >> 16) == 0x8300) {
	// SIS8300 device

	SX50_1GByte_Flag = 0 ;
	if( ((version_register & 0xFF00) == 0x1400) && ((option_register & 0x100) == 0x100) ) {
	SX50_1GByte_Flag = 1 ;
	}

	//#define AMOUNT_OF_CHUNKS (MEMORY_ON_SIS8300 / MEMORYTEST_CHUNK)

	if( SX50_1GByte_Flag == 0 ) {
	printf("Testing 512MByte DDR2 memory...\n");
	uint_MEMORYTEST_CHUNK = MEMORYTEST_CHUNK ;
	uint_amount_of_chunks = MEMORY_ON_SIS8300_512MBYTE / uint_MEMORYTEST_CHUNK ;

	}
	else {
	printf("Testing 1GByte DDR2 memory...\n");
	uint_MEMORYTEST_CHUNK = MEMORYTEST_CHUNK ;
	uint_amount_of_chunks = MEMORY_ON_SIS8300_1GBYTE / uint_MEMORYTEST_CHUNK ;
	}
  }
  if ((version_register >> 16) == 0x8301) {
	// SIS8300-L device
	printf("Testing 2GByte DDR3 memory...\n");
	uint_MEMORYTEST_CHUNK = MEMORYTEST_CHUNK ;
	uint_amount_of_chunks = MEMORY_ON_SIS8300L_2GBYTE / uint_MEMORYTEST_CHUNK;
  }

  printf("\n");
  printf("uint_amount_of_chunks    = 0x%08X\n", uint_amount_of_chunks);
  printf("\n");

// stop test-histogramming (in case it was enabled)
  myReg.offset = SIS8300_TEST_HISTO_CONTROL;
  myReg.data = 0x0;
  ioctl(fp, SIS8300_REG_WRITE, &myReg);

  // allocating memoryblock
  data = (uint32_t *)malloc(uint_MEMORYTEST_CHUNK);
  if(data == NULL){
    printf("Error allocating data-buffer !\n");
    return -1;
  }
  // allocating memoryblock for verifying
  dataVfy = (uint32_t *)malloc(uint_MEMORYTEST_CHUNK);
  if(dataVfy == NULL){
    printf("Error allocating data-buffer for verifying!\n");
    return -1;
  }

  // enable ddr2 test write interface
  myReg.offset = DDR2_ACCESS_CONTROL;
  myReg.data = (1<<DDR2_PCIE_TEST_ENABLE);
  ioctl(fp, SIS8300_REG_WRITE, &myReg);

  // allow 2 outstanding pcie requests
  myReg.offset = 0x215;
  myReg.data = 2;
  ioctl(fp, SIS8300_REG_WRITE, &myReg);

  error = 0;
  loops = 0;
  end_loops = strtol(argv[2],NULL,16);
  chunk_no = 0;
  pattern = 0;


  while((error == 0) && (loops != end_loops)){

      printf("pattern: increment        ");
      writeIncrementPattern(fp, data, uint_MEMORYTEST_CHUNK,  uint_amount_of_chunks);
      if (readVerifyIncrementPattern(fp, data, dataVfy, uint_MEMORYTEST_CHUNK,  uint_amount_of_chunks)){
	  printf("verify error. IncrementPattern\n");
	  error = 1;
	  break;
      }else{
	  printf("  -----   successful  ------\n");
      }


      for(pattern = 0;pattern < 4;pattern++){

	    printf("pattern: %s ", patternStrings[pattern]);
	    for(i_chunk = 0;i_chunk < uint_amount_of_chunks;i_chunk++){
		//printf("pattern    = %d   i_chunk = %d \n", pattern, i_chunk);
		buildTestData(data, dataVfy, uint_MEMORYTEST_CHUNK, uint_MEMORYTEST_CHUNK * i_chunk, pattern);

		if (writeReadVerify(fp, data, dataVfy, uint_MEMORYTEST_CHUNK, uint_MEMORYTEST_CHUNK * i_chunk)){
		  printf("verify error. pattern:%s page:%d\n", patternStrings[pattern], i_chunk);
		  fflush(stdout);
		  error = 1;
		  break;
		}else{
		  printf("wrv");
		  fflush(stdout);
		}


	    }
	  printf("  -----   successful  ------\n");
      }
      loops++;
      printf("\n---------------   %d/%d successful loop(s).  ---------------\n\n", loops, end_loops);

  }

  close(fp);
  free(data);
  free(dataVfy);

  return 0;
}



// writes incremt data blocks
int writeIncrementPattern(int fp, uint32_t *data,  unsigned int  chunk_len, unsigned int nof_chunks){

  int i_chunck, i, index, write_len_byte,rest_len_byte;
  uint32_t pattern;
  sis8300_reg myReg;

    unsigned char *char_data_ptr;

    char_data_ptr = (unsigned char*)data;
#if 0
    printf("chunk_len: %d\n", chunk_len);
#endif
  for (i_chunck=0;i_chunck<nof_chunks;i_chunck++) {
      pattern = (chunk_len*i_chunck) / 4;
      // len is in bytes
      for(i = 0;i < (chunk_len / 4);i++){
	*(data + i) = pattern;
	pattern = pattern + 1 ;
      }
      myReg.offset = 0x212;
      myReg.data = chunk_len*i_chunck;
      ioctl(fp, SIS8300_REG_WRITE, &myReg);

      printf("w");
      fflush(stdout);

//***********//
      rest_len_byte = chunk_len;
      index = 0;
      do{
        if(rest_len_byte > 64){
            write_len_byte = 64;
        }
        else{
            write_len_byte = rest_len_byte;
        }

        write(fp, &char_data_ptr[index], write_len_byte);

        rest_len_byte -= write_len_byte;
        index += write_len_byte;
#if 0
        printf("%d | %d\n", write_len_byte, rest_len_byte);
        fflush(stdout);
#endif
      }while(rest_len_byte != 0);
//***********//
  }
  return 0;
}



// writes, reads and verifies data blocks
int readVerifyIncrementPattern(int fp, uint32_t *data, uint32_t *dataVfy,  unsigned int  chunk_len, unsigned int nof_chunks){
  uint32_t pattern;
  int i, i_chunck, errCnt, k;
  sis8300_reg myReg;

  errCnt = 0;

  for (i_chunck=0;i_chunck<nof_chunks;i_chunck++) {
      pattern = (chunk_len*i_chunck) / 4;
      // len is in bytes
      for(i = 0;i < (chunk_len / 4);i++){
	*(dataVfy + i) = pattern;
	pattern = pattern + 1 ;
      }

      memset(data, 0, chunk_len);

      myReg.offset = 0x202;
      myReg.data = chunk_len*i_chunck;
      ioctl(fp, SIS8300_REG_WRITE, &myReg);

      printf("r");
      fflush(stdout);
      read(fp, data, chunk_len);
      //printf("read ok.%p\n", dataVfy);

      printf("v");
      fflush(stdout);
      if(memcmp(data, dataVfy, chunk_len) ){
	printf("   adr  read        expected\n");
	for(i = 0;i < (chunk_len / 4);i++){
	  if(*(data + i) != *(dataVfy + i)){
	    printf("*0x%X:%08X != %08X\n", i * 4, *(data + i), *(dataVfy + i));
	    if(errCnt++ == 15){
	      printf("more errors skipped...\n");
	      return -1;
	    }
	  }
	}
	return -1;
      }


  }


  return 0;
}




// fills both memoryblock with selected amount and type of testdata
// types:
// 0: walking 1's
// 1: walking 0's
// 2: alternating (0xAA55AA55)
// 3: random
void buildTestData(uint32_t *data, uint32_t *dataVfy, int len, int offset, int type){

  int i;
  uint32_t pattern;

  if (len == 0)
    return;

  switch(type){

    case 1:
      pattern = 0x00000001;

      // len is in bytes
      for(i = 0;i < (len / 4);i++){
	*(data + i) = pattern;
	*(dataVfy + i) = pattern;
	pattern = pattern << 1;
	if (pattern == 0)
	  pattern = 0x00000001;
      }

      break;

    case 2:
      pattern = 0xFFFFFFFE;

      // len is in bytes
      for(i = 0;i < (len / 4);i++){
	*(data + i) = pattern;
	*(dataVfy + i) = pattern;
	pattern = (pattern << 1) + 1;
	if (pattern == 0xFFFFFFFF)
	  pattern = 0xFFFFFFFE;
      }

      break;
    case 3:
      pattern = 0xAA55AA55;

      // len is in bytes
      for(i = 0;i < (len / 4);i++){
	*(data + i) = pattern;
	*(dataVfy + i) = pattern;
	pattern = ~pattern;
      }

      break;
    case 4:
      pattern = rand() << 16 | rand();

      // len is in bytes
      for(i = 0;i < (len / 4);i++){
	*(data + i) = pattern;
	*(dataVfy + i) = pattern;
	pattern = rand() << 16 | rand();
      }
      break;
  }
}

// writes, reads and verifies data blocks
int writeReadVerify(int fp, uint32_t *data, uint32_t *dataVfy, int len, int offset){

  int i, errCnt, k, index, write_len_byte,rest_len_byte;;
  sis8300_reg myReg;

  errCnt = 0;

  unsigned char *char_data_ptr;
  char_data_ptr = (unsigned char*)data;

  myReg.offset = 0x212;
  myReg.data = offset;
  ioctl(fp, SIS8300_REG_WRITE, &myReg);

//***********//
      rest_len_byte = len;
      index = 0;
      do{
        if(rest_len_byte > 64){
            write_len_byte = 64;
        }
        else{
            write_len_byte = rest_len_byte;
        }

        write(fp, &char_data_ptr[index], write_len_byte);

        rest_len_byte -= write_len_byte;
        index += write_len_byte;
#if 0
        printf("%d | %d\n", write_len_byte, rest_len_byte);
        fflush(stdout);
#endif
      }while(rest_len_byte != 0);
//***********//

  //printf("write ok.%p\n", data);

  memset(data, 0, len);

  myReg.offset = 0x202;
  myReg.data = offset;
  ioctl(fp, SIS8300_REG_WRITE, &myReg);

  read(fp, data, len);
  //printf("read ok.%p\n", dataVfy);

  if(memcmp(data, dataVfy, len) ){
    printf("   adr  read        expected\n");
    for(i = 0;i < (len / 4);i++){
      if(*(data + i) != *(dataVfy + i)){
	printf("*0x%X:%08X != %08X\n", i * 4, *(data + i), *(dataVfy + i));
	if(errCnt++ == 15){
	  printf("more errors skipped...\n");
	  return -1;
	}
      }
    }

    return -1;
  }
  return 0;
}

void errToFile(FILE *errLog, int chunk_no, uint32_t *data){
}
