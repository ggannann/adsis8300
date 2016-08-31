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
  
  unsigned int i;
  int fp;
  sis8300_reg reg;
  unsigned int out;
  unsigned int expIn;
  unsigned int error;

  if( argc != 2 ){
    printf("usage: %s [device]\n", argv[0]);
    return -1;
  }

  fp = open( argv[1], O_RDWR );
  if( fp < 0 ){
    printf( "can't open device: '%s'\n", argv[1] );
    return -1;
  }

  printf( "\nTesting RTM LVDS I/O:\n" );
  
  reg.offset = SIS8300_RTM_LVDS_IO_CONTROL_REG;
  
  error = 0;
  
  printf( "Clear all databits test ... " );  
  
  // write-read-compare with data = 0
    
  out = 0x07000000;
  expIn = 0x07000000;
  reg.data = out;
  ioctl( fp, SIS8300_REG_WRITE, &reg );
  usleep( 1000 );
  ioctl( fp, SIS8300_REG_READ, &reg );

  if( reg.data != 0x07000000 ){
    printf( "\nError on compare! data = 0\n");
    printf( "written: %08X, readback: %08X, expected: %08X\n\n", out, reg.data, expIn );
    error ++;
  }

  if( error == 0 ){
  printf( "passed!\n" );              
  }

  printf( "Databits bitshift pattern test ... " );

  // bitshift 1 pattern databits
  for( i = 0;i < 3;i++ ){

    out = ( 0x07<<24 ) | ( 1<<( 16 + i ));
    expIn = ( 0x07<<24 ) | ( 1<<( 16 + i )) | ( 1<<( 3 + i ));
    
    // write-read
    reg.data = out;
    ioctl( fp, SIS8300_REG_WRITE, &reg );
    usleep( 1000 );
    ioctl( fp, SIS8300_REG_READ, &reg );

    // compare
    if( reg.data != expIn ){
      if( error == 0 ){
	printf( "\nError on compare! Databits bitshift pattern, loop: %d\n", i );
      }
      error ++;
      printf( "written : %08X, readback: %08X, expected: %08X\n", out, reg.data, expIn );
    }

    if( error > 9 ){
    printf( "\n" );              
    return -1;
    }
      
  }
  
  if( error != 0 ){
  printf( "\n" );              
  return -1;
  }
  else{
  printf( "passed!\n" );              
  }

  printf( "Databits increment pattern test ... " );

  //  error = 0;
 
  // increment pattern
  for( i = 0;i < 8;i++ ){
  
    out = ( 0x07<<24 ) | ( i<<16 );
    expIn = ( 0x07<<24 ) | ( i<<16 ) | ( i<<3 );
    
    // write-read-compare
    reg.data = out;
    ioctl( fp, SIS8300_REG_WRITE, &reg );
    usleep( 1000 );
    ioctl( fp, SIS8300_REG_READ, &reg );
    
    if( reg.data != expIn ){
      if( error == 0 ){
      printf( "\nError on compare! Databits increment pattern, loop: %d\n", i );
      }
      error ++;
      printf( "written: %08X, readback: %08X, expected: %08X\n", out, reg.data, expIn );
    }

    if( error > 9 ){
    printf( "\n" );              
    return -1;
    }
  
  }
  
  if( error != 0 ){
  printf( "\n" );              
  return -1;
  }
  else{
  printf( "passed!\n" );              
  }
  
  printf( "Enablebits increment pattern test ... " );

//  error = 0;  
  
  // reset output buffers
  reg.data = 0x07<<24;
  ioctl( fp, SIS8300_REG_WRITE, &reg );
  usleep( 1000 );
  reg.data = 0;
  ioctl( fp, SIS8300_REG_WRITE, &reg );
  usleep( 1000 );
  
  // pattern 2 enablebits
  for( i = 0;i < 3;i++ ){
    // normal pattern
    out = ( (1<<i)<<24 ) | ( 0x07<<16 );
    expIn = ( (1<<i)<<24 ) | ( 0x07<<16 ) | ( 1<<(i+3));
    
    // write-read-compare
    reg.data = out;
    ioctl( fp, SIS8300_REG_WRITE, &reg );
    usleep( 1000 );
    
    ioctl( fp, SIS8300_REG_READ, &reg );
    
    if( reg.data != expIn ){
      if( error == 0 ){      
      printf( "\nError on compare! Enablebits bitshift pattern, loop: %d\n", i );
      }
      error ++;
      printf( "written: %08X, readback: %08X, expected: %08X\n", out, reg.data, expIn );
    }

    if( error > 9 ){
    printf( "\n" );              
    return -1;
    }
    
    reg.data = ( (1<<i)<<24 ); //clear databit
    ioctl( fp, SIS8300_REG_WRITE, &reg );
    usleep( 1000 );
    reg.data = 0; // clear enable bits
    ioctl( fp, SIS8300_REG_WRITE, &reg );
    usleep( 1000 );

  }
  
  if( error != 0 ){
  printf( "\n" );              
  return -1;
  }
  else{
  printf( "passed!\n" );              
  }
  
  printf( "Buffer output hold pattern test ... " );
  
//  error = 0;  
   
  // reset output buffers
  reg.data = 0x07<<24;
  ioctl( fp, SIS8300_REG_WRITE, &reg );
  usleep( 1000 );
  reg.data = 0;
  ioctl( fp, SIS8300_REG_WRITE, &reg );
  usleep( 1000 );
  
  // pattern 2 enable bits and hold
  for( i = 0;i < 3;i++ ){
    // normal pattern
    out = ( (1<<i)<<24 ) | ( 0x07<<16 );
 
    // write
    reg.data = out;
    ioctl( fp, SIS8300_REG_WRITE, &reg );
    usleep( 1000 );
  
    // output hold pattern
    expIn = ( 0x07<<16 ) | ( 1<<(i+3));
    
    // write-read-compare
    reg.data = ( 0x07<<16 ); //disable
    ioctl( fp, SIS8300_REG_WRITE, &reg );
    usleep( 1000 );
    
    ioctl( fp, SIS8300_REG_READ, &reg );
    
    if( reg.data != expIn ){
      if( error == 0 ){      
      printf( "\nError on compare! Buffer output hold pattern test, loop: %d\n", i );
      }
      error ++;
      printf( "written first: %08X, written then: 0x003F0000,  readback: %08X, expected: %08X\n", out, reg.data, expIn );
    }
      
    if( error > 9 ){
    printf( "\n" );              
    return -1;
    }

    reg.data = ( (1<<i)<<24 ); //clear databit
    ioctl( fp, SIS8300_REG_WRITE, &reg );
    usleep( 1000 );
    reg.data = 0; // clear enable bits
    ioctl( fp, SIS8300_REG_WRITE, &reg );
    usleep( 1000 );
   
  }

  if( error != 0 ){
  printf( "\n" );        
  return -1;
  }
  else{
  printf( "passed!\n\n" );              
  }

  // switch off current enable bit
  reg.data = ( 0x07<<24 );
  ioctl( fp, SIS8300_REG_WRITE, &reg );
  usleep( 1000 );
  reg.data = 0;
  ioctl( fp, SIS8300_REG_WRITE, &reg );
  usleep( 1000 );

  return 0;  
  }