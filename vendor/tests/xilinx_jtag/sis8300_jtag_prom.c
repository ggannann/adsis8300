#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <linux/fs.h>
#include <stdint.h>
#include "../../driver/sis8300_defs.h"

#define SIS8300_XILINX_JTAG_REG  			0x02

#define CE_PROMPT       1     // display no command menu, only prompt
#define CE_PARAM_ERR    2
#define CE_NO_DMA_BUF   3
#define CE_FORMAT       4


#define BYPASS    0xFFFF
#define IDCODE    0x00FE
#define ISPEN     0x00E8
#define FPGM      0x00EA
//#define FADDR     0x00EB
#define ISC_ADDRESS_SHIFT     0x00EB
//#define FVFY1     0x00F8
#define XCS_READ  0x00EF
#define NORMRST   0x00F0
#define FERASE    0x00EC
#define SERASE    0x000A
#define FDATA0    0x00ED
#define CONFIG    0x00EE

#define XSC_UNLOCK			0xAA55
#define ISC_NOOP			0x00E0
#define XSC_BLANK_CHECK		0x000D
#define XSC_CLEAR_STATUS	0x00F4
#define XSC_DATA_DONE		0x0009

#define ISC_ENABLE      0x00E8
#define XSC_ENABLEC     0x00E9
#define XSC_CONFIG      0x00EE
#define XSC_CONFIG_PLUS 0x00F6
#define XSC_DATA_SUCR	0x000E
#define XSC_DATA_CCB	0x000C
#define XSC_DATA_RDPT	0x0004
#define XSC_DATA_WRPT	0x00F7
#define XSC_DATA_BTC	0x00F2

#define MAX_NOF_JTG_DEVS   2

unsigned int gl_jtg_nof_devices ;
unsigned int gl_jtg_select ;
int gl_jtg_state ;
unsigned int gl_jtg_dev_idcodes[MAX_NOF_JTG_DEVS] ;
unsigned int gl_jtg_ir_len[MAX_NOF_JTG_DEVS] ;

int	gl_prom_XCF32P = 0 ;  


unsigned char gl_wbuf[0x400000]; /* 4MByte ;  min. 2MByte for one XCF16P */
unsigned char gl_rbuf[0x400000]; /* 4MByte ;  min. 2MByte for one XCF16P */



/*===========================================================================*/
/* Prototypes					  			     */
/*===========================================================================*/
int mcs_file_unpack (char* fname, char* data_buffer, unsigned int* mcs_data_byte_length) ;

int jtag_tap_reset (int device_fp) ;

int jtag_reset (int device_fp) ;
int jtag_instruction(int device_fp, unsigned int icode) ;
unsigned int jtag_data(int device_fp, unsigned int din, unsigned int len) ;
int jtag_rd_data(int device_fp, void *buf, unsigned int len, int state) ;
int jtag_wr_data(int device_fp, void  *buf, unsigned int len, int   state) ;


int jtag_program_verifier (int device_fp, char* fname, int program) ;
int jtag_erase (int device_fp);

void print_jtag_chain (void ) ;


//int jtag_program (unsigned int device_fp, char* fname) ;
int jtag_done (int device_fp) ;
int jtag_check_blank (int device_fp) ;
int jtag_read (int device_fp, char* fname) ;

void jtag_tap_pause_1ms (void);
void jtag_tap_pause_5s (void);
void jtag_erase_pause_30s (void);


unsigned int gl_debug = 0 ;


int main(int argc, char **argv){

char* mcs_file_name_ptr;
char* cmd_parameter_ptr ;

int dev_fp;
sis8300_reg myReg;

int return_code ;


if (argc!=4)   {
  fprintf(stderr, "\n\n\n\n");
  fprintf(stderr, "usage: %s  PATH   CMD-Paramter   PROM-FILE-Name      \n", argv[0]);
  fprintf(stderr, "\n");
  fprintf(stderr, "example: %s /dev/sis8300-0  V sis8300_v0101.mcs     \n", argv[0]);
  fprintf(stderr, "\n");
  fprintf(stderr, "CMD-Paramter:     I     Read IDCode(s)            \n");
  fprintf(stderr, "                  V     Verify Prom against File     \n");
  fprintf(stderr, "                  P     Program Prom with File and Load FPGA     \n");
  fprintf(stderr, "                  E     Erase     \n");
  fprintf(stderr, "                  C     Check blank     \n");
  fprintf(stderr, "                  R     Read     \n");
  
  
  fprintf(stderr, "\n\n\n\n");
  return -1;
  }

  dev_fp = open(argv[1], O_RDWR);
  if(dev_fp < 0){
    printf("can't open device\n");
    return -1;
  }


  cmd_parameter_ptr = argv[2] ;

  printf("\033[?25l"); // Curser OFF

  printf("\n");
  printf("\n");
  myReg.offset = 0x00;
  ioctl(dev_fp, SIS8300_REG_READ, &myReg);
  printf("Module and Version = 0x%08X\n", myReg.data);
  myReg.offset = 0x01;
  ioctl(dev_fp, SIS8300_REG_READ, &myReg);
  printf("Serial Number      = 0x%08X\n", myReg.data);
  printf("\n");


	myReg.offset = 0x04;
	myReg.data = 1; // set USER LED
	ioctl(dev_fp, SIS8300_REG_WRITE, &myReg);

	switch (*cmd_parameter_ptr) {
	
		case 'I' :
			printf("I\n");

			gl_jtg_select = 0 ;
			jtag_reset (dev_fp) ;
			print_jtag_chain();
			jtag_tap_reset (dev_fp) ;
			break;
		case 'V' :
			printf("V\n");
			if (argc<4)   {
				printf("not PROM-FILE-Name\n");
				return -1 ;
			}
			mcs_file_name_ptr = argv[3] ;

			jtag_reset (dev_fp) ;
			print_jtag_chain();
			if (gl_jtg_nof_devices == 0)  { 
				printf("\n\nERROR:     \n\n\n");
				return -1 ;
			}
			else {
				gl_jtg_select = 0 ;
				return_code = jtag_program_verifier (dev_fp, mcs_file_name_ptr,0) ;
				if (return_code != 0) {
					printf("\nVerifier failed !!!!!!!!!!!!!!!!!!!!!!!!!! \n\n");
				}
				else {
					printf("\nVerifier OK \n\n");
				}
                                system("date");
			}
			jtag_tap_reset (dev_fp) ;
			break;

		case 'P' :
			printf("P\n");
			if (argc<4)   {
				printf("not PROM-FILE-Name\n");
				return -1 ;
			}
			mcs_file_name_ptr = argv[3] ;
			jtag_reset (dev_fp) ;
			print_jtag_chain();
			if (gl_jtg_nof_devices == 0)  { 
				printf("\n\nERROR:     \n\n\n");
			}
			else {
				gl_jtg_select = 0 ;
	
				printf("\n");
				printf("Erase Prom (30 sec.) \n");
				return_code = jtag_erase (dev_fp) ;
				printf("Blank Check \n");
				return_code=jtag_check_blank (dev_fp) ;
				if (return_code != 0) {
					printf("Prom is not empty\n");   
				}
				else {
					printf("Prom is empty\n");   
					return_code = jtag_program_verifier (dev_fp, mcs_file_name_ptr,1) ;
					if (return_code != 0) {
						printf("\nVerifier failed !!!!!!!!!!!!!!!!!!!!!!!!!! \n\n");
					}
					else {
						printf("\nVerifier OK \n\n");
	//					return_code=jtag_instruction(dev_fp, CONFIG);
						//printf("CONFIG %08X\n", ret);   
						printf("\nLoad FPGA \n\n");
                                                system("date");
						sleep(2) ;
						
					     
					}
				}
	
			}
			break;


		case 'E' :
			printf("E\n");
			jtag_reset (dev_fp) ;
			print_jtag_chain();
			if (gl_jtg_nof_devices == 0)  { 
				printf("\n\nERROR:     \n\n\n");
				return -1 ;
			}
			gl_jtg_select = 0 ;
			printf("Erase Prom (30 sec.) \n");
			return_code = jtag_erase (dev_fp) ;
			printf("Blank Check \n");
			return_code=jtag_check_blank (dev_fp) ;
			if (return_code == 0) {
				printf("prom is empty\n");   
			}
			else {
				printf("prom is not empty\n");   
			}
			break;


	
		case 'C' :
			printf("C\n");

			jtag_reset (dev_fp) ;
			print_jtag_chain();
			if (gl_jtg_nof_devices == 0)  { 
				printf("\n\nERROR:     \n\n\n");
				return -1 ;
			}
			gl_jtg_select = 0 ;
			printf("Blank Check \n");
			return_code=jtag_check_blank (dev_fp) ;
			if (return_code == 0) {
				printf("prom is empty\n");   
			}
			else {
				printf("prom is not empty\n");   
			}
			break;
				
		case 'R' :
			printf("R\n");
			
			if (argc<4)   {
				printf("no  PROM-FILE-Name\n");
				return -1 ;
			}
			mcs_file_name_ptr = argv[3] ;
			
			jtag_reset (dev_fp) ;
			print_jtag_chain();
			if (gl_jtg_nof_devices == 0)  { 
				printf("\n\nERROR:     \n\n\n");
				return -1 ;
			}
			gl_jtg_select = 0 ;
			return_code=jtag_read (dev_fp, mcs_file_name_ptr) ;
	
			break;
		default:
			printf("not valid CMD-Paramter\n");
			return -1 ;
			break;
	}


	printf("\033[?25h"); // Curser ON
	close(dev_fp);

	return 0;
}





















/***************************************************************************************************************/
/***************************************************************************************************************/
/***************************************************************************************************************/
/***************************************************************************************************************/
void jtag_tap_pause_1ms ()
{
  usleep(1000) ;
  return  ;
}


/***************************************************************************************************************/
void jtag_tap_pause_5s ()
{
  usleep(5000000) ;
  return  ;
}

/***************************************************************************************************************/
void jtag_erase_pause_30s ()
{
  usleep(30000000) ;
  return  ;
}


/***************************************************************************************************************/
/***************************************************************************************************************/
/***************************************************************************************************************/
/***************************************************************************************************************/


 

int jtag_tap_reset (int device_fp)
{
sis8300_reg myReg;
unsigned int ux,uy;

  myReg.offset = SIS8300_XILINX_JTAG_REG;

  ux = 0 ;
  do {
     uy=0;
     do { 
	  myReg.data = 0x2; //  
	  ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
	 } while (++uy < 5);

	  myReg.data = 0x0; //  
	  ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
   } while (++ux < 3);

  return 0 ;
}




int jtag_reset (int device_fp)
{
sis8300_reg myReg;
unsigned int data;
unsigned int ux,uy;

  printf("JTAG Reset\n");
  system("date");
  myReg.offset = SIS8300_XILINX_JTAG_REG;
  ux = 0 ;
  do {
     uy=0;
     do { 
	  myReg.data = 0x2;   // set TMS and 5 x TCLK
	  ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
	  usleep(1);
     } while (++uy < 5);
      myReg.data = 0x0;   // 
      ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
      usleep(1);
  } while (++ux < 3);

  myReg.data = 0x2;   // 
  ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
  usleep(1);
  
  myReg.data = 0x0;   // 
  ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
  usleep(1);

  gl_jtg_nof_devices = 0 ;	
  gl_jtg_state = -1 ;
  ux=MAX_NOF_JTG_DEVS;

   while (1) {
      uy=0;
      do {
	  myReg.data = 0x0;   // 
	  ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
	  usleep(1);
      } while (++uy < 32);

      ioctl(device_fp, SIS8300_REG_READ, &myReg);
      data = myReg.data;
      //printf("ux = 0x%08x   data=0x%08x \n",ux, data);	  
      if ((data == 0) || (data == 0xffffffff) || (ux == 0)) break;
	  gl_jtg_nof_devices = gl_jtg_nof_devices+ 1 ;
 	  gl_jtg_dev_idcodes[MAX_NOF_JTG_DEVS-ux] = data ;
 	  //gl_jtg_ir_len[MAX_NOF_JTG_DEVS-ux] = 8 ; /* valid only for 18v0x proms */
	  --ux;
	  gl_jtg_state = 0 ;
	  //printf("inside ux = 0x%08x   myReg=0x%08x \n",ux, data);	  
   }



  myReg.data = 0x2;   // 
  ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
  usleep(1);

  myReg.data = 0x2;   // 
  ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
  usleep(1);

  myReg.data = 0x0;   // 
  ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
  usleep(1);
  

  return 0 ;
}




/*--------------------------- jtag_instruction ------------------------------*/
 
int jtag_instruction(int device_fp, unsigned int icode)
{
sis8300_reg myReg;
  int return_code ;
  unsigned int put_nof_data ;
  unsigned int data ;
  unsigned int addr ;
  unsigned int read_flag=0 ;
   u_int    ux,uy;
   u_int    tdo;
   u_char   ms;

  myReg.offset = SIS8300_XILINX_JTAG_REG;

  myReg.data = 0x3; //  
  ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
  usleep(1);
  myReg.data = 0x3; //  
  ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
  usleep(1);

  myReg.data = 0x1; //  
  ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
  usleep(1);
  myReg.data = 0x1; //  
  ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
  usleep(1);

  ux=gl_jtg_nof_devices;
  tdo=0; 
  ms=0;   


// es muss mit Device ux=0 angefangen werden
//#define richtig_rum
//#ifdef richtig_rum
  ux=0;
  do {
	  //read_flag=0;
      uy=0;
      do {
         if ((ux == gl_jtg_nof_devices-1) && (uy == (gl_jtg_ir_len[ux] -1))) ms=2;

         if (ux != (gl_jtg_nof_devices-1 - gl_jtg_select) ) {
	      myReg.data = ms |1 ; //  
	      ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
	      usleep(1);
         } else {

	      ioctl(device_fp, SIS8300_REG_READ, &myReg);
	      data = myReg.data;
	      tdo |=((data >>31) &1) <<uy;

	      myReg.data = ms |(icode &1) ; //  
	      ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
	      usleep(1);
	      icode >>=1;
         }
      } while (++uy < gl_jtg_ir_len[ux]);

  } while (++ux<gl_jtg_nof_devices)  ;



  myReg.data = 0x3; //  
  ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
  usleep(1);
  myReg.data = 0x1; //  
  ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
  usleep(1);


	//printf("\nend: tdo=0x%08x \n\n",tdo);
  return tdo;
}






/*--------------------------- jtag_data -------------------------------------*/
//
unsigned int jtag_data(int device_fp, unsigned int din, unsigned int len)
{
  sis8300_reg myReg;
  int return_code ;
  unsigned int data ;
  unsigned int addr ;
  u_int    dev,uy;
  u_int32_t   tdo;
  u_char   ms;

  //errno=0;

  myReg.offset = SIS8300_XILINX_JTAG_REG;

  myReg.data = 0x3; //  
  ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
  usleep(1) ;
  myReg.data = 0x1; //  
  ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
  usleep(1) ;
  myReg.data = 0x1; //  
  ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
  usleep(1) ;

   
  dev=gl_jtg_nof_devices ;   
  tdo=0; 
  ms=0;

  
   
  while (dev) {
      dev--;
      if (dev != gl_jtg_select) {
		data = (dev) ? 1 : 3;
		myReg.data = data ; //  
		ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
		usleep(1) ;
     } else {
         uy=0;
         while (len) {
            len--;
            if ((dev == 0) && (len == 0)) ms=2;
		ioctl(device_fp, SIS8300_REG_READ, &myReg);
		usleep(1) ;
		data = myReg.data;
		tdo |=((data >>31) &1) <<uy;
		uy++;

		data=ms |((u_char)din &1);
		din >>=1;
		myReg.data = data ; //  
		ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
		usleep(1) ;
		
         }
      }
   }

  myReg.data = 0x3; //  
  ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
  usleep(1) ;

  myReg.data = 0x1; //  
  ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
  usleep(1) ;

  return tdo;
}












/*-------------------------- jtag_rd_data ---------------------------------- */
 
int jtag_rd_data(int device_fp, 
   void  *buf,
   unsigned int len,     /*  0: end */
   int   state)   /*  1: start */
                  /*  0: continue */
                  /* -1: end */
{
volatile sis8300_reg myReg;
  int return_code ;
  unsigned int data ;
  unsigned int addr ;
  u_int32_t   *bf;
  u_int    ux;
  unsigned int data_array[0x100] ;
  unsigned int i_ux ;
  unsigned int req_nof_data, put_nof_data ;

  
  myReg.offset = SIS8300_XILINX_JTAG_REG;

  if (gl_jtg_state < 0) return CE_PROMPT;

   if (len == 0) {
      if (gl_jtg_state == 1) { 
	      myReg.data = 0x3; //  
	      ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
	      usleep(1) ;
	      myReg.data = 0x3; //  
	      ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
	      usleep(1) ;
	      myReg.data = 0x1; //  
	      ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
	      usleep(1) ;
	      gl_jtg_state=0; 
	  }
      return 0;
   }


   if ((state == 1) && (gl_jtg_state > 0)) {
	      myReg.data = 0x3; //  
	      ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
	      usleep(1) ;
	      myReg.data = 0x3; //  
	      ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
	      usleep(1) ;
	      myReg.data = 0x1; //  
	      ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
	      usleep(1) ;

	      gl_jtg_state=0;
   }



   if (gl_jtg_state == 0) {
	      myReg.data = 0x3; //  
	      ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
	      //usleep(1) ;
	      myReg.data = 0x1; //  
	      ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
	      usleep(1) ;

	      ux=gl_jtg_nof_devices;
	      while (--ux != gl_jtg_select) {
		  myReg.data = 0x1; //  
		  ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
		  usleep(1) ;
	      }
	      gl_jtg_state=1;
   }



  bf=(u_int32_t*)buf;
  //bf=(u_int32_t*)buf;
	  //printf("len=0x%08x \n",len);
  while (len > 4) {

    ux=0;
    do {
	myReg.data = 0x1; //  
	ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
    // dummy read
	ioctl(device_fp, SIS8300_REG_READ, &myReg);
	data = myReg.data;
	//usleep(1) ;
    } while (++ux < 32);

    ioctl(device_fp, SIS8300_REG_READ, &myReg);
    data = myReg.data;
	  //printf("data=0x%08x\n",data);
    *bf++ = data ;
    len -=4;
  }


    ux=0;
    do {
	  myReg.data = 0x1; //  
	  ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
    // dummy read
	ioctl(device_fp, SIS8300_REG_READ, &myReg);
	data = myReg.data;
	//usleep(1) ;
    } while (++ux < 32);

    ioctl(device_fp, SIS8300_REG_READ, &myReg);
    data = myReg.data;
    *bf++ = data ;

  
   if (state < 0) {
	      myReg.data = 0x3; //  
	      ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
	      usleep(1) ;
	      myReg.data = 0x3; //  
	      ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
	      usleep(1) ;
	      myReg.data = 0x1; //  
	      ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
	      usleep(1) ;
	      gl_jtg_state=0;
   }

   return 0;
}




 
/*--------------------------- jtag_wr_data ----------------------------------*/

int jtag_wr_data(int device_fp, 
   void  *buf,
   unsigned int len,     /* Anzahl Byte */
   int   state)          /* -1: end     */
{
sis8300_reg myReg;
  unsigned int req_nof_data ;
  unsigned int put_nof_data ;
  int return_code ;
	unsigned int data ;
	unsigned int addr ;
	unsigned int *bf;
	unsigned int din;
	unsigned int dev;
	unsigned int ux;
	unsigned char ms;

   if (gl_jtg_state < 0) return CE_PROMPT;
	//printf("len=0x%08x\n",len);

   bf=(u_int32_t*)buf; 
   len >>=2;
   if (len == 0) return 0;

  myReg.offset = SIS8300_XILINX_JTAG_REG;

  myReg.data = 0x3; //  
  ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
  usleep(1);

  myReg.data = 0x1; //  
  ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
  usleep(1);
  myReg.data = 0x1; //  
  ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
  usleep(1);
	  
  dev=gl_jtg_nof_devices ;   
  ms=0;

  
  while (dev) {
      dev--;
      if (dev != gl_jtg_select) {
		data = (dev) ? 1 : 3;
		myReg.data = data ; //  
		ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
      } else {
			//printf("len=0x%08x\n",len);
	while (len) {
            len--;
            din=*bf++; ux=32;
            while (ux) {
		ux--;
		if ((dev == 0) && (len == 0) && (ux == 0)) ms=2;
		data = ms |((u_char)din &1);
		myReg.data = data ; //  
		ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
		din >>=1;
            }
         }
      }

   }


   if (state < 0) {
	myReg.data = 0x3; //  
	ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
	myReg.data = 0x1; //  
	ioctl(device_fp, SIS8300_REG_WRITE, &myReg);
	gl_jtg_state=0;
   }

   return 0;
}





int mcs_file_unpack (char* fname, char* data_buffer, unsigned int* mcs_data_byte_length)
{
	int retcode ;
	char line_in[0x1000]  ;
    FILE *loaderfile;


	unsigned int line_length  ;
	unsigned int nof_byte_in_record	;	
	unsigned int byte_address_in_record	 ;	
	unsigned int type_in_record ;
	unsigned int extended_address_in_record ;
	unsigned char databyte_in_record[0x10] ;
	unsigned int wr_buf_index ;

	unsigned int i  ;


// begin modification XCF32P
	for (i=0;i<0x400000;i++) {
		data_buffer[i]  = (char)0xff ;
	} 
// end modification XCF32P


	*mcs_data_byte_length = 0 ;
	wr_buf_index = 0 ;

	loaderfile=fopen(fname,"r");
    if (loaderfile) {
    //if ((int)loaderfile>0) {
		printf("\nloader file %s opened\n",fname);
	}
     else {
		printf("\n\nERROR:   loader file %s not found\n\n",fname);
		return -1;
    }

	retcode=fscanf(loaderfile,"%s\n",line_in);	       
	while (retcode>0) {
		/* printf("%s\n",line_in); */
		/* check 1. character */
		retcode = strncmp(line_in,":",1) ;
		if (retcode != 0) {
			printf("mcs-file error: 1.character is not ':' \n");
			return -1 ;
		}


		/* read in_line length */
		line_length = strlen(line_in) ;
		if (line_length <= 10) {
			printf("mcs-file error: number of character in recordline is less than 11 \n");
			return -1 ;
		}


		retcode=sscanf(&line_in[1],"%2x%4x%2x",&nof_byte_in_record, 
													 &byte_address_in_record, &type_in_record) ;

		if(retcode != 3) {
			printf("mcs-file error: format error1 \n");
			return -1 ;
		}
		/* printf("nof_byte_in_record     = 0x%08x\n",nof_byte_in_record);     */
 		/* printf("byte_address_in_record = 0x%08x\n",byte_address_in_record); */
 		/* printf("type_in_record         = 0x%08x\n",type_in_record);         */

		switch (type_in_record) {

			case 0x0 : { /* data record */
				/* address check */
				if(wr_buf_index > 0x400000) {
					printf("mcs-file error: file is too long \n");
					printf("wr_buf_index     = 0x%08x\n",wr_buf_index);  /*   */
					return -1 ;
				}

				if (byte_address_in_record  != (wr_buf_index & 0xffff)) {
					printf("mcs-file error: address error! \n");
					printf("byte_address_in_record   =  0x%04x! \n",byte_address_in_record);
					printf("(wr_buf_index && 0xffff) =  0x%04x! \n\n",(wr_buf_index & 0xffff));
					return -1 ;
				}	;

				
				for (i=0;i<nof_byte_in_record;i++) {
					retcode=sscanf(&line_in[9+(2*i)],"%2x", databyte_in_record) ;
					if(retcode != 1) {
						printf("mcs-file error: data_in_record \n");
						return -1 ;
					}
					//printf("wr_buf_index = 0x%08x   databyte_in_record = 0x%02x\n",wr_buf_index,databyte_in_record[0]);     
					data_buffer[wr_buf_index]  =  databyte_in_record[0] ;
					wr_buf_index++ ;
				} 
			}  
			break;
			
			case 0x1 : { /* end-of_record */
				//printf("end of mcs-file  \n");
				/* printf("wr_buf_index                  = 0x%08x\n",wr_buf_index);  /*   */
				/* printf("data_buffer[wr_buf_index-1]   = 0x%08x\n",data_buffer[wr_buf_index-1]);  /*   */
				/* printf("data_buffer[wr_buf_index]     = 0x%08x\n",data_buffer[wr_buf_index]);  /*   */
				/* printf("data_buffer[wr_buf_index+1]   = 0x%08x\n",data_buffer[wr_buf_index+1]);  /*   */
				if(wr_buf_index <= 0x30000) {
					printf("mcs-file error: not enough data \n");
					printf("wr_buf_index     = 0x%08x\n",wr_buf_index);  /*   */
					return -1 ;
				}
				*mcs_data_byte_length = wr_buf_index ;
				return 0 ;
			}  
			break;
			
			case 0x2 : { /* extended segment address record */
				printf("mcs-file error: not supported  TYPE of data \n");
				return -1 ;
			}  
			break;
			
			case 0x4 : { /* extended linear address record */
				/* check address */
				retcode=sscanf(&line_in[9],"%4x",&extended_address_in_record) ;
				if(retcode != 1) {
					printf("mcs-file error: extended_address_in_record \n");
					return -1 ;
				}

				if (extended_address_in_record  != (wr_buf_index >> 16)) {
					printf("mcs-file error: extended linear address ! \n");
					printf("extended_address_in_record =  0x%04x! \n",extended_address_in_record);
					printf("(wr_buf_index >> 16)   =  0x%04x! \n\n",(wr_buf_index >> 16));
					return -1 ;
				}	;
			}  
			break;
			
			default : {
				printf("mcs-file error: wrong TYPE of data \n");
				return -1 ;
			}  
			break;
		}  /* switch */
		retcode=fscanf(loaderfile,"%s\n",line_in);	       
	} 

  return 0 ;
}










/*---------------------------   -------------------------------------*/




void print_jtag_chain (void )
{
	unsigned int i, i_ux ;
	gl_prom_XCF32P = 0 ;  


	printf("number of devices  = %d   \n\n",gl_jtg_nof_devices);
	if (gl_jtg_nof_devices == 0) {
		printf("no device found  \n");
		printf("check if JTAG is not connected !  \n");
	}
	else {
	  for (i=0;i<gl_jtg_nof_devices;i++)  {
		i_ux = (gl_jtg_nof_devices - 1) - i ;
		printf("device %d :  dev_idcodes = 0x%08x       ",i+1,gl_jtg_dev_idcodes[i_ux]);

		if ((gl_jtg_dev_idcodes[i_ux] & 0x0fffffff) == 0x05026093) {
			printf("device = 18V04 \n");
			gl_jtg_ir_len[i_ux] = 8 ; /* valid only for 18v0x proms */
		}	
		else {
		  if ((gl_jtg_dev_idcodes[i_ux] & 0x0fffffff) == 0x05036093) {
			printf("device = 18V04 (STM)\n");
			gl_jtg_ir_len[i_ux] = 8 ; /* valid only for 18v0x proms */
		  }	
		  else {
			if ((gl_jtg_dev_idcodes[i_ux] & 0x0fffffff) == 0x0061C093) {
			  printf("device = XC2S200\n");
			  gl_jtg_ir_len[i_ux] = 5 ; /* valid  for XC2S200 */
			}	
			else {
			  if ((gl_jtg_dev_idcodes[i_ux] & 0x0fffffff) == 0x00a30093) {
				printf("device = XC2S600 \n");
				gl_jtg_ir_len[i_ux] = 5 ; /* valid XC2S600 */
			  }	
			  else {
				if ((gl_jtg_dev_idcodes[i_ux] & 0x0fffffff) == 0x05058093) {
					printf("device = XCF16P \n");
					gl_jtg_ir_len[i_ux] = 16 ; /* valid XCF16P */
				}	
// begin modification XCF32P
				else {
					if ((gl_jtg_dev_idcodes[i_ux] & 0x0fffffff) == 0x05059093) {
						printf("device = XCF32P \n");
						gl_prom_XCF32P = 1 ; /* valid XCF32P */
						gl_jtg_ir_len[i_ux] = 16 ; /* valid XCF32P */
					}	
					else {
					  if ((gl_jtg_dev_idcodes[i_ux] & 0xffffffff) == 0xc2a96093) {
						  printf("device = xc5vlx50t \n");
						  gl_jtg_ir_len[i_ux] = 10 ; /* valid xc5vlx50t   */ 
					  }	
					  else {
						printf("unknown device  \n");
					  }	
					}	
// begin modification XCF32P
				}	
			  }	
			}	
		  }	
		}	
	  }	
	}
	printf("  \n");


}
   




















 

int jtag_program_verifier (int device_fp, char* fname, int program)
{
//	int return_code ;
//	unsigned int data ;
//	unsigned int addr ;
	unsigned int i  ;
	unsigned int mcs_buffer_length  ;
	unsigned int rest_mcs_buffer_length ;

	int rest_write_length  ;	
	unsigned int max_write_length  ; 
	unsigned int write_byte_index  ;

	unsigned int rest_read_length ;
	unsigned int max_read_length  ;
	unsigned int read_length      ;
	unsigned int read_byte_index ;
	unsigned int max_error_value ;
	unsigned int error_counter ;
	int      ret;
    int retcode=1;

        printf("JTAG program/verifier\n");
        system("date");
	
	//  u_long   lx;

	jtag_reset (device_fp) ;
	print_jtag_chain();
	if (gl_jtg_nof_devices == 0)  { 
		printf("\n\nERROR:     \n\n\n");
		return -1 ;
	}

	//gl_jtg_select = 0 ;

	retcode=mcs_file_unpack (fname, gl_wbuf, &mcs_buffer_length) ;
	if (retcode != 0) {
		printf("\n\nERROR:  mcs-file unpack    \n\n\n");
		return -1 ;
	}

	//mcs_buffer_length = 0x200 ; // only for test
	//printf("\n\ngl_wbuf = 0x%02x 0x%02x 0x%02x 0x%02x    \n", gl_wbuf[0], gl_wbuf[1], gl_wbuf[2], gl_wbuf[3] );



 
/*--- ISP PROM programmieren */
 

	if (program == 1) {
		jtag_tap_reset (device_fp) ;

		if (gl_debug==1) { printf("\n"); }  
		ret=jtag_instruction(device_fp, ISPEN);
		if (gl_debug==1) { printf("ISPEN.0 %08X\n", ret);}   
		ret=jtag_data(device_fp, 0xd0, 8); // 
		if (gl_debug==1) { printf("jtag_data       %08X\n", ret); }   

		if (gl_debug==1) { printf("\n");   }
		ret=jtag_instruction(device_fp, ISPEN);
		if (gl_debug==1) { printf("ISPEN.0 %08X\n", ret); }   
		ret=jtag_data(device_fp, 0x03, 8); // 
		if (gl_debug==1) { printf("jtag_data       %08X\n", ret); }   

		if (gl_debug==1) { printf("\n");   }
		ret=jtag_instruction(device_fp, XSC_DATA_BTC);
		if (gl_debug==1) { printf("XSC_DATA_BTC.0 %08X\n", ret); }  

// begin modification XCF32P
		if (gl_prom_XCF32P == 1) {
			ret=jtag_data(device_fp, 0xffffffeC, 32); // 
		}
		else {
			ret=jtag_data(device_fp, 0xffffffe4, 32); // 
		}
// end modification XCF32P
		
		
		if (gl_debug==1) { printf("jtag_data       %08X\n", ret); }  

		if (gl_debug==1) { printf("\n"); }  
		ret=jtag_instruction(device_fp, FPGM);
		if (gl_debug==1) { printf("FPGM.0 %08X\n", ret); }   

		jtag_tap_pause_1ms () ;
		jtag_tap_reset (device_fp) ;


		rest_write_length = mcs_buffer_length ;	
		printf("mcs_buffer_length = %08X\n", mcs_buffer_length);   

		rest_write_length = (rest_write_length + 0x1ff) & 0xfffe00 ; /*  */	

		max_write_length = 0x20 ; /*   */
		write_byte_index = 0 ;
	
		printf("\n\nWrite data to prom \n");
		printf("\n\nrest_write_length 0x%08x \n", rest_write_length);

		if (gl_debug==1) { printf("\n"); }   
		ret=jtag_instruction(device_fp, ISPEN);
		if (gl_debug==1) { printf("ISPEN.0 %08X\n", ret);   }
		ret=jtag_data(device_fp, 0xd0, 8); // 
		if (gl_debug==1) { printf("jtag_data       %08X\n", ret); }   
		
		do {
			jtag_instruction(device_fp, FDATA0);
			ret=jtag_wr_data(device_fp, &gl_wbuf[write_byte_index], max_write_length, -1) ;			
			if (ret != 0) {	return -1 ;}
			if (write_byte_index == 0) { /* ony first time */
				ret=jtag_instruction(device_fp, ISC_ADDRESS_SHIFT);
				//printf("\nISC_ADDRESS_SHIFT.0 %02X\n", ret);
				//if (ret != 0x159) {printf("\nISC_ADDRESS_SHIFT.0 %02X\n", ret);}
				if ((ret&0xfff0) != 0x150) {printf("\nISC_ADDRESS_SHIFT.0 %02X\n", ret);}
				jtag_data(device_fp, 0, 24);
			}
			if (write_byte_index == 0x100000) { /* second bank */
				ret=jtag_instruction(device_fp, ISC_ADDRESS_SHIFT);
				if ((ret&0xfff0) != 0x150) {printf("\nISC_ADDRESS_SHIFT.1 %02X\n", ret);}
				jtag_data(device_fp, 0x100000, 24);
			}

			
// begin modification XCF32P
			if (write_byte_index == 0x200000) { /* 3. bank */
				ret=jtag_instruction(device_fp, ISC_ADDRESS_SHIFT);
				if ((ret&0xfff0) != 0x150) {printf("\nISC_ADDRESS_SHIFT.2 %02X\n", ret);}
				jtag_data(device_fp, 0x200000, 24);
			}
			if (write_byte_index == 0x300000) { /* 4. bank */
				ret=jtag_instruction(device_fp, ISC_ADDRESS_SHIFT);
				if ((ret&0xfff0) != 0x150) {printf("\nISC_ADDRESS_SHIFT.3 %02X\n", ret);}
				jtag_data(device_fp, 0x300000, 24);
			}
// end modification XCF32P
			
			jtag_instruction(device_fp, FPGM);
			write_byte_index  = write_byte_index + max_write_length ;
			rest_write_length = rest_write_length - max_write_length ;
			i=0; 
			jtag_tap_pause_1ms () ;
			printf("rest_write_length = %08X            %03d%%  \r",rest_write_length, ((100*write_byte_index)/((mcs_buffer_length + 0x200) & 0xfffe00)));

		} while (rest_write_length > 0);
		//} while (0);

		// write to prom is finished

		if (gl_debug==1) { printf("\n");   }
		ret=jtag_instruction(device_fp, ISC_ENABLE);
		if (gl_debug==1) { printf("ISC_ENABLE.0 %08X\n", ret);  } 
		ret=jtag_data(device_fp, 0x03, 8);
		if (gl_debug==1) { printf("jtag_data    %08X\n", ret); }  

		if (gl_debug==1) { printf("\n");   }
		ret=jtag_instruction(device_fp, XSC_DATA_SUCR);
		if (gl_debug==1) { printf("XSC_DATA_SUCR.0 %08X\n", ret);   }
		ret=jtag_data(device_fp, 0xfffc, 16);
		if (gl_debug==1) { printf("jtag_data       %08X\n", ret);   }

		if (gl_debug==1) { printf("\n");   }
		ret=jtag_instruction(device_fp, FPGM);
		if (gl_debug==1) { printf("FPGM  %08X\n", ret); }   

		jtag_tap_pause_1ms () ;

		if (gl_debug==1) { printf("\n");   }
		ret=jtag_instruction(device_fp, ISC_ENABLE);
		if (gl_debug==1) { printf("ISC_ENABLE.0 %08X\n", ret);  } 
		ret=jtag_data(device_fp, 0x03, 8);
		if (gl_debug==1) { printf("jtag_data    %08X\n", ret); }   

		if (gl_debug==1) { printf("\n");   }
		ret=jtag_instruction(device_fp, ISC_ENABLE);
		if (gl_debug==1) { printf("ISC_ENABLE.0 %08X\n", ret);   }
		ret=jtag_data(device_fp, 0x03, 8);
		if (gl_debug==1) { printf("jtag_data    %08X\n", ret);   }


		if (gl_debug==1) { printf("\n");   }
		ret=jtag_instruction(device_fp, XSC_DATA_CCB);
		if (gl_debug==1) { printf("XSC_DATA_CCB.0 %08X\n", ret);  } 
		//ret=jtag_data(device_fp, 0xffff, 16); // Serial Load
		ret=jtag_data(device_fp, 0xfff9, 16); // Paralel Load 
		if (gl_debug==1) { printf("jtag_data    %08X\n", ret);   }

		//ret=jtag_instruction(device_fp, ISC_NOOP);
		//printf("ISC_NOOP after XSC_DATA_CCB %08X\n", ret);   
	
		if (gl_debug==1) { printf("\n");   }
		ret=jtag_instruction(device_fp, FPGM);
		if (gl_debug==1) { printf("FPGM  %08X\n", ret);  } 

		jtag_tap_pause_1ms () ;

		if (gl_debug==1) { printf("\n");  }  
		ret=jtag_instruction(device_fp, ISC_ENABLE);
		if (gl_debug==1) { printf("ISC_ENABLE.0 %08X\n", ret);   }
		ret=jtag_data(device_fp, 0x03, 8);
		if (gl_debug==1) { printf("jtag_data    %08X\n", ret);   }


		if (gl_debug==1) { printf("\n");   }
		ret=jtag_instruction(device_fp, XSC_DATA_DONE);
		if (gl_debug==1) { printf("XSC_DATA_DONE    %08X\n", ret); }  


// begin modification XCF32P
		if (gl_prom_XCF32P == 1) {
			ret=jtag_data(device_fp, 0xc0, 32); // 
		}
		else {
		ret=jtag_data(device_fp, 0xcc, 8); //  
		}
// end modification XCF32P
		if (gl_debug==1) { printf("jtag_data    %08X\n", ret);   }


		ret=jtag_instruction(device_fp, FPGM);
		if (gl_debug==1) { printf("FPGM  %08X\n", ret);   }

		jtag_tap_pause_1ms () ;


		ret=jtag_instruction(device_fp, NORMRST);
		if (gl_debug==1) { printf("NORMRST.0 %08X\n", ret); }   

		jtag_tap_pause_1ms () ;

	
		ret=jtag_instruction(device_fp, NORMRST);
		if (gl_debug==1) { printf("NORMRST.0 %08X\n", ret);  } 

		jtag_tap_pause_1ms () ;


		ret=jtag_instruction(device_fp, BYPASS);
		if (gl_debug==1) { printf("BYPASS.0 %08X\n", ret);   }
		ret=jtag_data(device_fp, 0x0000, 1);
		if (gl_debug==1) { printf("jtag_data       %08X\n", ret); }   

	}

  

 
#define verifier   
#ifdef verifier   
/*	retcode=mcs_file_unpack (fname, gl_wbuf, mcs_buffer_length) ; */
  //yyyyyyyyyy 

/* --- read ISP PROM and compare with file-data-buffer */


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// read first 8 MBit
   jtag_instruction(device_fp, ISPEN);
  	jtag_tap_pause_1ms () ;

   jtag_data(device_fp, 0x34, 6);
   ret=jtag_instruction(device_fp, ISC_ADDRESS_SHIFT);
      jtag_tap_pause_1ms () ;
   if ((ret&0xfff0) != 0x150) printf("\nISC_ADDRESS_SHIFT.3 %02X\n\n", ret);
   jtag_data(device_fp, 0, 24);
   jtag_instruction(device_fp, XCS_READ);

// delay
	jtag_tap_pause_1ms () ;
	jtag_tap_pause_1ms () ;
   
	//printf("mcs_buffer_length %08X\n", mcs_buffer_length); 

	max_error_value = 0x10 ;
	//mcs_buffer_length = 0x200000 ;	

	if (mcs_buffer_length > 0x100000) {
		rest_read_length = 0x100000 ;	
		rest_mcs_buffer_length = mcs_buffer_length - 0x100000 ;
	}
	else {
		rest_read_length = mcs_buffer_length ;	
		rest_mcs_buffer_length = 0 ;
	}

	max_read_length = 0x1000 ;
	read_byte_index = 0 ;
	printf("\n\nRead data from prom \n");

	while (rest_read_length != 0) {	
		if (rest_read_length >= max_read_length) {
			read_length = max_read_length ;
		}
		else {
			read_length = rest_read_length ;
		}
		rest_read_length = rest_read_length - read_length ;

		if ((ret=jtag_rd_data(device_fp, &gl_rbuf[read_byte_index], read_length, 0)) != 0) return ret;
		read_byte_index = read_byte_index + read_length ;
		printf("rest_read_length1  = %08X            %03d%%  \r",rest_read_length+rest_mcs_buffer_length, ((100*read_byte_index)/mcs_buffer_length));
	}   

	jtag_rd_data(device_fp, 0, 0, 0) ;	
	jtag_instruction(device_fp, NORMRST);
	
	if (rest_mcs_buffer_length != 0) {

		//printf("mcs_buffer_length      %08X\n", mcs_buffer_length); 
		//printf("rest_mcs_buffer_length %08X\n", rest_mcs_buffer_length); 
		//printf("read_byte_index        %08X\n", read_byte_index); 

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	// read second 8 MBit
		jtag_instruction(device_fp, ISPEN);
		jtag_data(device_fp, 0x34, 6);
		ret=jtag_instruction(device_fp, ISC_ADDRESS_SHIFT);
		if ((ret&0xfff0) != 0x150) printf("\nISC_ADDRESS_SHIFT.3 %02X\n\n", ret);
		jtag_data(device_fp, 0x100000, 24);

		jtag_instruction(device_fp, XCS_READ);

		jtag_tap_pause_1ms () ;

  
// begin modification XCF32P
		//rest_read_length = rest_mcs_buffer_length ;	
		if (rest_mcs_buffer_length > 0x100000) {
			rest_read_length = 0x100000 ;	
			rest_mcs_buffer_length = rest_mcs_buffer_length - 0x100000 ;
		}
		else {
			rest_read_length = rest_mcs_buffer_length ;	
			rest_mcs_buffer_length = 0 ;
		}
// end modification XCF32P

		max_read_length = 0x1000 ;

		//printf("\n\nRead data from prom \n");

		while (rest_read_length != 0) {	
			if (rest_read_length >= max_read_length) {
				read_length = max_read_length ;
			}
			else {
				read_length = rest_read_length ;
			}
			rest_read_length = rest_read_length - read_length ;

			if ((ret=jtag_rd_data(device_fp, &gl_rbuf[read_byte_index], read_length, 0)) != 0) return ret;
			read_byte_index = read_byte_index + read_length ;
			printf("rest_read_length2  = %08X            %03d%%  \r",rest_read_length, ((100*read_byte_index)/mcs_buffer_length));

		}   
		jtag_rd_data(device_fp, 0, 0, 0) ;	
		jtag_instruction(device_fp, NORMRST);
	
	}   


	
// begin modification XCF32P
	
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	if (rest_mcs_buffer_length != 0) {

		//printf("mcs_buffer_length      %08X\n", mcs_buffer_length); 
		//printf("rest_mcs_buffer_length %08X\n", rest_mcs_buffer_length); 
		//printf("read_byte_index        %08X\n", read_byte_index); 


	// read 3. 8 MBit
		jtag_instruction(device_fp, ISPEN);
		jtag_data(device_fp, 0x34, 6);
		ret=jtag_instruction(device_fp, ISC_ADDRESS_SHIFT);
		if ((ret&0xfff0) != 0x150) printf("\nISC_ADDRESS_SHIFT.3 %02X\n\n", ret);
		jtag_data(device_fp, 0x200000, 24);

		jtag_instruction(device_fp, XCS_READ);

		jtag_tap_pause_1ms () ;

  
		//rest_read_length = rest_mcs_buffer_length ;	
		if (rest_mcs_buffer_length > 0x100000) {
			rest_read_length = 0x100000 ;	
			rest_mcs_buffer_length = rest_mcs_buffer_length - 0x100000 ;
		}
		else {
			rest_read_length = rest_mcs_buffer_length ;	
			rest_mcs_buffer_length = 0 ;
		}

		max_read_length = 0x1000 ;

		//printf("\n\nRead data from prom \n");

		while (rest_read_length != 0) {	
			if (rest_read_length >= max_read_length) {
				read_length = max_read_length ;
			}
			else {
				read_length = rest_read_length ;
			}
			rest_read_length = rest_read_length - read_length ;

			if ((ret=jtag_rd_data(device_fp, &gl_rbuf[read_byte_index], read_length, 0)) != 0) return ret;
			read_byte_index = read_byte_index + read_length ;
			printf("rest_read_length3  = %08X            %03d%%  \r",rest_read_length, ((100*read_byte_index)/mcs_buffer_length));

		}   
		jtag_rd_data(device_fp, 0, 0, 0) ;	
		jtag_instruction(device_fp, NORMRST);
	
	}   

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	if (rest_mcs_buffer_length != 0) {

		//printf("mcs_buffer_length      %08X\n", mcs_buffer_length); 
		//printf("rest_mcs_buffer_length %08X\n", rest_mcs_buffer_length); 
		//printf("read_byte_index        %08X\n", read_byte_index); 


	// read 4. 8 MBit
		jtag_instruction(device_fp, ISPEN);
		jtag_data(device_fp, 0x34, 6);
		ret=jtag_instruction(device_fp, ISC_ADDRESS_SHIFT);
		if ((ret&0xfff0) != 0x150) printf("\nISC_ADDRESS_SHIFT.3 %02X\n\n", ret);
		jtag_data(device_fp, 0x300000, 24);

		jtag_instruction(device_fp, XCS_READ);

		jtag_tap_pause_1ms () ;

  
		//rest_read_length = rest_mcs_buffer_length ;	
		if (rest_mcs_buffer_length > 0x100000) {
			rest_read_length = 0x100000 ;	
			rest_mcs_buffer_length = rest_mcs_buffer_length - 0x100000 ;
		}
		else {
			rest_read_length = rest_mcs_buffer_length ;	
			rest_mcs_buffer_length = 0 ;
		}

		max_read_length = 0x1000 ;

		//printf("\n\nRead data from prom \n");

		while (rest_read_length != 0) {	
			if (rest_read_length >= max_read_length) {
				read_length = max_read_length ;
			}
			else {
				read_length = rest_read_length ;
			}
			rest_read_length = rest_read_length - read_length ;

			if ((ret=jtag_rd_data(device_fp, &gl_rbuf[read_byte_index], read_length, 0)) != 0) return ret;
			read_byte_index = read_byte_index + read_length ;
			printf("rest_read_length4  = %08X            %03d%%  \r",rest_read_length, ((100*read_byte_index)/mcs_buffer_length));

		}   
		jtag_rd_data(device_fp, 0, 0, 0) ;	
		jtag_instruction(device_fp, NORMRST);
	
	}   


	// end modification XCF32P

	
	
	
	
	
	
	printf("\nRead finished \n");

	/* compare */

	//printf("\nVerifier started \n");
	printf("\nVerifier started  mcs_buffer_length=%08X \n", mcs_buffer_length);
	error_counter=0;
	for (i=0;i<mcs_buffer_length;i++) {
//printf("        i = %08X, prom = %02X  file = %02X\n",i, gl_rbuf[i], gl_wbuf[i]);
 	 	if(gl_rbuf[i] != gl_wbuf[i]) {
			printf("i = %08X, prom = %02X  file = %02X\n",i, gl_rbuf[i], gl_wbuf[i]);
			error_counter++ ;
		}
	//	if(error_counter >= max_error_value) {break; } 
		if(error_counter >= 100) {break; } 


 	 	//if((i > 0x2bdffe) && (i < 0x2be03f)) {
		//	printf("i = %08X, prom = %02X  file = %02X\n",i, gl_rbuf[i], gl_wbuf[i]);
		//}
	
	
	}



	
	if(error_counter == 0) { 
		//printf("\nVerifier OK \n\n");
		return 0 ;
	}
	else {
		//printf("\nVerifier failed !!!!!!!!!!!!!!!!!!!!!!!!!! \n\n");
		return -1 ;
	}
#endif


	return 0 ;



}






int jtag_erase (int device_fp)
{
	unsigned int addr ;
	unsigned int i, i_loop ;

	int      ret;
        int retcode=1;

	
#define erase
#ifdef erase
        printf("JTAG erase\n");
        system("date");
	ret=jtag_instruction(device_fp, ISC_ENABLE);
	if (gl_debug==1) { printf("ISC_ENABLE.0 %08X\n", ret); }  
	ret=jtag_data(device_fp, 0x03, 8);
	if (gl_debug==1) { printf("jtag_data    %08X\n", ret); }   

	ret=jtag_instruction(device_fp, XSC_DATA_RDPT);
	if (gl_debug==1) { printf("XSC_DATA_RDPT.0 %08X\n", ret); }   
	ret=jtag_data(device_fp, 0x0000, 16);
	if (gl_debug==1) { printf("jtag_data       %08X\n", ret); } 

	ret=jtag_instruction(device_fp, XSC_DATA_WRPT);
	if (gl_debug==1) { printf("XSC_DATA_WRPT.0 %08X\n", ret); } 
	ret=jtag_data(device_fp, 0x0000, 16);
	if (gl_debug==1) { printf("jtag_data       %08X\n", ret); }   

	ret=jtag_instruction(device_fp, IDCODE);
	if (gl_debug==1) { printf("IDCODE.0 %08X\n", ret);  } 
	ret=jtag_data(device_fp, 0x00000000, 32);
	if (gl_debug==1) { printf("jtag_data       %08X\n", ret); }  



	jtag_tap_reset (device_fp) ;

	if (gl_debug==1) { printf("\n");  } 
	ret=jtag_instruction(device_fp, ISPEN);
	if (gl_debug==1) { printf("ISPEN.0 %08X\n", ret);  } 
	ret=jtag_data(device_fp, 0xd0, 8); // 
	if (gl_debug==1) { printf("jtag_data       %08X\n", ret); }   

	ret=jtag_instruction(device_fp, XSC_UNLOCK);
	if (gl_debug==1) { printf("XSC_UNLOCK.0 %08X\n", ret); }  
	ret=jtag_data(device_fp, 0x00003f, 24); // 
	if (gl_debug==1) { printf("jtag_data       %08X\n", ret); }   

	ret=jtag_instruction(device_fp, FERASE);
	if (gl_debug==1) { printf("FERASE.0 %08X\n", ret);   }
	ret=jtag_data(device_fp, 0x3f, 24); // 
	if (gl_debug==1) { printf("jtag_data       %08X\n", ret);  } 

	//ret=jtag_instruction(device_fp, ISC_NOOP);
	//printf("ISC_NOOP after FERASE  %08X\n", ret);   
	// delay
	i=0; 

	jtag_erase_pause_30s() ;




	if (gl_debug==1) { ret=jtag_instruction(device_fp, ISC_NOOP); }
	if (gl_debug==1) { printf("ISC_NOOP after FERASE and DELAY %08X\n", ret); }   

	i_loop = 0 ;
	do {
		i_loop++;
		ret=jtag_instruction(device_fp, ISC_NOOP);
	} while ((ret&0x10) == 0) ; // until busy 

	ret=jtag_instruction(device_fp, ISC_NOOP);
	if (gl_debug==1) { printf("ISC_NOOP.4 %08X\n", ret);  } 
	if (gl_debug==1) { printf("i_loop  %08X\n", i_loop);  } 


	ret=jtag_instruction(device_fp, NORMRST);
	if (gl_debug==1) { printf("NORMRST.0 %08X\n", ret);   }

	jtag_tap_pause_1ms () ;


	ret=jtag_instruction(device_fp, ISC_NOOP);
	if (gl_debug==1) { printf("ISC_NOOP after NORMRST %08X\n", ret); }   

#endif


//#define read_blankcheck
#ifdef read_blankcheck
	printf("\n");   
	//ret=jtag_instruction(device_fp, ISC_NOOP);
	//printf("ISC_NOOP.0 %08X\n", ret);   


// check if prom is empty
	max_error_value = 10 ;
	blank_check_buffer_length = 0x4000 ;	// length of chack array of each bank
// check 1. part of 1. bank

// read first 
	jtag_instruction(device_fp, ISPEN);
	jtag_data(device_fp, 0x34, 6);
	ret=jtag_instruction(device_fp, ISC_ADDRESS_SHIFT);
	if (ret != 0x159) printf("ISC_ADDRESS_SHIFT.3 %02X\n", ret);
	jtag_data(device_fp, 0, 24);
	jtag_instruction(device_fp, XCS_READ);

	jtag_tap_pause_1ms () ;
   
	rest_read_length = blank_check_buffer_length ;	
	max_read_length = 0x1000 ;
	read_byte_index = 0 ;
	printf("\n\nRead data from prom \n");

	while (rest_read_length != 0) {	
		if (rest_read_length >= max_read_length) {
			read_length = max_read_length ;
		}
		else {
			read_length = rest_read_length ;
		}
		rest_read_length = rest_read_length - read_length ;

		if ((ret=jtag_rd_data(device_fp, &gl_rbuf[read_byte_index], read_length, 0)) != 0) return ret;
		read_byte_index = read_byte_index + read_length ;
		printf("rest_read_length  = %08X            %03d%%  \r",rest_read_length, ((100*read_byte_index)/blank_check_buffer_length));

	}   

	jtag_rd_data(device_fp, 0, 0, 0) ;	
	jtag_instruction(device_fp, NORMRST);

	printf("\nEmpty Check on 1. Bank started \n");
	error_counter=0;

	for (i=0;i<blank_check_buffer_length;i++) {
 	 	if(gl_rbuf[i] != 0xff) {
			printf("i = %08X, prom = %02X  must = 0xff\n",i, gl_rbuf[i]);
			error_counter++ ;
		}
		if(error_counter >= max_error_value) {
			printf("1. bank of prom is not empty\n");
			break; 
		} 
	}
	if (error_counter == 0) {
		printf("Empty Check on 1. Bank OK \n");
	}



// check 1. part of 2. bank

// read first 
	jtag_instruction(device_fp, ISPEN);
	jtag_data(device_fp, 0x34, 6);
	ret=jtag_instruction(device_fp, ISC_ADDRESS_SHIFT);
	if (ret != 0x159) printf("ISC_ADDRESS_SHIFT.3 %02X\n", ret);
	jtag_data(device_fp, 0x100000, 24);
	jtag_instruction(device_fp, XCS_READ);

	jtag_tap_pause_1ms () ;
   
	rest_read_length = blank_check_buffer_length ;	
	max_read_length = 0x1000 ;
	read_byte_index = 0 ;
	printf("\nEmpty Check on 2. Bank started \n");

	while (rest_read_length != 0) {	
		if (rest_read_length >= max_read_length) {
			read_length = max_read_length ;
		}
		else {
			read_length = rest_read_length ;
		}
		rest_read_length = rest_read_length - read_length ;

		if ((ret=jtag_rd_data(device_fp, &gl_rbuf[read_byte_index], read_length, 0)) != 0) return ret;
		read_byte_index = read_byte_index + read_length ;
		printf("rest_read_length  = %08X            %03d%%  \r",rest_read_length, ((100*read_byte_index)/blank_check_buffer_length));

	}   

	jtag_rd_data(device_fp, 0, 0, 0) ;	
	jtag_instruction(device_fp, NORMRST);

	//printf("\nVerifier started \n");
	printf("\nVerifier started  blank_check_buffer_length=%08X \n", blank_check_buffer_length);
	error_counter=0;

	for (i=0;i<blank_check_buffer_length;i++) {
 	 	if(gl_rbuf[i] != 0xff) {
			printf("i = %08X, prom = %02X  must = 0xff\n",i, gl_rbuf[i]);
			error_counter++ ;
		}
		if(error_counter >= max_error_value) {
			printf("2. bank of prom is not empty\n");
			break; 
		} 
	}
	if (error_counter == 0) {
		printf("Empty Check on 2. Bank OK \n");
	}


#endif
	ret=jtag_instruction(device_fp, BYPASS);
	if (gl_debug==1) { printf("BYPASS.0 %08X\n", ret);   }
	ret=jtag_data(device_fp, 0x0000, 1);
	if (gl_debug==1) { printf("jtag_data       %08X\n", ret); }  

	return 0 ;
}






int jtag_check_blank (int device_fp)
{
	int      ret;
    int retcode=0x33;



#ifdef raus   
	jtag_reset (device_fp) ;
	print_jtag_chain();

	gl_jtg_select = 0 ;
#endif

        printf("JTAG blank check\n");
        system("date");

	if (gl_debug==1) { printf("\n"); }  
	//ret=jtag_instruction(device_fp, ISC_NOOP);
	//printf("ISC_NOOP. before XSC_ENABLEC  %08X\n", ret);   

	ret=jtag_instruction(device_fp, ISC_ENABLE);
	if (gl_debug==1) { printf("ISC_ENABLE.0 %08X\n", ret); }   
	ret=jtag_data(device_fp, 0x03, 8);
	if (gl_debug==1) { printf("jtag_data    %08X\n", ret);  } 

	ret=jtag_instruction(device_fp, XSC_DATA_RDPT);
	if (gl_debug==1) { printf("XSC_DATA_RDPT.0 %08X\n", ret);}   
	ret=jtag_data(device_fp, 0x0000, 16);
	if (gl_debug==1) { printf("jtag_data       %08X\n", ret); }  

	ret=jtag_instruction(device_fp, XSC_DATA_WRPT);
	if (gl_debug==1) { printf("XSC_DATA_WRPT.0 %08X\n", ret);   }
	ret=jtag_data(device_fp, 0x0000, 16);
	if (gl_debug==1) { printf("jtag_data       %08X\n", ret);   }

	ret=jtag_instruction(device_fp, IDCODE);
	if (gl_debug==1) { printf("IDCODE.0 %08X\n", ret);   }
	ret=jtag_data(device_fp, 0x00000000, 32);
	if (gl_debug==1) { printf("jtag_data       %08X\n", ret); }   



	if (gl_debug==1) { printf("\n"); }   
	ret=jtag_instruction(device_fp, ISC_ENABLE);
	if (gl_debug==1) { printf("ISC_ENABLE.0 %08X\n", ret);   }
	ret=jtag_data(device_fp, 0x03, 8);
	if (gl_debug==1) { printf("jtag_data    %08X\n", ret);   }

	ret=jtag_instruction(device_fp, XSC_DATA_RDPT);
	if (gl_debug==1) { printf("XSC_DATA_RDPT.0 %08X\n", ret);   }
	ret=jtag_data(device_fp, 0x0000, 16);
	if (gl_debug==1) { printf("jtag_data       %08X\n", ret);   }

	ret=jtag_instruction(device_fp, XSC_DATA_WRPT);
	if (gl_debug==1) { printf("XSC_DATA_WRPT.0 %08X\n", ret);   }
	ret=jtag_data(device_fp, 0x0000, 16);
	if (gl_debug==1) { printf("jtag_data       %08X\n", ret);   }

	ret=jtag_instruction(device_fp, IDCODE);
	if (gl_debug==1) { printf("IDCODE.0 %08X\n", ret);   }
	ret=jtag_data(device_fp, 0x00000000, 32);
	if (gl_debug==1) { printf("jtag_data       %08X\n", ret); }  



	ret=jtag_instruction(device_fp, ISC_ENABLE);
	if (gl_debug==1) { printf("ISC_ENABLE.0 %08X\n", ret);  } 
	ret=jtag_data(device_fp, 0x03, 8);
	if (gl_debug==1) { printf("jtag_data    %08X\n", ret); }  

	ret=jtag_instruction(device_fp, XSC_CLEAR_STATUS);
	if (gl_debug==1) { printf("XSC_CLEAR_STATUS.0 %08X\n", ret);}   

	jtag_tap_pause_1ms () ;


	if (gl_debug==1) { printf("\n"); }  
	ret=jtag_instruction(device_fp, XSC_BLANK_CHECK);
	if (gl_debug==1) { printf("XSC_BLANK_CHECK.0 %08X\n", ret); }  

// run 500 000 clocks
	jtag_tap_pause_5s () ;

	ret=jtag_data(device_fp, 0x0000, 8);
	if (gl_debug==1) { printf("jtag_data       %08X\n", ret); }  

	retcode = ret & 0x33 ; // 0: empty

	ret=jtag_instruction(device_fp, XSC_CLEAR_STATUS);
	if (gl_debug==1) { printf("XSC_CLEAR_STATUS.0 %08X\n", ret);  } 

// run 50 clocks
	jtag_tap_pause_1ms () ;



	ret=jtag_instruction(device_fp, NORMRST);
	if (gl_debug==1) { printf("NORMRST.0 %08X\n", ret); }  

// run 50 clocks
	jtag_tap_pause_1ms () ;


	ret=jtag_instruction(device_fp, BYPASS);
	if (gl_debug==1) { printf("BYPASS.0 %08X\n", ret);   }


	ret=jtag_instruction(device_fp, BYPASS);
	if (gl_debug==1) { printf("BYPASS.0 %08X\n", ret);   }
	ret=jtag_data(device_fp, 0x0000, 1);
	if (gl_debug==1) { printf("jtag_data       %08X\n", ret);  } 

return retcode ;

}








int jtag_read (int device_fp, char* fname)
{
sis8300_reg myReg;
	int return_code ;
	unsigned int data ;
	unsigned int addr ;
	unsigned int i ;
	unsigned int mcs_buffer_length ;
	unsigned int rest_mcs_buffer_length ;

//	unsigned int write_byte_index  ;

	unsigned int rest_read_length ;
	unsigned int max_read_length  ;
	unsigned int read_length      ;
	unsigned int read_byte_index ;
	unsigned int max_error_value ;
	unsigned int error_counter ;
	int      ret;
    int retcode=1;


	jtag_reset (device_fp) ;
	print_jtag_chain();
	if (gl_jtg_nof_devices == 0)  { 
		printf("\n\nERROR:     \n\n\n");
		return -1 ;
	}


	//gl_jtg_select = 0 ;

	//for (i=0;i<0x200000;i++) {
	//	gl_wbuf[i] = 0xff ;	
	//}
	retcode=mcs_file_unpack (fname, gl_wbuf, &mcs_buffer_length) ;
	if (retcode != 0) {
		printf("\n\nERROR:  mcs-file unpack    \n\n\n");
		return -1 ;
	}

	//mcs_buffer_length = 0x200000 ;




 
/*--- ISP PROM programmieren */
 

  

 
#define verifier   
#ifdef verifier   
/*	retcode=mcs_file_unpack (fname, gl_wbuf, mcs_buffer_length) ; */
   

/* --- read ISP PROM and compare with file-data-buffer */

// read first 8 MBit
   jtag_instruction(device_fp, ISPEN);
   jtag_data(device_fp, 0x34, 6);
   ret=jtag_instruction(device_fp, ISC_ADDRESS_SHIFT);
   if (ret != 0x11) printf("ISC_ADDRESS_SHIFT.3 %02X\n", ret);
   jtag_data(device_fp, 0, 24);
   jtag_instruction(device_fp, XCS_READ);


// run 50 clocks
	jtag_tap_pause_1ms () ;
   
	//printf("mcs_buffer_length %08X\n", mcs_buffer_length); 

	max_error_value = 0x10 ;
	//mcs_buffer_length = 0x200000 ;	

	if (mcs_buffer_length > 0x100000) {
		rest_read_length = 0x100000 ;	
		rest_mcs_buffer_length = mcs_buffer_length - 0x100000 ;
	}
	else {
		rest_read_length = mcs_buffer_length ;	
		rest_mcs_buffer_length = 0 ;
	}

	max_read_length = 0x1000 ;
	read_byte_index = 0 ;
	printf("\n\nRead data from prom \n");

	while (rest_read_length != 0) {	
		if (rest_read_length >= max_read_length) {
			read_length = max_read_length ;
		}
		else {
			read_length = rest_read_length ;
		}
		rest_read_length = rest_read_length - read_length ;

		if ((ret=jtag_rd_data(device_fp, &gl_rbuf[read_byte_index], read_length, 0)) != 0) return ret;
		read_byte_index = read_byte_index + read_length ;
		printf("rest_read_length  = %08X            %03d%%  \r",rest_read_length+rest_mcs_buffer_length, ((100*read_byte_index)/mcs_buffer_length));

	}   

	jtag_rd_data(device_fp, 0, 0, 0) ;	
	jtag_instruction(device_fp, NORMRST);
	
	if (rest_mcs_buffer_length != 0) {

		//printf("mcs_buffer_length      %08X\n", mcs_buffer_length); 
		//printf("rest_mcs_buffer_length %08X\n", rest_mcs_buffer_length); 
		//printf("read_byte_index        %08X\n", read_byte_index); 


	// read second 8 MBit
		jtag_instruction(device_fp, ISPEN);
		jtag_data(device_fp, 0x34, 6);
		ret=jtag_instruction(device_fp, ISC_ADDRESS_SHIFT);
		if (ret != 0x11) printf("ISC_ADDRESS_SHIFT.3 %02X\n", ret);
		jtag_data(device_fp, 0x100000, 24);

		jtag_instruction(device_fp, XCS_READ);

#ifdef raus
		//Delay !!??
		i=0; 
		myReg.offset = SIS8300_XILINX_JTAG_REG;
		do {
		      ioctl(device_fp, SIS8300_REG_READ, &myReg);
		      data = myReg.data;
		      i++ ;
		} while (i < 100);   /* min  20 */
#endif
		jtag_tap_pause_1ms () ;
		
		

	
		//rest_read_length = mcs_buffer_length ;	
		rest_read_length = rest_mcs_buffer_length ;	
		max_read_length = 0x1000 ;

		//printf("\n\nRead data from prom \n");

		while (rest_read_length != 0) {	
			if (rest_read_length >= max_read_length) {
				read_length = max_read_length ;
			}
			else {
				read_length = rest_read_length ;
			}
			rest_read_length = rest_read_length - read_length ;

			if ((ret=jtag_rd_data(device_fp, &gl_rbuf[read_byte_index], read_length, 0)) != 0) return ret;
			read_byte_index = read_byte_index + read_length ;
			printf("rest_read_length  = %08X            %03d%%  \r",rest_read_length, ((100*read_byte_index)/mcs_buffer_length));

		}   
		jtag_rd_data(device_fp, 0, 0, 0) ;	
		jtag_instruction(device_fp, NORMRST);
	
	}   

	printf("\nRead finished \n");

	/* compare */

	printf("\nVerifier started  mcs_buffer_length=%08X \n", mcs_buffer_length);
	error_counter=0;
	for (i=0;i<mcs_buffer_length;i++) {
 	 	if(gl_rbuf[i] != gl_wbuf[i]) {
			printf("i = %08X, prom = %02X  file = %02X\n",i, gl_rbuf[i], gl_wbuf[i]);
			error_counter++ ;
		}
	//	if(error_counter >= max_error_value) {break; } 
		if(error_counter >= 100) {break; } 

	
 	 	if(i > 0x2bdffe) {
			printf("i = %08X, prom = %02X  file = %02X\n",i, gl_rbuf[i], gl_wbuf[i]);
		}
	
	
	
	}



	
	if(error_counter == 0) { 
		printf("\nVerifier OK \n\n");
		return 0 ;
	}
	else {
		printf("\nVerifier failed !!!!!!!!!!!!!!!!!!!!!!!!!! \n\n");
		return -1 ;
	}
#endif //verifier

} 

 
