#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <linux/fs.h>
#include <sys/ioctl.h>
#include "../../driver/sis8300_defs.h"
#include "../../driver/sis8300_reg.h"

int main( int argc, char **argv ){

  unsigned int i, j, loops;
  int fp;
  sis8300_reg reg;
  unsigned int out;
  unsigned int expIn;

  if( argc != 3 ){
    printf("usage: %s [device] [loops]\n", argv[0]);
    return -1;
  }

  loops = strtoul(argv[2],NULL,16);

  fp = open( argv[1], O_RDWR );
  if( fp < 0 ){
    printf( "can't open device: '%s'\n", argv[1] );
    return -1;
  }
  
  printf("\n------------------------   Testing Harlink I/O [%s]   ------------------------\n",argv[1]);

  reg.offset = SIS8300_HARLINK_IN_OUT_CONTROL_REG;

  for( j = 0;j < loops;j++ ){
    
  for( i = 0;i < 16;i++ ){
    // normal pattern
    out = ( 1<<20 ) | ( i<<16 );
    expIn = ( 1<<20 ) | ( i<<16 ) | i;

    // write-read-compare
    reg.data = out;
    ioctl( fp, SIS8300_REG_WRITE, &reg );
    usleep( 1000 );

    ioctl( fp, SIS8300_REG_READ, &reg );

    if( reg.data != expIn ){
      printf( "Error on compare! normal pattern, loop: %d\n", i );
      printf( "written: %08X, readback: %08X, expected: %08X\n", out, reg.data, expIn );
      printf( "Failing bits:\n" );
      for( i = 0;i < 4;i++ ){
	if( (reg.data & 1<<i) != (expIn & 1<<i) ){
	  printf( "Harlink Bit %d\n", i );
	}
      }
      reg.data = 0;
      ioctl( fp, SIS8300_REG_WRITE, &reg );
      return -1;
    }

    // inverted pattern
    out = ( 1<<20 ) | ( i<<16 );
    out = ( out & 0xFFF0FFFF ) | (~out & 0x000F0000); // invert bits 16-19
    expIn = ( 1<<20 ) | ( i<<16 ) | i;
    expIn = ( expIn & 0xFFF0FFF0 ) | (~expIn & 0x000F0000) | (~expIn & 0x0000000F); // invert bits 16-19 and 0-3

    // write-read-compare
    reg.data = out;
    ioctl( fp, SIS8300_REG_WRITE, &reg );
    usleep( 1000 );

    ioctl( fp, SIS8300_REG_READ, &reg );

    if( reg.data != expIn ){
      printf( "Error on compare! inverted pattern, loop: %d\n", i );
      printf( "written: %08X, readback: %08X, expected: %08X\n", out, reg.data, expIn );
      printf( "Failing bits:\n" );
      for( i = 0;i < 4;i++ ){
	if( (reg.data & 1<<i) != (expIn & 1<<i) ){
	  printf( "Harlink Bit %d\n", i );
	}
      }
      reg.data = 0;
      ioctl( fp, SIS8300_REG_WRITE, &reg );
      return -1;
    }
  }
  
  }
  reg.data = 0;
  ioctl( fp, SIS8300_REG_WRITE, &reg );
  printf( "Test OK\n" );

  return 0;
}
