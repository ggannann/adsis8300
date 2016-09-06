 
/***************************************************************************/
/*                                                                         */
/*  Filename: sis8300-test.c.cxx                                           */
/*                                                                         */
/*  Hardware: SIS8300_V2                                                   */
/*  Firmware: V_2402                                                       */
/*                                                                         */
/*  Funktion: adc sampling                                                 */
/*                                                                         */
/*  Autor:                TH                                               */
/*  date:                 10.01.2012                                       */
/*  last modification:    19.02.2013                                       */
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
/*  � 2013                                                                 */
/*                                                                         */
/***************************************************************************/
// 
//#define GNU_PLOT
#define ADC_WINDOW_XPOSITION				10
#define ADC_WINDOW_YPOSITION				10
#define ADC_WINDOW_XSIZE				1400
#define ADC_WINDOW_YSIZE				600

#define CERN_ROOT_PLOT

#ifdef GNU_PLOT
   #include <unistd.h>
   #include "gnuplot_i.hpp"
#endif
   
#ifdef CERN_ROOT_PLOT
   #include "TApplication.h"
   #include "TObject.h"
   #include "TH1.h" 
   #include "TH1D.h"
   #include "TH1F.h"
   #include "TH2D.h"
   #include "TGraph.h"
   #include "TMultiGraph.h"
   #include "TMath.h"
   #include "TCanvas.h"
    //#include "TRandom.h"
   //#include "TThread.h"
   #include <TSystem.h>
#include "TLatex.h"
#endif


#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <linux/fs.h>

#include <sys/ioctl.h>
//#include "../../driver/hllpciex4_defs.h"
//#include "../../driver/hllpciex4_ioctl.h"
#include "../../driver/sis8300_defs.h"
#include "../../driver/sis8300_reg.h"






#define SIS8300_MEMORY_MAX_BLOCK_LENGTH			0x1000000  // 256-bit blocks -> 8 x 32-bit  -> 16 x 16-bit


int SIS8300_ADC_SPI_Setup(int device,  unsigned int adc_device_no);
int SIS8300_ADC_SPI_Set_Test_mode(int device,  unsigned int adc_device_no, unsigned int testmode);

int SIS8300_AD9510_SPI_Setup(int device, unsigned int* ch_divider_configuration_array, unsigned int ad9510_synch_cmd);
int SIS8300_ADC_SPI_Read(int device,  unsigned int adc_device_no, unsigned int spi_addr, unsigned int* read_data);



//#include "/home/th/cern_root_source/root/include/TCanvas.h"
//#include <TROOT.h>
//#include <TH1.h>
//#include <TFile.h>




unsigned int gl_sample_length;
unsigned int gl_multievent_nof_events;

#define min_max

#ifdef min_max
unsigned short gl_adc_min[10], gl_adc_max[10], gl_adc_mean[10];
#endif

#define ADC_BUFFER_LENGTH 0x1000000 // 16 Msamples / 32MByte
//#define ADC_BUFFER_LENGTH 0x400000 // 4 Msamples / 8MByte

unsigned int gl_uint_adc_buffer[ADC_BUFFER_LENGTH]; // 32MByte	
unsigned short gl_ushort_adc1_buffer[ADC_BUFFER_LENGTH]; //  
unsigned short gl_ushort_adc2_buffer[ADC_BUFFER_LENGTH]; //  
unsigned short gl_ushort_adc3_buffer[ADC_BUFFER_LENGTH]; // 
unsigned short gl_ushort_adc4_buffer[ADC_BUFFER_LENGTH]; //  
unsigned short gl_ushort_adc5_buffer[ADC_BUFFER_LENGTH]; //  
unsigned short gl_ushort_adc6_buffer[ADC_BUFFER_LENGTH]; //  
unsigned short gl_ushort_adc7_buffer[ADC_BUFFER_LENGTH]; //  
unsigned short gl_ushort_adc8_buffer[ADC_BUFFER_LENGTH]; //  

unsigned int i_ch;
unsigned int max_nof_channels=8;

unsigned short* ushort_adc_buffer_ptr[8]; //  
unsigned short* ushort_adcA_buffer_ptr; //  
unsigned short* ushort_adcB_buffer_ptr; //  
   


#define MAXIF 10

//handling cmd line input	
	int numSlot=1;
	int numWrapSizeMode=9; // EVENT_CONF_PAGE_SIZE_256_WRAP
	int numSamples=256; // EVENT_CONF_PAGE_SIZE_256_WRAP
	int numEvents=10;
	int numPre = 64;

char gl_command[256];





int sis8300_ReadRegister(int device, unsigned int address, unsigned int* data)
{
    sis8300_reg myReg;
    myReg.offset = address;
    ioctl(device, SIS8300_REG_READ, &myReg);
    *data = myReg.data ;
    return 0;			 
}

int sis8300_WriteRegister(int device, unsigned int address, unsigned int data)
{
    sis8300_reg myReg;
    // enable ddr2 test write interface
    myReg.offset = address;
    myReg.data = data;
    ioctl(device, SIS8300_REG_WRITE, &myReg);  
    return 0;			 
}



int sis8300_ReadMemory(int device, unsigned int address, unsigned int len, unsigned int* buffer)
{
    sis8300_reg myReg;

    myReg.offset = 0x202;
    myReg.data = address;
    ioctl(device, SIS8300_REG_WRITE, &myReg);

    read(device, buffer, len);
    
    return 0;			 
}

   //myReg.offset = SIS8300_IDENTIFIER_VERSION_REG;
    //ioctl(fp, SIS8300_REG_READ, &myReg);


int sis8300_WriteMemory(int device, unsigned int address, unsigned int len, unsigned int* buffer)
{
    sis8300_reg myReg;

    // enable ddr2 test write interface
    myReg.offset = DDR2_ACCESS_CONTROL;
    myReg.data = (1<<DDR2_PCIE_TEST_ENABLE);
    ioctl(device, SIS8300_REG_WRITE, &myReg);  
    
    
    myReg.offset = 0x212;
    myReg.data = address;
    ioctl(device, SIS8300_REG_WRITE, &myReg);

    write(device, buffer, len);

    // disable ddr2 test write interface
    myReg.offset = DDR2_ACCESS_CONTROL;
    myReg.data = (0<<DDR2_PCIE_TEST_ENABLE);
    ioctl(device, SIS8300_REG_WRITE, &myReg);  

    return 0;			 
}

int dumpSamplesToFile( int channel, int loop, unsigned short *buffer, int len ){
  FILE *fp;
  char s[32];
  int i;
  
  sprintf( s, "samples_ch%d_loop%d.txt", channel, loop );
  
  fp = fopen( s, "w" );
  if( fp == NULL ){
     printf( "Error creating sample dump file\n" );
     return -1;
  }
    
  //fwrite( buffer, sizeof(unsigned short), len, fp );
  for(i = 0;i < len;i++) {
    fprintf(fp, "%d\r\n", buffer[i]);
  }
    
  fclose( fp );
  return 0;
}



int main(int argc, char **argv)
{

//main(int argc, char *argv[]){
    int i, k, s;
    int ymin, ymax ;
    int ywidth, y_delta ;


    //    unsigned short* ushort_adc_data_ptr ;
    
    int fp;	
    unsigned int i_adc, ui;
    unsigned int data;
    unsigned int dual_channel_memory_logic_flag;
    sis8300_reg myReg;

  char ch ;
   int  cmd_autozoom = 0 ;
   int  cmd_adc_no = -1 ;
   int  cmd_sample_block_length = -1 ;
   int  cmd_pre_trigger_delay = -1 ;
   int  cmd_sample_mode = -1 ;
   int  cmd_clock_source = -1 ;
   int cmd_dump_loops = -1;
   
   int  test_adc_no = -1 ;
   int  test_sample_block_length = -1 ;
   int  test_sample_length = -1 ;
   int  test_pre_trigger_delay = -1 ;
   int  test_sample_mode = -1 ;
   int  test_clock_source = -1 ;
   
   int dump_loop_counter = 0;
   
  unsigned int ad9510_divider_configuration_array[8]  ;

  ushort_adc_buffer_ptr[0] = gl_ushort_adc1_buffer;
  ushort_adc_buffer_ptr[1] = gl_ushort_adc2_buffer;
  ushort_adc_buffer_ptr[2] = gl_ushort_adc3_buffer;
  ushort_adc_buffer_ptr[3] = gl_ushort_adc4_buffer;
  ushort_adc_buffer_ptr[4] = gl_ushort_adc5_buffer;
  ushort_adc_buffer_ptr[5] = gl_ushort_adc6_buffer;
  ushort_adc_buffer_ptr[6] = gl_ushort_adc7_buffer;
  ushort_adc_buffer_ptr[7] = gl_ushort_adc8_buffer;
 
	
/********************************************************************************************************************/
// decode command and set configuration parameters
 
  /* Save command line into string "command" */
    memset(gl_command,0,sizeof(gl_command));
    // memset(startchoice,0,sizeof(startchoice));
    for (i=0;i<argc;i++) {
      strcat(gl_command,argv[i]);
      strcat(gl_command," ");
    }

    if(argc < 2){
	printf("usage: %s [device] \n", argv[0]);
	return -1;
    }


	fp = open(argv[1], O_RDWR);
	//fp = open("/dev/sis8300-0", O_RDWR);
	if(fp < 0){
		printf("can't open device\n");
		return -1;
	}
	
	
    printf("argc %d    \n", argc);
  
     // default	      
    cmd_autozoom              = 0 ;
    cmd_adc_no                = 0 ;
    cmd_sample_block_length   = 100 ;
    cmd_pre_trigger_delay     = 100 ;
    cmd_sample_mode           = 0 ;
    cmd_clock_source	      = 0 ;
    cmd_dump_loops = 0;
    
    
    if (argc > 2) {
	/* Save command line into string "command" */
	memset(gl_command,0,sizeof(gl_command));
	// memset(startchoice,0,sizeof(startchoice));
	for (i=2;i<argc;i++) {
	    strcat(gl_command,argv[i]);
	    strcat(gl_command," ");
	}
	printf("gl_command %s    \n", gl_command);

       

      //while ((ch = getopt(argc, argv, "b:M:?lha:g:p:s:S:")) != -1)
      while ((ch = getopt(argc, argv, "?lhZA:L:D:M:C:F:")) != -1)
	switch (ch) {
	  case 'Z':
	    cmd_autozoom = 1 ;
	    break;
	  case 'A':
	    sscanf(optarg,"%d",&cmd_adc_no) ;
	    break;
	  case 'L':
	    sscanf(optarg,"%d",&cmd_sample_block_length) ;
	    break;
	  case 'D':
	    sscanf(optarg,"%d",&cmd_pre_trigger_delay) ;
	    break;
	  case 'M':
	    sscanf(optarg,"%d",&cmd_sample_mode) ;
	    break;
	  case 'C':
	    sscanf(optarg,"%d",&cmd_clock_source) ;
	    break;
	  case 'F':
	    sscanf(optarg,"%d",&cmd_dump_loops) ;
	    break;
	  case '?':
	  case 'h':
	  default:
	    printf("Usage: %s device  [-?h] [-Z] [-A adc_no] [-L sample_length] [-D pre_trigger_delay] [-M sample_mode] [-C clock_source]", argv[0]);
	    //printf("[-g energyGap] [-p energyPeaking] [-s scaleFactor] [-S shiftOffset]\n");
	    printf("   \n");
	    printf("       -Z  	       Auto-Zoom \n");
	    printf("       -A num      Display Graph of ADC channel (default:0; 0=all channles, 1 to 8 = chx) \n");
	    printf("       -L num      Sample Block Length (default: num = 200, max: num = 32768) \n");
	    printf("       -D num      PreTrigger Delay (default: num = 100, max: num = 2046) \n");
	    printf("   \n");
	    printf("       -M 0        Soft-Start \n");
	    printf("       -M 1        Internal Trigger \n");
	    printf("       -M 2        External Trigger \n");
	    printf("   \n");
	    printf("       -C 0        Internal 250MHz Clock Oscillator \n");
	    printf("       -C 1        External SMA Clock \n");
	    printf("       -C 2        External HARLINK Clock \n");
	    printf("   \n");
	    printf("       -F x        Dump x loops to file \n");
	    printf("   \n");
	    printf("       -h          Print this message\n");
	    printf("   \n");
	    exit(1);
	  }  
    
      } // if (argc > 2)

      // chack parameters
      if ((cmd_adc_no >= 0) && (cmd_adc_no <= 10)) {
	test_adc_no = cmd_adc_no ;
      }
      else {
	 printf("parameter cmd_adc_no not valid:   %d  \n",cmd_adc_no);
	 test_adc_no = 0 ;
      }

      if ((cmd_sample_block_length >= 1) && (cmd_sample_block_length <= 0x100000)) {
	test_sample_length       = cmd_sample_block_length * 16 ;
	test_sample_block_length = cmd_sample_block_length ;
      }
      else {
	 printf("parameter cmd_sample_block_length not valid:   %d  \n",cmd_sample_block_length);
	 cmd_sample_block_length  = 0x100 ; 
	 test_sample_length       = cmd_sample_block_length * 16 ;
	 test_sample_block_length = cmd_sample_block_length ;
     }

      if ((cmd_pre_trigger_delay >= 1) && (cmd_pre_trigger_delay <= 2046)) {
	test_pre_trigger_delay = cmd_pre_trigger_delay ;
      }
      else {
	 printf("parameter cmd_pre_trigger_delay not valid:   %d  \n",cmd_pre_trigger_delay);
	 test_pre_trigger_delay = 100 ;
      }

      if ((cmd_sample_mode >= 0) && (cmd_sample_mode <= 2)) {
	test_sample_mode = cmd_sample_mode ;
      }
      else {
	 printf("parameter test_sample_mode not valid:   %d  \n",cmd_sample_mode);
	 test_sample_mode = 0 ;
      }
 
      if ((cmd_clock_source >= 0) && (cmd_clock_source <= 2)) {
	test_clock_source = cmd_clock_source ;
      }
      else {
	 printf("parameter cmd_clock_source not valid:   %d  \n",cmd_clock_source);
	 test_clock_source = 0 ;
      }

	  unsigned int uint_disable_even_channels_flag = 0 ;
	  if (test_sample_length > 4096) {
 		printf("\n\nsample_length > 4096 -> disable channel 2/4/6/8  \n\n");
		uint_disable_even_channels_flag = 1;
		usleep(1000000) ;
	 }

      printf("cmd_autozoom              = %d    \n", cmd_autozoom);
      printf("test_adc_no               = %d    \n", test_adc_no);
      printf("test_sample_block_length  = %d    \n", test_sample_block_length);
      printf("test_sample_length        = %d    \n", test_sample_length);
      printf("test_pre_trigger_delay    = %d    \n", test_pre_trigger_delay);
      printf("test_sample_mode          = %d    \n", test_sample_mode);
      printf("test_clock_source         = %d    \n", test_clock_source);
      printf(" \n");
	
	
	
	
#ifdef GNU_PLOT
      #define NPOINTS    256

      vector<double> vx;
      vector<double> vy;
    
      Gnuplot g1 = Gnuplot("lines");
#endif
   
    
#ifdef CERN_ROOT_PLOT
      //int CanvasBackgroundColor = 33 ; // light blue
      int CanvasBackgroundColor = 10 ; // white
      int Channel_1_Color  = 2 ; //  
      int Channel_2_Color  = 3 ; //  
      int Channel_3_Color  = 4 ; //  
      int Channel_4_Color  = 6 ; //  
      int Channel_5_Color  = 7 ; //  
      int Channel_6_Color  = 8 ; //  
      int Channel_7_Color  = 9 ; //  
      int Channel_8_Color  = 28 ; //  
      int Channel_9_Color  = 30 ; //  
      int Channel_10_Color = 50 ; //  


      Int_t n=0x20000 ;  
      int x[n] , y[n] ;

 
      // a global canvas for the drawing of histograms
      TApplication theApp("SIS8300 Test", &argc, argv);

      //TCanvas *c1 = new TCanvas("c1","SIS8300",800,400);
      TCanvas *c1 = new TCanvas("c1","SIS8300",ADC_WINDOW_XPOSITION, ADC_WINDOW_YPOSITION, ADC_WINDOW_XSIZE, ADC_WINDOW_YSIZE);
      c1->SetGrid();
      c1->SetFillColor(CanvasBackgroundColor);
      c1->Divide(1);
      c1->cd(1);

      TGraph *gr0 = new TGraph(n,x,y);  
      TGraph *gr1 = new TGraph(n,x,y);  
      TGraph *gr2 = new TGraph(n,x,y);  
      TGraph *gr3 = new TGraph(n,x,y);  
      TGraph *gr4 = new TGraph(n,x,y);  
      TGraph *gr5 = new TGraph(n,x,y);  
      TGraph *gr6 = new TGraph(n,x,y);  
      TGraph *gr7 = new TGraph(n,x,y);  
      TGraph *gr8 = new TGraph(n,x,y);  
 
      gr0->SetLineColor(CanvasBackgroundColor);
      gr1->SetLineColor(Channel_1_Color);
      gr2->SetLineColor(Channel_2_Color);
      gr3->SetLineColor(Channel_3_Color);
      gr4->SetLineColor(Channel_4_Color);
      gr5->SetLineColor(Channel_5_Color);
      gr6->SetLineColor(Channel_6_Color);
      gr7->SetLineColor(Channel_7_Color);
      gr8->SetLineColor(Channel_8_Color);

      
      TLatex *text_adc1 = new TLatex(10,10,"Ch 1");
      TLatex *text_adc2 = new TLatex(10,10,"Ch 2");
      TLatex *text_adc3 = new TLatex(10,10,"Ch 3");
      TLatex *text_adc4 = new TLatex(10,10,"Ch 4");
      TLatex *text_adc5 = new TLatex(10,10,"Ch 5");
      TLatex *text_adc6 = new TLatex(10,10,"Ch 6");
      TLatex *text_adc7 = new TLatex(10,10,"Ch 7");
      TLatex *text_adc8 = new TLatex(10,10,"Ch 8");

      text_adc1->SetTextColor(Channel_1_Color);
      text_adc2->SetTextColor(Channel_2_Color);
      text_adc3->SetTextColor(Channel_3_Color);
      text_adc4->SetTextColor(Channel_4_Color);
      text_adc5->SetTextColor(Channel_5_Color);
      text_adc6->SetTextColor(Channel_6_Color);
      text_adc7->SetTextColor(Channel_7_Color);
      text_adc8->SetTextColor(Channel_8_Color);
 
      
      c1->Update();
     usleep(10);
#endif



    
      sis8300_ReadRegister(fp, SIS8300_IDENTIFIER_VERSION_REG, &data);
      printf("SIS8300_IDENTIFIER_VERSION_REG = 0x%08X\n", data);

      sis8300_ReadRegister(fp, SIS8300_FIRMWARE_OPTIONS_REG, &data);
      printf("SIS8300_FIRMWARE_OPTIONS_REG    = 0x%08X\n", data);
    
      dual_channel_memory_logic_flag = 0 ;
      if ((data&0x4) == 0x4) {
	  dual_channel_memory_logic_flag = 1 ;
      }
    
      // Clock Distribution Multiplexer
      myReg.offset = SIS8300_CLOCK_DISTRIBUTION_MUX_REG;
	switch(test_clock_source){
	  case 0:
	    myReg.data = 0x0000F; // internal 250MHz clock oscillator
	    break;
	  case 1:
	    myReg.data = 0x00F0F; // external SMA clock
	    break;
	  case 2:
	    myReg.data = 0x00A0F; // external HARLINK clock
	    break;
	}
	ioctl(fp, SIS8300_REG_WRITE, &myReg);
	
// ADC
	for (i_adc=0;i_adc<5;i_adc++) {
	  SIS8300_ADC_SPI_Setup(fp, i_adc);
	}

// AD9510 Clock Distributuon IC

//ch_divider_configuration_array[i] :  
//	bits <3:0>:   Divider High
//	bits <7:4>:   Divider Low
//	bits <11:8>:  Phase Offset
//	bit  <12>:    Select Start High
//	bit  <13>:    Force 
//	bit  <14>:    Nosyn (individual) 
//	bit  <15>:    Bypass Divider 

//#define CLK_25MHZ //with Phase
#ifdef CLK_25MHZ
	ad9510_divider_configuration_array[0] = 0x0000 + 0x44 ;  // (ADC1-CLK, ch1/2)
	//ad9510_divider_configuration_array[1] = 0x0200 + 0x44 ;  // (ADC2-CLK, ch3/4) //phase: 2 clks
	ad9510_divider_configuration_array[1] = 0x0000 + 0x44 ;  // (ADC2-CLK, ch3/4)
	ad9510_divider_configuration_array[2] = 0x0000 + 0x44 ;  // (ADC3-CLK, ch4/5)
	ad9510_divider_configuration_array[3] = 0x0000 + 0x44 ;  // (ADC4-CLK, ch6/7)
	ad9510_divider_configuration_array[4] = 0x0000 + 0x44 ;  // (ADC5-CLK, ch8/9)
	ad9510_divider_configuration_array[5] = 0x0000 + 0x44 ;  // (Frontpanel Clk, Harlink)
	ad9510_divider_configuration_array[6] = 0x0000 + 0x44 ;  // (FPGA DIV-CLK05) used for synch. of external Triggers
#else // bypass
	ad9510_divider_configuration_array[0] = 0x0000 + 0x8000 ;  // (ADC1-CLK, ch1/2)
	ad9510_divider_configuration_array[1] = 0x0000 + 0x8000 ;  // (ADC2-CLK, ch3/4)
	ad9510_divider_configuration_array[2] = 0x0000 + 0x8000 ;  // (ADC3-CLK, ch4/5)
	ad9510_divider_configuration_array[3] = 0x0000 + 0x8000 ;  // (ADC4-CLK, ch6/7)
	ad9510_divider_configuration_array[4] = 0x0000 + 0x8000 ;  // (ADC5-CLK, ch8/9)
	ad9510_divider_configuration_array[5] = 0x0000 + 0x8000 ;  // (Frontpanel Clk, Harlink)
	ad9510_divider_configuration_array[6] = 0x0000 + 0x8000 ;  // (FPGA DIV-CLK05) used for synch. of external Triggers
#endif
	ad9510_divider_configuration_array[7] = 0xC000 + 0x00 ;  // (FPGA DIV-CLK69) used for sychn. of AD910 ISc and Bypass
	SIS8300_AD9510_SPI_Setup(fp, ad9510_divider_configuration_array, 1 /*ad9510_synch_cmd*/ );
	
      /***************/	
 	usleep(1) ;

	//SIS8300_Write_Register(fp, SIS8300_ADC_INPUT_TAP_DELAY_REG, 0x1f00 + 11);// 
	myReg.offset = SIS8300_ADC_INPUT_TAP_DELAY_REG;
	myReg.data  =0x1f00 + 11 ;  //  // Tap delay = 11 and all ADCs
	ioctl(fp, SIS8300_REG_WRITE, &myReg);  
	usleep(1) ;

	// disable ddr2 test write interface
	myReg.offset = DDR2_ACCESS_CONTROL;
	myReg.data = 0 ; // (1<<DDR2_PCIE_TEST_ENABLE);
	ioctl(fp, SIS8300_REG_WRITE, &myReg);  
      
      
        myReg.offset = SIS8300_PRETRIGGER_DELAY_REG;
	myReg.data = test_pre_trigger_delay;
	ioctl(fp, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;
	
	
 	myReg.offset = SIS8300_SAMPLE_LENGTH_REG;
	myReg.data = test_sample_block_length;
	ioctl(fp, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;

	myReg.data = 0x300; // enable ch1-8
	if (uint_disable_even_channels_flag == 1) {
		myReg.data = myReg.data + 0xAA; //disable ch2/4/6/8
	}

	myReg.offset = SIS8300_SAMPLE_CONTROL_REG;
	

    // if external trigger
	if (test_sample_mode == 2){
	    myReg.data |= 0x800; // enable external trigger
	}
    // if interal trigger
	if (test_sample_mode == 1){
	    myReg.data |= 0x400; // enable internal trigger
	}
	ioctl(fp, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;

	// external trigger setup
	if (test_sample_mode == 2){
	    myReg.offset = SIS8300_HARLINK_IN_OUT_CONTROL_REG ;
	    myReg.data = 0x100  ; // Enable Harlink 1 Input
	    ioctl(fp, SIS8300_REG_WRITE, &myReg);
	}
	
	// internal trigger setup
	if (test_sample_mode == 1){
	    myReg.data = 0x0  ; // 
	    myReg.data = myReg.data + 0x7000000  ; // Enable / GT / FIR
	  //  myReg.data = myReg.data + 0x6000000  ; // Enable / GT /
	    myReg.data = myReg.data + 0x100000  ; //Pulse Length = 0x10
	    myReg.data = myReg.data + 0x0f0F  ; // G = 15, P = 14 

	    myReg.offset = SIS8300_TRIGGER_SETUP_CH9_REG ;
	    ioctl(fp, SIS8300_REG_WRITE, &myReg);
	    myReg.offset = SIS8300_TRIGGER_SETUP_CH10_REG ;
	    ioctl(fp, SIS8300_REG_WRITE, &myReg);

	    //myReg.data = 0x1000110  ; // Threshold Off / On 
	    myReg.data = 0x500  ; // FIR Threshold  On 
	    //myReg.data = 32600  ; // FIR Threshold  On 
	    myReg.offset = SIS8300_TRIGGER_THRESHOLD_CH9_REG ;
	    ioctl(fp, SIS8300_REG_WRITE, &myReg);
	    myReg.offset = SIS8300_TRIGGER_THRESHOLD_CH10_REG ;
	    ioctl(fp, SIS8300_REG_WRITE, &myReg);
	}
	#define SIS8300_TRIGGER_SETUP_CH1_REG			0x100

#define SIS8300_TRIGGER_THRESHOLD_CH1_REG		0x110

	
	
	// reset sample logic
	sis8300_WriteRegister( fp, SIS8300_ACQUISITION_CONTROL_STATUS_REG, 0x00004 ); //  

	// Start sample Loop	 
	numSamples = test_sample_length ;
	do {

		sis8300_WriteRegister( fp, SIS8300_SAMPLE_START_ADDRESS_CH1_REG, 0x0 );      // 1. 1M-Block 16 Msamples
		sis8300_WriteRegister( fp, SIS8300_SAMPLE_START_ADDRESS_CH2_REG, 0x100000 ); // 2. 1M-Block 16 Msamples
		sis8300_WriteRegister( fp, SIS8300_SAMPLE_START_ADDRESS_CH3_REG, 0x200000 ); // 3. 1M-Block 16 Msamples
		sis8300_WriteRegister( fp, SIS8300_SAMPLE_START_ADDRESS_CH4_REG, 0x300000 ); // 4. 1M-Block 16 Msamples
		sis8300_WriteRegister( fp, SIS8300_SAMPLE_START_ADDRESS_CH5_REG, 0x400000 ); // 5. 1M-Block 16 Msamples
		sis8300_WriteRegister( fp, SIS8300_SAMPLE_START_ADDRESS_CH6_REG, 0x500000 ); // 6. 1M-Block 16 Msamples
		sis8300_WriteRegister( fp, SIS8300_SAMPLE_START_ADDRESS_CH7_REG, 0x600000 ); // 7. 1M-Block 16 Msamples
		sis8300_WriteRegister( fp, SIS8300_SAMPLE_START_ADDRESS_CH8_REG, 0x700000 ); // 8. 1M-Block 16 Msamples
	   // }

	 // start sampling
	    myReg.offset = SIS8300_ACQUISITION_CONTROL_STATUS_REG;
	    // TBD: mode 1 or 2
	    if(test_sample_mode == 0){
	      myReg.data = 0x00001  ; // start immediately
	    }else{
	      myReg.data = 0x00002  ; // armed, wait for trigger
	    }
	    ioctl(fp, SIS8300_REG_WRITE, &myReg);

	 // wait if BUSY or Arm
	    myReg.offset = SIS8300_ACQUISITION_CONTROL_STATUS_REG;
	    do {
	      gSystem->ProcessEvents();  // handle GUI events
	      ioctl(fp, SIS8300_REG_READ, &myReg);
	      printf("ctrl_reg = 0x%08X\n", myReg.data);
	    } while ((myReg.data & 0x3) != 0 ) ;

	    memset(gl_ushort_adc1_buffer, 0, numSamples*2);
	    memset(gl_ushort_adc2_buffer, 0, numSamples*2);
	    memset(gl_ushort_adc3_buffer, 0, numSamples*2);
	    memset(gl_ushort_adc4_buffer, 0, numSamples*2);
	    memset(gl_ushort_adc5_buffer, 0, numSamples*2);
	    memset(gl_ushort_adc6_buffer, 0, numSamples*2);
	    memset(gl_ushort_adc7_buffer, 0, numSamples*2);
	    memset(gl_ushort_adc8_buffer, 0, numSamples*2);

// read all ADC channels
			for (i_ch=0; i_ch<max_nof_channels; i_ch++) {
				sis8300_ReadMemory(fp, (i_ch) * (0x100000 * 16 * 2), numSamples*2, (uint32_t *) ushort_adc_buffer_ptr[i_ch]);
			}
   

	
  
#ifdef min_max
	    for(i = 0;i < max_nof_channels;i++){
	      gl_adc_min[i] = 0xFFFF;
	      gl_adc_max[i] = 0;
	      gl_adc_mean[i] = 0;
	      s = 0;
	    }
	    
	    // calc min/max mean
	    for(k = 0;k < max_nof_channels;k++){
	      for(i = 0;i < numSamples;i++){
		if(ushort_adc_buffer_ptr[k][i] < gl_adc_min[k]) gl_adc_min[k] = ushort_adc_buffer_ptr[k][i];
		if(ushort_adc_buffer_ptr[k][i] > gl_adc_max[k]) gl_adc_max[k] = ushort_adc_buffer_ptr[k][i];
		s = s + ushort_adc_buffer_ptr[k][i];
	      }
//	      gl_adc_mean[k] = TMath Mean(ushort_adc_buffer_ptr[k][0], ushort_adc_buffer_ptr[k][numSamples - 1]
	      gl_adc_mean[k] = s / numSamples;
	      s = 0;
	    }

	    //myReg.offset = SIS8300_SAMPLE_START_ADDRESS_CH1_REG;
	    //ioctl(fp, SIS8300_REG_READ, &myReg);
	    //printf("addr reg = 0x%08X\n", myReg.data);
	    
	    // all channels or just one?
	    
	    if(test_adc_no == 0){
	      i = 0;
	      k = 10;
	    }else{
	      i = test_adc_no - 1;
	      k = i + 1;
	    }
	      
	    for(;i < k;i++){
//	      printf("adc%2d min:%u max:%u ampl:%4u -3dB:%5.1f\n", i+1, gl_adc_min[i], gl_adc_max[i], gl_adc_max[i] - gl_adc_min[i], (gl_adc_max[i] - gl_adc_min[i]) * 0.7);
	      printf("adc%2d mean:%u meanoffs:%4d min:%3u max:%5u ampl:%5u -3dB:%5.1f\n", i+1, gl_adc_mean[i], gl_adc_mean[i]-32768, gl_adc_min[i], gl_adc_max[i], gl_adc_max[i] - gl_adc_min[i], (gl_adc_max[i] - gl_adc_min[i]) * 0.7);
	    }
	  
	    printf(" \n");
#endif

#ifdef CERN_ROOT_PLOT
	    c1->Clear();
	    c1->cd(1);

	  // check min and max for y-cordiante
	    ymin = 0 ;
	    ymax = 0x3fff ;

	    if (cmd_autozoom != 0) {
		ymax = 0 ;
		//ymin = 0xffff ; // 16 bit
		ymin = 0x3fff ; // 14 bit
		if ((test_adc_no == 0) || (test_adc_no == 1)) {
		  for(i=0; i<numSamples; i++){
		    if(gl_ushort_adc1_buffer[i] < ymin) ymin = gl_ushort_adc1_buffer[i];
		    if(gl_ushort_adc1_buffer[i] > ymax) ymax = gl_ushort_adc1_buffer[i];
		  }
		}


		if (((test_adc_no == 0) || (test_adc_no == 2)) && (uint_disable_even_channels_flag == 0)) {
		  for(i=0; i<numSamples; i++){
		    if(gl_ushort_adc2_buffer[i] < ymin) ymin = gl_ushort_adc2_buffer[i];
		    if(gl_ushort_adc2_buffer[i] > ymax) ymax = gl_ushort_adc2_buffer[i];
		  }
		}

		if ((test_adc_no == 0) || (test_adc_no == 3)) {
		  for(i=0; i<numSamples; i++){
		    if(gl_ushort_adc3_buffer[i] < ymin) ymin = gl_ushort_adc3_buffer[i];
		    if(gl_ushort_adc3_buffer[i] > ymax) ymax = gl_ushort_adc3_buffer[i];
		  }
		}

		if (((test_adc_no == 0) || (test_adc_no == 4))  && (uint_disable_even_channels_flag == 0)) {
		  for(i=0; i<numSamples; i++){
		    if(gl_ushort_adc4_buffer[i] < ymin) ymin = gl_ushort_adc4_buffer[i];
		    if(gl_ushort_adc4_buffer[i] > ymax) ymax = gl_ushort_adc4_buffer[i];
		  }
		}

		if ((test_adc_no == 0) || (test_adc_no == 5)) {
		  for(i=0; i<numSamples; i++){
		    if(gl_ushort_adc5_buffer[i] < ymin) ymin = gl_ushort_adc5_buffer[i];
		    if(gl_ushort_adc5_buffer[i] > ymax) ymax = gl_ushort_adc5_buffer[i];
		  }
		}

		if (((test_adc_no == 0) || (test_adc_no == 6))  && (uint_disable_even_channels_flag == 0)) {
		  for(i=0; i<numSamples; i++){
		    if(gl_ushort_adc6_buffer[i] < ymin) ymin = gl_ushort_adc6_buffer[i];
		    if(gl_ushort_adc6_buffer[i] > ymax) ymax = gl_ushort_adc6_buffer[i];
		  }
		}

		if ((test_adc_no == 0) || (test_adc_no == 7)) {
		  for(i=0; i<numSamples; i++){
		    if(gl_ushort_adc7_buffer[i] < ymin) ymin = gl_ushort_adc7_buffer[i];
		    if(gl_ushort_adc7_buffer[i] > ymax) ymax = gl_ushort_adc7_buffer[i];
		  }
		}

		if (((test_adc_no == 0) || (test_adc_no == 8))  && (uint_disable_even_channels_flag == 0)) {
		  for(i=0; i<numSamples; i++){
		    if(gl_ushort_adc8_buffer[i] < ymin) ymin = gl_ushort_adc8_buffer[i];
		    if(gl_ushort_adc8_buffer[i] > ymax) ymax = gl_ushort_adc8_buffer[i];
		  }
		}


	    }
	  printf("numSamples              = %d    \n", numSamples);
	  printf("cmd_autozoom              = %d    \n", cmd_autozoom);
	  printf("test_adc_no               = %d    \n", test_adc_no);
	  printf("ymin = %d  ymax = %d    \n", ymin, ymax);

	  // plot cordinates
	// initialze x-cordinates
	    for (i=0;i<numSamples;i++){
		x[i] = i ;
	    }

	    for(i = 0; i < numSamples; i++){
	      y[i]  = ymin+((ymax-ymin)/2)  ; 
	    }
	    y[(numSamples/2)] = ymin ;
	    y[((numSamples/2))+1] = ymax ;
	    gr0->DrawGraph(numSamples,x,y,"AL");  

	    ywidth = ymax - ymin ;
	    y_delta = ywidth/11 ;
	    
	  // plot channel 1
	    if ((test_adc_no == 0) || (test_adc_no == 1)) {
		for (i=0;i<numSamples;i++){
		    y[i] = (int) gl_ushort_adc1_buffer[i]; 
		}
		gr1->DrawGraph(numSamples,x,y, "L");   
		text_adc1->DrawLatex(numSamples+10,ymax-y_delta,"Ch 1");
	    }
		
	  // plot channel 2
	    if (((test_adc_no == 0) || (test_adc_no == 2)) && (uint_disable_even_channels_flag == 0)) {
		for (i=0; i<numSamples; i++){
		    y[i] = (int) gl_ushort_adc2_buffer[i]; 
		}
		gr2-> DrawGraph(numSamples,x,y, "L");   
		text_adc2->DrawLatex(numSamples+10,ymax-(2*y_delta),"Ch 2");
	    }

	  // plot channel 3
	    if ((test_adc_no == 0) || (test_adc_no == 3)) {
		for (i=0; i<numSamples; i++){
		    y[i] = (int) gl_ushort_adc3_buffer[i]; 
		}
		gr3-> DrawGraph(numSamples,x,y, "L");   
		text_adc3->DrawLatex(numSamples+10,ymax-(3*y_delta),"Ch 3");
	    }

	  // plot channel 4
	    if (((test_adc_no == 0) || (test_adc_no == 4))  && (uint_disable_even_channels_flag == 0)){
		for (i=0; i<numSamples; i++){
		    y[i] = (int) gl_ushort_adc4_buffer[i]; 
		}
		gr4-> DrawGraph(numSamples,x,y, "L");   
		text_adc4->DrawLatex(numSamples+10,ymax-(4*y_delta),"Ch 4");
	    }

	  // plot channel 5
	    if ((test_adc_no == 0) || (test_adc_no == 5)) {
		for (i=0; i<numSamples; i++){
		    y[i] = (int) gl_ushort_adc5_buffer[i]; 
		}
		gr5-> DrawGraph(numSamples,x,y, "L");   
		text_adc5->DrawLatex(numSamples+10,ymax-(5*y_delta),"Ch 5");
	    }

	  // plot channel 6
	    if (((test_adc_no == 0) || (test_adc_no == 6))  && (uint_disable_even_channels_flag == 0)) {
		for (i=0; i<numSamples; i++){
		    y[i] = (int) gl_ushort_adc6_buffer[i]; 
		}
		gr6-> DrawGraph(numSamples,x,y, "L");   
		text_adc6->DrawLatex(numSamples+10,ymax-(6*y_delta),"Ch 6");
	    }

	  // plot channel 7
	    if ((test_adc_no == 0) || (test_adc_no == 7)) {
		for (i=0; i<numSamples; i++){
		    y[i] = (int) gl_ushort_adc7_buffer[i]; 
		}
		gr7-> DrawGraph(numSamples,x,y, "L");   
		text_adc7->DrawLatex(numSamples+10,ymax-(7*y_delta),"Ch 7");
	    }

	  // plot channel 8
	    if (((test_adc_no == 0) || (test_adc_no == 8)) && (uint_disable_even_channels_flag == 0)) {
		for (i=0; i<numSamples; i++){
		    y[i] = (int) gl_ushort_adc8_buffer[i]; 
		}
		gr8-> DrawGraph(numSamples,x,y, "L");   
		text_adc8->DrawLatex(numSamples+10,ymax-(8*y_delta),"Ch 8");
	    }

	      c1->Update();
	      gSystem->ProcessEvents();  // handle GUI events

#endif	    
	      
	// dump to file
	if( cmd_dump_loops ){
	  if( dump_loop_counter < cmd_dump_loops ){

	    if ((test_adc_no == 0) || (test_adc_no == 1)) dumpSamplesToFile( 1, dump_loop_counter, gl_ushort_adc1_buffer, numSamples );
	    if (((test_adc_no == 0) || (test_adc_no == 2)) && (uint_disable_even_channels_flag == 0)) dumpSamplesToFile( 2, dump_loop_counter, gl_ushort_adc2_buffer, numSamples );
	    if ((test_adc_no == 0) || (test_adc_no == 3)) dumpSamplesToFile( 3, dump_loop_counter, gl_ushort_adc3_buffer, numSamples );
	    if (((test_adc_no == 0) || (test_adc_no == 4))  && (uint_disable_even_channels_flag == 0))dumpSamplesToFile( 4, dump_loop_counter, gl_ushort_adc4_buffer, numSamples );
	    if ((test_adc_no == 0) || (test_adc_no == 5)) dumpSamplesToFile( 5, dump_loop_counter, gl_ushort_adc5_buffer, numSamples );
	    if (((test_adc_no == 0) || (test_adc_no == 6))  && (uint_disable_even_channels_flag == 0))dumpSamplesToFile( 6, dump_loop_counter, gl_ushort_adc6_buffer, numSamples );
	    if ((test_adc_no == 0) || (test_adc_no == 7)) dumpSamplesToFile( 7, dump_loop_counter, gl_ushort_adc7_buffer, numSamples );
	    if (((test_adc_no == 0) || (test_adc_no == 8))  && (uint_disable_even_channels_flag == 0))dumpSamplesToFile( 8, dump_loop_counter, gl_ushort_adc8_buffer, numSamples );
	    
	    dump_loop_counter++;
	  }
	}
 
	  usleep(1000000) ;
	    gSystem->ProcessEvents();  // handle GUI events
	
	} while (1);	
	

	
	usleep(1) ;
	close(fp);

}



/****************************************************************************************************************************************/
/****************************************************************************************************************************************/
/****************************************************************************************************************************************/

	// Out 4  (FPGA DIV-CLK05) : LVDS 3.5 mA
	// Out 5  (ADC3-CLK, ch4/5) : LVDS 3.5 mA
	// Out 6  (ADC2-CLK, ch3/4) : LVDS 3.5 mA
	// Out 7  (ADC1-CLK, ch1/2) : LVDS 3.5 mA
	
	// Out 4  (FPGA DIV-CLK69) : LVDS 3.5 mA
	// Out 5  (ADC5-CLK, ch8/9) : LVDS 3.5 mA
	// Out 6  (ADC4-CLK, ch6/7) : LVDS 3.5 mA
	// Out 7  (Frontpanel Clk, Harlink) : LVDS 3.5 mA

//ch_divider_configuration_array[i] :  
//	bits <3:0>:   Divider High
//	bits <7:4>:   Divider Low
//	bits <11:8>:  Phase Offset
//	bit  <12>:    Select Start High
//	bit  <13>:    Force 
//	bit  <14>:    Nosyn (individual) 
//	bit  <15>:    Bypass Divider 
//
//	i=0:	AD910 #1 Out 7  (ADC1-CLK, ch1/2)
//	i=1:	AD910 #1 Out 6  (ADC2-CLK, ch3/4)
//	i=2:	AD910 #1 Out 5  (ADC3-CLK, ch4/5)
//	i=3:	AD910 #2 Out 6  (ADC4-CLK, ch6/7)
//	i=4:	AD910 #2 Out 5  (ADC5-CLK, ch8/9)
//	i=5:	AD910 #2 Out 7  (Frontpanel Clk, Harlink)
//	i=6:	AD910 #1 Out 4  (FPGA DIV-CLK05) used for synch. of external Triggers
//	i=7:	AD910 #2 Out 4  (FPGA DIV-CLK69) used for sychn. of AD910 ISc


int SIS8300_AD9510_SPI_Setup(int device, unsigned int* ch_divider_configuration_array, unsigned int ad9510_synch_cmd )
{
	sis8300_reg myReg;

//	myReg.data = 0xAADD;
//      AA: address
// 	DD: data

 
// AD9510 (ADC channels 1 to 6)
	// set AD9510 to Bidirectional Mode and Soft Reset 
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + 0x00B0;
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;

	// set AD9510 to Bidirectional Mode  
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + 0x0090;
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;

	// default: Asychrnon PowerDown, no Prescaler
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + 0x0A01;
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;
	
	// Out 0 (not used) : total Power-Down
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + 0x3c0B;
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;
	
	// Out 1 (not used) : total Power-Down
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + 0x3D0B;
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;
	
	// Out 2 (not used) : total Power-Down
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + 0x3E0B;
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;
	
	// Out 3 (not used) : total Power-Down
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + 0x3F0B;
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;


	// Out 4  (FPGA DIV-CLK05) : LVDS 3.5 mA
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + 0x4002; 
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;

	// Out 5  (ADC3-CLK, ch4/5) : LVDS 3.5 mA
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + 0x4102; 
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;

	// Out 6  (ADC2-CLK, ch3/4) : LVDS 3.5 mA
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + 0x4202; 
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;

	// Out 7  (ADC1-CLK, ch1/2) : LVDS 3.5 mA
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + 0x4302; 
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;


	
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + 0x4500; // addr
	//myReg.data = myReg.data + 0x1D ;
	myReg.data = myReg.data + 0x10 ; // Power-Down RefIn
	myReg.data = myReg.data + 0x08 ; // Shut Down Clk to PLL Prescaler
	myReg.data = myReg.data + 0x04 ; // Power-Down CLK2
	myReg.data = myReg.data + 0x01 ; // CLK1 Drives Distribution Section
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;
	




// Out 4  (FPGA DIV-CLK05) : Divider Low/High
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + 0x5000; // addr
	myReg.data = myReg.data + (ch_divider_configuration_array[6] & 0xff) ; //  
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;
	
	// Out 4  (FPGA DIV-CLK05) : Bypasse Diver (7), No Sychn (6), Force Individual Start (5), Start High (4), Phase Offset (3:0)
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + 0x5100; // addr
	myReg.data = myReg.data + ((ch_divider_configuration_array[6] >> 8) & 0xff)  ; //  
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;
	
	// Out 5  (ADC3-CLK, ch4/5) : Divider Low/High
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + 0x5200; // addr
	myReg.data = myReg.data + (ch_divider_configuration_array[2] & 0xff) ; //  
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;
	
	// Out 5  (ADC3-CLK, ch4/5) : Bypasse Diver (7), No Sychn (6), Force Individual Start (5), Start High (4), Phase Offset (3:0)
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + 0x5300; // addr
	myReg.data = myReg.data + ((ch_divider_configuration_array[2] >> 8) & 0xff)  ; //  
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;
	
	// Out 6  (ADC2-CLK, ch2/3) : Divider Low/High
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + 0x5400; // addr
	myReg.data = myReg.data + (ch_divider_configuration_array[1] & 0xff) ; //  
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;
	
	// Out 6  (ADC2-CLK, ch2/3) : Bypasse Diver (7), No Sychn (6), Force Individual Start (5), Start High (4), Phase Offset (3:0)
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + 0x5500; // addr
	myReg.data = myReg.data + ((ch_divider_configuration_array[1] >> 8) & 0xff)  ; //  
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;
	
	// Out 7  (ADC1-CLK, ch1/2) : Divider Low/High
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + 0x5600; // addr
	myReg.data = myReg.data + (ch_divider_configuration_array[0] & 0xff) ; //  
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;
	
	// Out 7  (ADC1-CLK, ch1/2) : Bypasse Diver (7), No Sychn (6), Force Individual Start (5), Start High (4), Phase Offset (3:0)
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + 0x5700; // addr
	myReg.data = myReg.data + ((ch_divider_configuration_array[0] >> 8) & 0xff)  ; //  
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;

	// update command
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + 0x5A01;
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;
	
	
// AD9510 (ADC channels 7 to 10)
	// set AD9510 to Bidirectional Mode and Soft Reset 
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x00B0;
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;

	// set AD9510 to Bidirectional Mode 
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x0090;
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;

	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x0A01;
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;
	
	// Out 0 (not used) : total Power-Down
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x3c0B;
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;

	
	// Out 1 (not used) : total Power-Down
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x3D0B;
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;
	
	// Out 2 (not used) : total Power-Down
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x3E0B;
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;
	
	// Out 3 (not used) : total Power-Down
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x3F0B;
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;


	// Out 4  (FPGA DIV-CLK69) : LVDS 3.5 mA
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x4002; 
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;

	// Out 5  (ADC5-CLK, ch8/9) : LVDS 3.5 mA
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x4102; 
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;

	// Out 6  (ADC4-CLK, ch6/7) : LVDS 3.5 mA
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x4202; 
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;

	// Out 7  (Frontpanel Clk, Harlink) : LVDS 3.5 mA
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x4302; // on 
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;

	


	
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x4500; // addr
	myReg.data = myReg.data + 0x10 ; // Power-Down RefIn
	myReg.data = myReg.data + 0x08 ; // Shut Down Clk to PLL Prescaler
	myReg.data = myReg.data + 0x04 ; // Power-Down CLK2
	myReg.data = myReg.data + 0x01 ; // CLK1 Drives Distribution Section
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;
	
	// Out 4  (FPGA DIV-CLK69) : Divider Low/High
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x5000; // addr
	myReg.data = myReg.data + (ch_divider_configuration_array[7] & 0xff) ; //  
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;
	
	// Out 4  (FPGA DIV-CLK69) : Bypasse Diver (7), No Sychn (6), Force Individual Start (5), Start High (4), Phase Offset (3:0)
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x5100; // addr
	myReg.data = myReg.data + ((ch_divider_configuration_array[7] >> 8) & 0xff)  ; //  
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;
	
	// Out 5  (ADC5-CLK, ch8/9) : Divider Low/High
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x5200; // addr
	myReg.data = myReg.data + (ch_divider_configuration_array[4] & 0xff) ; //  
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;
	
	// Out 5  (ADC5-CLK, ch8/9) : Bypasse Diver (7), No Sychn (6), Force Individual Start (5), Start High (4), Phase Offset (3:0)
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x5300; // addr
	myReg.data = myReg.data + ((ch_divider_configuration_array[4] >> 8) & 0xff)  ; //  
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;


	// Out 6  (ADC4-CLK, ch6/7) : Divider Low/High
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x5400; // addr
	myReg.data = myReg.data + (ch_divider_configuration_array[3] & 0xff) ; //  
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;
	
	// Out 6  (ADC4-CLK, ch6/7) : Bypasse Diver (7), No Sychn (6), Force Individual Start (5), Start High (4), Phase Offset (3:0)
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x5500; // addr
	myReg.data = myReg.data + ((ch_divider_configuration_array[3] >> 8) & 0xff)  ; //  
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;
	
	// Out 7  (Frontpanel Clk, Harlink) : Divider Low/High
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x5600; // addr
	myReg.data = myReg.data + (ch_divider_configuration_array[5] & 0xff) ; //  
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;
	
	// Out 7  (Frontpanel Clk, Harlink) : Bypasse Diver (7), No Sychn (6), Force Individual Start (5), Start High (4), Phase Offset (3:0)
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x5700;// addr
	myReg.data = myReg.data + ((ch_divider_configuration_array[5] >> 8) & 0xff)  ; //  
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;

	// update command
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x5A01;
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;



	// synch Cmd	
	if (ad9510_synch_cmd == 1) {
	    // set Function of "Function pin" to SYNCB (Default Reset)
	    myReg.offset = SIS8300_AD9510_SPI_REG;
	    myReg.data =  AD9510_GENERATE_SPI_RW_CMD + 0x5822;
	    ioctl(device, SIS8300_REG_WRITE, &myReg);
	    usleep(1) ;
	    // update command
	    myReg.offset = SIS8300_AD9510_SPI_REG;
	    myReg.data =   AD9510_GENERATE_SPI_RW_CMD + 0x5A01;
	    ioctl(device, SIS8300_REG_WRITE, &myReg);
	    usleep(1) ;

	    
	    // set Function of "Function pin" to SYNCB (Default Reset)
	    myReg.offset = SIS8300_AD9510_SPI_REG;
	    myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x5822;
	    ioctl(device, SIS8300_REG_WRITE, &myReg);
	    usleep(1) ;
	    // update command
	    myReg.offset = SIS8300_AD9510_SPI_REG;
	    myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x5A01;
	    ioctl(device, SIS8300_REG_WRITE, &myReg);
	    usleep(1) ;

	    // set use "FPGA DIV-CLK69" for Sychn Pulse  - Logic (VIRTEX5) 
	    myReg.offset = SIS8300_AD9510_SPI_REG;
	    myReg.data = AD9510_SPI_SET_FUNCTION_SYNCH_FPGA_CLK69  ; //set clock
	    ioctl(device, SIS8300_REG_WRITE, &myReg);
	    usleep(1) ;

	    // generate sych Pulse   (pulse Function pins) and hold FPGA_CLK69 to sycnh pulse
	    myReg.offset = SIS8300_AD9510_SPI_REG;
	    myReg.data = AD9510_GENERATE_FUNCTION_PULSE_CMD + AD9510_SPI_SET_FUNCTION_SYNCH_FPGA_CLK69   ; // 
	    ioctl(device, SIS8300_REG_WRITE, &myReg);
	    usleep(1) ;
	    
	}
	
	return 0;			 
}


/******************************************************************************/
/*                                                                            */ 
/*     SIS8300_ADC_SPI_Setup                                                  */ 
/*                                                                            */ 
/******************************************************************************/
/*                                                                            */ 
/* input:                                                                     */ 
/* adc_device_no: 0 to 4                                                      */ 
/*                                                                            */ 
/* return_codes:	                                                      */
/* 0:   	OK		                                              */
/* 0x800    FSM is BUSY                                                       */
/*                                                                            */ 
/******************************************************************************/


int SIS8300_ADC_SPI_Setup(int device,  unsigned int adc_device_no)
{
	unsigned int uint_adc_mux_select;
	unsigned int addr,data;
	sis8300_reg myReg;
	
	if (adc_device_no > 4) {return -1;}
	uint_adc_mux_select = adc_device_no << 24 ;
	
	myReg.offset = SIS8300_ADC_SPI_REG;

	// output type LVDS
	addr = (0x14 & 0xffff) << 8 ; 
	data = (0x40 & 0xff)  ; 
	myReg.data = uint_adc_mux_select + addr + data;
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;
	
	addr = (0x16 & 0xffff) << 8 ; 
	data = (0x00 & 0xff)  ; 
	myReg.data = uint_adc_mux_select + addr + data;
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;
	
	addr = (0x17 & 0xffff) << 8 ; 
	data = (0x00 & 0xff)  ; 
	myReg.data = uint_adc_mux_select + addr + data;
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;
	
	// register update cmd
	addr = (0xff & 0xffff) << 8 ; 
	data = (0x01 & 0xff)  ; 
	myReg.data = uint_adc_mux_select + addr + data;
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;
	

	return 0;			 
}


int SIS8300_ADC_SPI_Set_Test_mode(int device,  unsigned int adc_device_no, unsigned int testmode)
{
	unsigned int uint_adc_mux_select;
	unsigned int addr,data;
	sis8300_reg myReg;
	
	if (adc_device_no > 4) {return -1;}
	if (testmode > 7) {testmode=0;}
	uint_adc_mux_select = adc_device_no << 24 ;
	
	myReg.offset = SIS8300_ADC_SPI_REG;

 	
	addr = (0x0d & 0xffff) << 8 ; 
	data = (testmode & 0xff)  ; 
	myReg.data = uint_adc_mux_select + addr + data;
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;
	
	// register update cmd
	addr = (0xff & 0xffff) << 8 ; 
	data = (0x01 & 0xff)  ; 
	myReg.data = uint_adc_mux_select + addr + data;
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1) ;
	

	return 0;			 
}


int SIS8300_ADC_SPI_Read(int device,  unsigned int adc_device_no, unsigned int spi_addr, unsigned int* read_data)
{
	unsigned int uint_adc_mux_select;
	unsigned int data;
	unsigned int pollcounter;
	pollcounter = 1000;
	if (adc_device_no > 4) {return -1;}
	uint_adc_mux_select = adc_device_no << 24 ;

    data = uint_adc_mux_select + 0x800000 +  ((spi_addr & 0xffff) << 8) + (0x00 & 0xff)  ; 
    sis8300_WriteRegister(device,  SIS8300_ADC_SPI_REG,  data) ;


	do {
		sis8300_ReadRegister(device,  SIS8300_ADC_SPI_REG, &data) ;		
	     //printf("SIS8300_Read_Register  = 0x%08x  pollcounter = %d\n", data, pollcounter);
		pollcounter--;
	} while (((data & 0x80000000) == 0x80000000) && (pollcounter > 0));
	//     printf("SIS8300_Read_Register  = 0x%08x  pollcounter = %d\n", data, pollcounter);
	if (pollcounter == 0) {return -2;}
	sis8300_ReadRegister(device,  SIS8300_ADC_SPI_REG, &data) ;		
	*read_data = data & 0xff ;
	return 0;			 
}
