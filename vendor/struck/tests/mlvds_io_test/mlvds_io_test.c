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
  
  unsigned int i, j, loops, modules = 0;
  unsigned int l, del = 2; // del value for usleep in us
  int fp, fp2;
  sis8300_reg reg, reg2, reg3;
  unsigned int out;
  unsigned int expIn;

  if( argc < 4 ){
    printf( "usage: %s [number of devices] [device 1] [device 2] [loops]\n", argv[0] );
    return -1;
  }
  
  if( strcmp( argv[1], "1" ) == 0 ){
    modules = 1;
    if( argc < 4 ){
      printf( "too few arguments\n" );
      printf( "usage: %s [number of devices = 1] [device 1] [loops]\n", argv[0] );      
      return -1;
    }
     
  }
  if( strcmp( argv[1], "2" ) == 0 ){
    modules = 2;
    if( argc < 5 ){
      printf( "too few arguments\n" );
      printf( "usage: %s [number of devices = 2] [device 1] [device 2] [loops]\n", argv[0] );      
      return -1;
    }
  }
  
  if( modules == 0 ){
    printf( "usage: %s [number of devices] [device 1] [device 2] [loops]\n", argv[0] );
    return -1;
  }

  if( modules == 1 ){
   loops = strtoul(argv[3],NULL,16);
  }
  if( modules == 2 ){
   loops = strtoul(argv[4],NULL,16);
  }
 
  // always open 1 module
  fp = open( argv[2], O_RDWR );
  if( fp < 0 ){
    printf( "can't open device: '%s'\n", argv[2] );
    return -1;
  }
  
  // open 2nd module on request
  if( modules == 2 ){
      fp2 = open( argv[3], O_RDWR );
      if( fp2 < 0 ){
	printf( "can't open device: '%s'\n", argv[3] );
	return -1;
      }
  }


  
/*  reg3.offset = SIS8300_HARLINK_IN_OUT_CONTROL_REG;	// for scope trigger pulse
  reg3.data = 0x0;	 // set trigger signal to L (inactive)
  ioctl( fp, SIS8300_REG_WRITE, &reg3 ); // prepare HARLINK for trigger pulse generation
  reg3.data = 0x110000;	 // prepare data for trigger pulse enable and set to 1
*/

//  while(1){ // für Testzwecke eingefügt (AG)
  for( j = 0;j < loops;j++ ){

  if( modules == 1 ){
    printf( "Testing MLVDS I/O on single module...\n" );
    
    reg.offset = SIS8300_MLVDS_IO_CONTROL_REG;
    
    // pattern 1 databits
    for( i = 0;i < 256;i++ ){
      // normal pattern
      out = ( 0xFF<<24 ) | ( i<<16 );
      expIn = ( 0xFF<<24 ) | ( i<<16 ) | i;
      
      // write-read-compare
      reg.data = out;
      ioctl( fp, SIS8300_REG_WRITE, &reg );
//      asm volatile ("eieio");
      if(del){usleep( del );}
      
      ioctl( fp, SIS8300_REG_READ, &reg );
      
      if( reg.data != expIn ){
	printf( "Error on compare! normal pattern, loop: %d\n", i );
	printf( "written: %08X, readback: %08X, expected: %08X\n", out, reg.data, expIn );
	// 2. Ausgabe zur Fehlersuche AG
//	ioctl( fp, SIS8300_REG_READ, &reg );
//	printf( "written: %08X, readback: %08X, expected: %08X\n", out, reg.data, expIn );
	// Ende 2. Ausgabe zur Fehlersuche AG
	printf( "Failing bits:\n" );
	for( i = 0;i < 8;i++ ){
	  if( (reg.data & 1<<i) != (expIn & 1<<i) ){
	    printf( "MLVDS Databit %d\n", i );
	  }
	}

	// disable driver
	reg.data = 0;
	ioctl( fp, SIS8300_REG_WRITE, &reg );
	// disable leds
	reg.offset = SIS8300_USER_CONTROL_STATUS_REG;
	reg.data = 0x10000;
	ioctl( fp, SIS8300_REG_WRITE, &reg );
	
	return -1;
      }
      
      // inverted pattern
      out = ( 0xFF<<24 ) | ( i<<16 );
      out = ( out & 0xFF00FFFF ) | (~out & 0x00FF0000); // invert bits 16-23
      expIn = ( 0xFF<<24 ) | ( i<<16 ) | i;
      expIn = ( expIn & 0xFF00FF00 ) | (~expIn & 0x00FF0000)| (~expIn & 0x000000FF); // invert bits 16-23 and 0-7
      
      // write-read-compare
      reg.data = out;
      ioctl( fp, SIS8300_REG_WRITE, &reg );
      if(del){usleep( del );}
      
      ioctl( fp, SIS8300_REG_READ, &reg );
      
      if( reg.data != expIn ){
	printf( "Error on compare! inverted pattern, loop: %d\n", i );
	printf( "written: %08X, readback: %08X, expected: %08X\n", out, reg.data, expIn );
	// 2. Ausgabe zur Fehlersuche AG
//	ioctl( fp, SIS8300_REG_READ, &reg );
//	printf( "written: %08X, readback: %08X, expected: %08X\n", out, reg.data, expIn );
	// Ende 2. Ausgabe zur Fehlersuche AG
	printf( "Failing bits:\n" );
	for( i = 0;i < 8;i++ ){
	  if( (reg.data & 1<<i) != (expIn & 1<<i) ){
	    printf( "MLVDS Databit %d\n", i );
	  }
	}

	// disable driver
	reg.data = 0;
	ioctl( fp, SIS8300_REG_WRITE, &reg );
	// disable leds
	reg.offset = SIS8300_USER_CONTROL_STATUS_REG;
	reg.data = 0x10000;
	ioctl( fp, SIS8300_REG_WRITE, &reg );
	
	return -1;
      }
    }
    printf( "Databits OK\n" );
    
    // pattern 2 enablebits
    for( i = 0;i < 256;i++ ){
      // normal pattern
      out = ( i<<24 ) | ( 0xFF<<16 );
      expIn = ( i<<24 ) | ( 0xFF<<16 ) | i;
      
      // write-read-compare
      reg.data = out;
      ioctl( fp, SIS8300_REG_WRITE, &reg );
      if(del){usleep( del );}
      
      ioctl( fp, SIS8300_REG_READ, &reg );
      
      if( reg.data != expIn ){
	printf( "Error on compare! normal pattern, loop: %d\n", i );
	printf( "written: %08X, readback: %08X, expected: %08X\n", out, reg.data, expIn );
	// 2. Ausgabe zur Fehlersuche AG
//	ioctl( fp, SIS8300_REG_READ, &reg );
//	printf( "written: %08X, readback: %08X, expected: %08X\n", out, reg.data, expIn );
	// Ende 2. Ausgabe zur Fehlersuche AG
	printf( "Failing bits:\n" );
	for( i = 0;i < 8;i++ ){
	  if( (reg.data & 1<<i) != (expIn & 1<<i) ){
	    printf( "MLVDS Enablebit %d\n", i );
	  }
	}

	// disable driver
	reg.data = 0;
	ioctl( fp, SIS8300_REG_WRITE, &reg );
	// disable leds
	reg.offset = SIS8300_USER_CONTROL_STATUS_REG;
	reg.data = 0x10000;
	ioctl( fp, SIS8300_REG_WRITE, &reg );
		
	return -1;
      }
      
      // inverted pattern
      out = ( i<<24 ) | ( 0xFF<<16 );
      out = ( out & 0x00FFFFFF ) | (~out & 0xFF000000); // invert bits 24-31
      expIn = ( i<<24 ) | ( 0xFF<<16 ) | i;
      expIn = ( expIn & 0x00FFFF00 ) | (~expIn & 0xFF000000)| (~expIn & 0x000000FF); // invert bits 24-31 and 0-7
      
      // write-read-compare
      reg.data = out;
      ioctl( fp, SIS8300_REG_WRITE, &reg );
      if(del){usleep( del );}
      
      ioctl( fp, SIS8300_REG_READ, &reg );
      
      if( reg.data != expIn ){
	printf( "Error on compare! inverted pattern, loop: %d\n", i );
	printf( "written: %08X, readback: %08X, expected: %08X\n", out, reg.data, expIn );
	// 2. Ausgabe zur Fehlersuche AG
//	ioctl( fp, SIS8300_REG_READ, &reg );
//	printf( "written: %08X, readback: %08X, expected: %08X\n", out, reg.data, expIn );
	// Ende 2. Ausgabe zur Fehlersuche AG
	printf( "Failing bits:\n" );
	for( i = 0;i < 8;i++ ){
	  if( (reg.data & 1<<i) != (expIn & 1<<i) ){
	    printf( "MLVDS Enablebit %d\n", i );
	  }
	}
	
	// disable driver
	reg.data = 0;
	ioctl( fp, SIS8300_REG_WRITE, &reg );
	// disable leds
	reg.offset = SIS8300_USER_CONTROL_STATUS_REG;
	reg.data = 0x10000;
	ioctl( fp, SIS8300_REG_WRITE, &reg );
	
	return -1;
      }
    }
    printf( "Enablebits OK\n" );
    
    printf( "Test OK\n" );
  }else{
    // Direction Module 1 -> Module 2
    
    printf( "Testing MLVDS I/O from '%s'(user led off) to '%s'(user led on)...\n", argv[2], argv[3] );
    
    reg.offset = SIS8300_USER_CONTROL_STATUS_REG;
    reg.data = 0x10000;
    ioctl( fp, SIS8300_REG_WRITE, &reg );
    reg.data = 0x1;
    ioctl( fp2, SIS8300_REG_WRITE, &reg );
    
    reg.offset = SIS8300_MLVDS_IO_CONTROL_REG;
    reg2.offset = SIS8300_MLVDS_IO_CONTROL_REG;
    
    // set module 2 to receiver
    reg2.data = 0;
    ioctl( fp2, SIS8300_REG_WRITE, &reg2 );
//  usleep( 1000 );

    // pattern 1 databits
    for( i = 0;i < 256;i++ ){
      // normal pattern
      out = ( 0xFF<<24 ) | ( i<<16 );
      expIn = i;
      
      // write-read-compare
      reg.data = out;
      ioctl( fp, SIS8300_REG_WRITE, &reg );
      if(del){usleep( del );}
      
      ioctl( fp2, SIS8300_REG_READ, &reg2 );
      
      if( reg2.data != expIn ){
//	ioctl( fp, SIS8300_REG_WRITE, &reg3 ); // generate trigger pulse
	printf( "Error on compare! normal pattern, loop: %d\n", i );
	printf( "written: %08X, readback: %08X, expected: %08X\n", out, reg2.data, expIn );
	// 2. Ausgabe zur Fehlersuche AG
//	for( l = 0;l < 20;l++ ){
//	  ioctl( fp2, SIS8300_REG_READ, &reg2 );
//	  printf( "written: %08X, readback: %08X, expected: %08X\n", out, reg2.data, expIn );
//	}  
	// Ende 2. Ausgabe zur Fehlersuche AG
	printf( "Failing bits:\n" );
	for( i = 0;i < 8;i++ ){
	  if( (reg2.data & 1<<i) != (expIn & 1<<i) ){
	    printf( "MLVDS Databit %d\n", i );
	  }
	}
	// disable driver
	reg.data = 0;
	ioctl( fp, SIS8300_REG_WRITE, &reg );
	ioctl( fp2, SIS8300_REG_WRITE, &reg );
	// disable leds
	reg.offset = SIS8300_USER_CONTROL_STATUS_REG;
	reg.data = 0x10000;
	ioctl( fp, SIS8300_REG_WRITE, &reg );
	ioctl( fp2, SIS8300_REG_WRITE, &reg );
	return -1;
      }
      
      // inverted pattern
      out = ( 0xFF<<24 ) | ( i<<16 );
      out = ( out & 0xFF00FFFF ) | (~out & 0x00FF0000); // invert bits 16-23
      expIn = i;
      expIn = ( expIn & 0xFFFFFF00 ) | (~expIn & 0x000000FF); // invert bits 0-7
      
      // write-read-compare
      reg.data = out;
      ioctl( fp, SIS8300_REG_WRITE, &reg );
      if(del){usleep( del );}
      
      ioctl( fp2, SIS8300_REG_READ, &reg2 );
      
      if( reg2.data != expIn ){
//	ioctl( fp, SIS8300_REG_WRITE, &reg3 ); // generate trigger pulse
	printf( "Error on compare! inverted pattern, loop: %d\n", i );
	printf( "written: %08X, readback: %08X, expected: %08X\n", out, reg2.data, expIn );
	// 2. Ausgabe zur Fehlersuche AG
//	for( l = 0;l < 20;l++ ){
//	  ioctl( fp2, SIS8300_REG_READ, &reg2 );
//	  printf( "written: %08X, readback: %08X, expected: %08X\n", out, reg2.data, expIn );
//	}
	// Ende 2. Ausgabe zur Fehlersuche AG
	printf( "Failing bits:\n" );
	for( i = 0;i < 8;i++ ){
	  if( (reg2.data & 1<<i) != (expIn & 1<<i) ){
	    printf( "MLVDS Databit %d\n", i );
	  }
	}
	// disable driver
	reg.data = 0;
	ioctl( fp, SIS8300_REG_WRITE, &reg );
	ioctl( fp2, SIS8300_REG_WRITE, &reg );
	// disable leds
	reg.offset = SIS8300_USER_CONTROL_STATUS_REG;
	reg.data = 0x10000;
	ioctl( fp, SIS8300_REG_WRITE, &reg );
	ioctl( fp2, SIS8300_REG_WRITE, &reg );
	return -1;
      }
    }
    printf( "Databits OK\n" );

    
    // pattern 2 enablebits
    for( i = 0;i < 256;i++ ){
      // normal pattern
      out = ( i<<24 ) | ( 0xFF<<16 );
      expIn = i;
      
      // write-read-compare
      reg.data = out;
      ioctl( fp, SIS8300_REG_WRITE, &reg );
      if(del){usleep( del );}
      
      ioctl( fp2, SIS8300_REG_READ, &reg2 );
      
      if( reg2.data != expIn ){
//	ioctl( fp, SIS8300_REG_WRITE, &reg3 ); // generate trigger pulse
	printf( "Error on compare! normal pattern, loop: %d\n", i );
	printf( "written: %08X, readback: %08X, expected: %08X\n", out, reg2.data, expIn );
	// 2. Ausgabe zur Fehlersuche AG
//	for( l = 0;l < 20;l++ ){
//	  ioctl( fp2, SIS8300_REG_READ, &reg2 );
//	  printf( "written: %08X, readback: %08X, expected: %08X\n", out, reg2.data, expIn );
//	}  
	// Ende 2. Ausgabe zur Fehlersuche AG
	printf( "Failing bits:\n" );
	for( i = 0;i < 8;i++ ){
	  if( (reg2.data & 1<<i) != (expIn & 1<<i) ){
	    printf( "MLVDS Enablebit %d\n", i );
	  }
	}
	// disable driver
	reg.data = 0;
	ioctl( fp, SIS8300_REG_WRITE, &reg );
	ioctl( fp2, SIS8300_REG_WRITE, &reg );
	// disable leds
	reg.offset = SIS8300_USER_CONTROL_STATUS_REG;
	reg.data = 0x10000;
	ioctl( fp, SIS8300_REG_WRITE, &reg );
	ioctl( fp2, SIS8300_REG_WRITE, &reg );
	return -1;
      }
      
      // inverted pattern
      out = ( i<<24 ) | ( 0xFF<<16 );
      out = ( out & 0x00FFFFFF ) | (~out & 0xFF000000); // invert bits 24-31
      expIn = i;
      expIn = ( expIn & 0xFFFFFF00 ) | (~expIn & 0x000000FF); // invert bits 0-7
      
      // write-read-compare
      reg.data = out;
      ioctl( fp, SIS8300_REG_WRITE, &reg );
      if(del){usleep( del );}
      
      ioctl( fp2, SIS8300_REG_READ, &reg2 );
      
      if( reg2.data != expIn ){
//	ioctl( fp, SIS8300_REG_WRITE, &reg3 ); // generate trigger pulse
	printf( "Error on compare! inverted pattern, loop: %d\n", i );
	printf( "written: %08X, readback: %08X, expected: %08X\n", out, reg2.data, expIn );
	// 2. Ausgabe zur Fehlersuche AG
//	for( l = 0;l < 20;l++ ){
//	  ioctl( fp2, SIS8300_REG_READ, &reg2 );
//	  printf( "written: %08X, readback: %08X, expected: %08X\n", out, reg2.data, expIn );
//	}  
	// Ende 2. Ausgabe zur Fehlersuche AG
	printf( "Failing bits:\n" );
	for( i = 0;i < 8;i++ ){
	  if( (reg2.data & 1<<i) != (expIn & 1<<i) ){
	    printf( "MLVDS Enablebit %d\n", i );
	  }
	}
	// disable driver
	reg.data = 0;
	ioctl( fp, SIS8300_REG_WRITE, &reg );
	ioctl( fp2, SIS8300_REG_WRITE, &reg );
	// disable leds
	reg.offset = SIS8300_USER_CONTROL_STATUS_REG;
	reg.data = 0x10000;
	ioctl( fp, SIS8300_REG_WRITE, &reg );
	ioctl( fp2, SIS8300_REG_WRITE, &reg );
	return -1;
      }
    }
    printf( "Enablebits OK\n" );
    
    // disable driver
    reg.data = 0;
    ioctl( fp, SIS8300_REG_WRITE, &reg );
    ioctl( fp2, SIS8300_REG_WRITE, &reg );
    // disable leds
    reg.offset = SIS8300_USER_CONTROL_STATUS_REG;
    reg.data = 0x10000;
    ioctl( fp, SIS8300_REG_WRITE, &reg );
    ioctl( fp2, SIS8300_REG_WRITE, &reg );
    
    // Direction Module 2 -> Module 1
    
    printf( "Testing MLVDS I/O from '%s'(user led off) to '%s'(user led on)...\n", argv[3], argv[2] );
    
    reg.offset = SIS8300_USER_CONTROL_STATUS_REG;
    reg.data = 0x1;
    ioctl( fp, SIS8300_REG_WRITE, &reg );
    reg.data = 0x10000;
    ioctl( fp2, SIS8300_REG_WRITE, &reg );
    
    reg.offset = SIS8300_MLVDS_IO_CONTROL_REG;
    reg2.offset = SIS8300_MLVDS_IO_CONTROL_REG;
    
    // set module 1 to receiver
    reg.data = 0;
    ioctl( fp, SIS8300_REG_WRITE, &reg );

    // pattern 1 databits
    for( i = 0;i < 256;i++ ){
      // normal pattern
      out = ( 0xFF<<24 ) | ( i<<16 );
      expIn = i;
      
      // write-read-compare
      reg2.data = out;
      ioctl( fp2, SIS8300_REG_WRITE, &reg2 );
      if(del){usleep( del );}
      
      ioctl( fp, SIS8300_REG_READ, &reg );
      
      if( reg.data != expIn ){
//	ioctl( fp, SIS8300_REG_WRITE, &reg3 ); // generate trigger pulse
	printf( "Error on compare! normal pattern, loop: %d\n", i );
	printf( "written: %08X, readback: %08X, expected: %08X\n", out, reg.data, expIn );
	// 2. Ausgabe zur Fehlersuche AG
//	for( l = 0;l < 20;l++ ){
//	  ioctl( fp, SIS8300_REG_READ, &reg );
//	  printf( "written: %08X, readback: %08X, expected: %08X\n", out, reg.data, expIn );
//	}  
	// Ende 2. Ausgabe zur Fehlersuche AG
	printf( "Failing bits:\n" );
	for( i = 0;i < 8;i++ ){
	  if( (reg.data & 1<<i) != (expIn & 1<<i) ){
	    printf( "MLVDS Databit %d\n", i );
	  }
	}
	// disable driver
	reg.data = 0;
	ioctl( fp, SIS8300_REG_WRITE, &reg );
	ioctl( fp2, SIS8300_REG_WRITE, &reg );
	// disable leds
	reg.offset = SIS8300_USER_CONTROL_STATUS_REG;
	reg.data = 0x10000;
	ioctl( fp, SIS8300_REG_WRITE, &reg );
	ioctl( fp2, SIS8300_REG_WRITE, &reg );
	return -1;
      }
      
      // inverted pattern
      out = ( 0xFF<<24 ) | ( i<<16 );
      out = ( out & 0xFF00FFFF ) | (~out & 0x00FF0000); // invert bits 16-23
      expIn = i;
      expIn = ( expIn & 0xFFFFFF00 ) | (~expIn & 0x000000FF); // invert bits 0-7
      
      // write-read-compare
      reg2.data = out;
      ioctl( fp2, SIS8300_REG_WRITE, &reg2 );
      if(del){usleep( del );}
      
      ioctl( fp, SIS8300_REG_READ, &reg );
      
      if( reg.data != expIn ){
//	ioctl( fp, SIS8300_REG_WRITE, &reg3 ); // generate trigger pulse
	printf( "Error on compare! inverted pattern, loop: %d\n", i );
	printf( "written: %08X, readback: %08X, expected: %08X\n", out, reg.data, expIn );
	// 2. Ausgabe zur Fehlersuche AG
//	for( l = 0;l < 20;l++ ){
//	  ioctl( fp, SIS8300_REG_READ, &reg );
//	  printf( "written: %08X, readback: %08X, expected: %08X\n", out, reg.data, expIn );
//	}
	// Ende 2. Ausgabe zur Fehlersuche AG
	printf( "Failing bits:\n" );
	for( i = 0;i < 8;i++ ){
	  if( (reg.data & 1<<i) != (expIn & 1<<i) ){
	    printf( "MLVDS Databit %d\n", i );
	  }
	}
	// disable driver
	reg.data = 0;
	ioctl( fp, SIS8300_REG_WRITE, &reg );
	ioctl( fp2, SIS8300_REG_WRITE, &reg );
	// disable leds
	reg.offset = SIS8300_USER_CONTROL_STATUS_REG;
	reg.data = 0x10000;
	ioctl( fp, SIS8300_REG_WRITE, &reg );
	ioctl( fp2, SIS8300_REG_WRITE, &reg );
	return -1;
      }
    }
    printf( "Databits OK\n" );
 
    
    // pattern 2 enablebits
    for( i = 0;i < 256;i++ ){
      // normal pattern
      out = ( i<<24 ) | ( 0xFF<<16 );
      expIn = i;
      
      // write-read-compare
      reg2.data = out;
      ioctl( fp2, SIS8300_REG_WRITE, &reg2 );
      if(del){usleep( del );}
      
      ioctl( fp, SIS8300_REG_READ, &reg );
      
      if( reg.data != expIn ){
//	ioctl( fp, SIS8300_REG_WRITE, &reg3 ); // generate trigger pulse
	printf( "Error on compare! normal pattern, loop: %d\n", i );
	printf( "written: %08X, readback: %08X, expected: %08X\n", out, reg.data, expIn );
	// 2. Ausgabe zur Fehlersuche AG
//	for( l = 0;l < 20;l++ ){
//	  ioctl( fp, SIS8300_REG_READ, &reg );
//	  printf( "written: %08X, readback: %08X, expected: %08X\n", out, reg.data, expIn );
//	}  
	// Ende 2. Ausgabe zur Fehlersuche AG
	printf( "Failing bits:\n" );
	for( i = 0;i < 8;i++ ){
	  if( (reg.data & 1<<i) != (expIn & 1<<i) ){
	    printf( "MLVDS Enablebit %d\n", i );
	  }
	}
	// disable driver
	reg.data = 0;
	ioctl( fp, SIS8300_REG_WRITE, &reg );
	ioctl( fp2, SIS8300_REG_WRITE, &reg );
	// disable leds
	reg.offset = SIS8300_USER_CONTROL_STATUS_REG;
	reg.data = 0x10000;
	ioctl( fp, SIS8300_REG_WRITE, &reg );
	ioctl( fp2, SIS8300_REG_WRITE, &reg );
	return -1;
      }
      
      // inverted pattern
      out = ( i<<24 ) | ( 0xFF<<16 );
      out = ( out & 0x00FFFFFF ) | (~out & 0xFF000000); // invert bits 24-31
      expIn = i;
      expIn = ( expIn & 0xFFFFFF00 ) | (~expIn & 0x000000FF); // invert bits 24-31 and 0-7
      
      // write-read-compare
      reg2.data = out;
      ioctl( fp2, SIS8300_REG_WRITE, &reg2 );
      if(del){usleep( del );}
      
      ioctl( fp, SIS8300_REG_READ, &reg );
      
      if( reg.data != expIn ){
//	ioctl( fp, SIS8300_REG_WRITE, &reg3 ); // generate trigger pulse
	printf( "Error on compare! inverted pattern, loop: %d\n", i );
	printf( "written: %08X, readback: %08X, expected: %08X\n", out, reg.data, expIn );
	// 2. Ausgabe zur Fehlersuche AG
//	for( l = 0;l < 20;l++ ){
//	  ioctl( fp, SIS8300_REG_READ, &reg );
//	  printf( "written: %08X, readback: %08X, expected: %08X\n", out, reg.data, expIn );
//	}  
	// Ende 2. Ausgabe zur Fehlersuche AG
	printf( "Failing bits:\n" );
	for( i = 0;i < 8;i++ ){
	  if( (reg.data & 1<<i) != (expIn & 1<<i) ){
	    printf( "MLVDS Enablebit %d\n", i );
	  }
	}
	// disable driver
	reg.data = 0;
	ioctl( fp, SIS8300_REG_WRITE, &reg );
	ioctl( fp2, SIS8300_REG_WRITE, &reg );
	// disable leds
	reg.offset = SIS8300_USER_CONTROL_STATUS_REG;
	reg.data = 0x10000;
	ioctl( fp, SIS8300_REG_WRITE, &reg );
	ioctl( fp2, SIS8300_REG_WRITE, &reg );
	return -1;
      }
    }
    
    // disable driver
    reg.data = 0;
    ioctl( fp, SIS8300_REG_WRITE, &reg );
    ioctl( fp2, SIS8300_REG_WRITE, &reg );
    // disable leds
    reg.offset = SIS8300_USER_CONTROL_STATUS_REG;
    reg.data = 0x10000;
    ioctl( fp, SIS8300_REG_WRITE, &reg );
    ioctl( fp2, SIS8300_REG_WRITE, &reg );
    
    printf( "Enablebits OK\n" );

    printf( "Test OK\n" );
    
  } // End of Loops    
    
  }
//  }  // end of while - für Testzwecke eingefügt (AG)
  return 0;  
}
