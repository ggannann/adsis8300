

//
// sis8300-test.c
// th Struck Innovative Systeme GmbH
// last rev. 01.10.2010 th
//
//#define GNU_PLOT

#include <TSystem.h>

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
//int SIS8300_AD9510_SPI_Setup(int device);


int SIS8300_Read_Register(int device,  unsigned int reg_addr, unsigned int* read_data)
{
    sis8300_reg myReg;
    myReg.offset = reg_addr;
    ioctl(device, SIS8300_REG_READ, &myReg);
    //printf("Version = 0x%08X\n", myReg.data);
    *read_data = myReg.data ;
    return 0;
}


int SIS8300_Write_Register(int device,  unsigned int reg_addr, unsigned int  write_data)
{
    unsigned int uint_adc_mux_select;
    unsigned int addr,data;
    sis8300_reg myReg;
    myReg.offset = reg_addr;
    myReg.data = write_data;
    ioctl(device, SIS8300_REG_WRITE, &myReg);
    return 0;
}




/****************************************************************************************************************************************/
/****************************************************************************************************************************************/
/****************************************************************************************************************************************/


int SIS8300_AD9510_SPI_Setup(int device, unsigned int* ch_divider_configuration_array, unsigned int ad9510_synch_cmd )
{
    sis8300_reg myReg;
    unsigned int sis8300_flag ;
    sis8300_flag =  0 ;

    myReg.offset = SIS8300_IDENTIFIER_VERSION_REG;
    ioctl(device, SIS8300_REG_READ, &myReg);
    printf("Version = 0x%08X\n", myReg.data);
    if ((myReg.data & 0x83000000) == 0x83000000)
    {
        sis8300_flag = 1 ; // else SIS8300-L (8301)
    }

// AD9510 (ADC channels 1 to 6)
    // Soft Reset
    if(sis8300_flag == 1)
    {
        myReg.data = AD9510_GENERATE_SPI_RW_CMD + 0x00B0; //Bidirectional Mode
    }
    else
    {
        myReg.data = AD9510_GENERATE_SPI_RW_CMD + 0x0030;  // not Bidirectional Mode
    }
    myReg.offset = SIS8300_AD9510_SPI_REG;
    ioctl(device, SIS8300_REG_WRITE, &myReg);
    usleep(1) ;

    if(sis8300_flag == 1)
    {
        myReg.data = AD9510_GENERATE_SPI_RW_CMD + 0x0090; //Bidirectional Mode
    }
    else
    {
        myReg.data = AD9510_GENERATE_SPI_RW_CMD + 0x0010;  // not Bidirectional Mode
    }
    myReg.offset = SIS8300_AD9510_SPI_REG;
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


#ifdef CLK2
    printf("AD9510 No.1: Use CLK_2!\n");
    myReg.offset = SIS8300_AD9510_SPI_REG;
    myReg.data = AD9510_GENERATE_SPI_RW_CMD + 0x4500; // addr
    myReg.data = myReg.data + 0x10 ; // Power-Down RefIn
    myReg.data = myReg.data + 0x08 ; // Shut Down Clk to PLL Prescaler
    myReg.data = myReg.data + 0x02 ; // Power-Down CLK1
    myReg.data = myReg.data + 0x00 ; // CLK2 Drives Distribution Section
    ioctl(device, SIS8300_REG_WRITE, &myReg);
    usleep(1) ;
#else
    printf("AD9510 No.1: Use CLK_1!\n");
    myReg.offset = SIS8300_AD9510_SPI_REG;
    myReg.data = AD9510_GENERATE_SPI_RW_CMD + 0x4500; // addr
    myReg.data = myReg.data + 0x10 ; // Power-Down RefIn
    myReg.data = myReg.data + 0x08 ; // Shut Down Clk to PLL Prescaler
    myReg.data = myReg.data + 0x04 ; // Power-Down CLK2
    myReg.data = myReg.data + 0x01 ; // CLK1 Drives Distribution Section
    ioctl(device, SIS8300_REG_WRITE, &myReg);
    usleep(1) ;
#endif // CLK2




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
    // Soft Reset
    if(sis8300_flag == 1)
    {
        myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x00B0; //Bidirectional Mode
    }
    else
    {
        myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x0030;  // not Bidirectional Mode
    }
    myReg.offset = SIS8300_AD9510_SPI_REG;
    ioctl(device, SIS8300_REG_WRITE, &myReg);
    usleep(1) ;


    if(sis8300_flag == 1)
    {
        myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x0090; //Bidirectional Mode
    }
    else
    {
        myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x0010;  // not Bidirectional Mode
    }
    myReg.offset = SIS8300_AD9510_SPI_REG;
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



#ifdef CLK2
    printf("AD9510 No.2: Use CLK_2!\n");
    myReg.offset = SIS8300_AD9510_SPI_REG;
    myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x4500; // addr
    myReg.data = myReg.data + 0x10 ; // Power-Down RefIn
    myReg.data = myReg.data + 0x08 ; // Shut Down Clk to PLL Prescaler
    myReg.data = myReg.data + 0x02 ; // Power-Down CLK1
    myReg.data = myReg.data + 0x00 ; // CLK2 Drives Distribution Section
    ioctl(device, SIS8300_REG_WRITE, &myReg);
    usleep(1) ;
#else
    printf("AD9510 No.2: Use CLK_1!\n");
    myReg.offset = SIS8300_AD9510_SPI_REG;
    myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x4500; // addr
    myReg.data = myReg.data + 0x10 ; // Power-Down RefIn
    myReg.data = myReg.data + 0x08 ; // Shut Down Clk to PLL Prescaler
    myReg.data = myReg.data + 0x04 ; // Power-Down CLK2
    myReg.data = myReg.data + 0x01 ; // CLK1 Drives Distribution Section
    ioctl(device, SIS8300_REG_WRITE, &myReg);
    usleep(1) ;
#endif

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
    if (ad9510_synch_cmd == 1)
    {
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

    if (adc_device_no > 4)
    {
        return -1;
    }
    uint_adc_mux_select = adc_device_no << 24 ;

    myReg.offset = SIS8300_ADC_SPI_REG;

    // output type LVDS
    addr = (0x14 & 0xffff) << 8 ;

    data = (0x40 & 0xff)  ;
    //data = (0x41 & 0xff);

    myReg.data = uint_adc_mux_select + addr + data;
    ioctl(device, SIS8300_REG_WRITE, &myReg);
    usleep(1) ;

    addr = (0x16 & 0xffff) << 8 ;
    data = (0x00 & 0xff)  ;
    myReg.data = uint_adc_mux_select + addr + data;
    ioctl(device, SIS8300_REG_WRITE, &myReg);
    usleep(1) ;

//#ifdef raus
    addr = (0x0B & 0xffff) << 8 ;
    data = (0x00 & 0xff)  ;
    myReg.data = uint_adc_mux_select + addr + data;
    ioctl(device, SIS8300_REG_WRITE, &myReg);
    usleep(1) ;
//#endif

//#ifdef raus
    addr = (0x17 & 0xffff) << 8 ;
    data = (0x00 & 0xff)  ;
    myReg.data = uint_adc_mux_select + addr + data;
    ioctl(device, SIS8300_REG_WRITE, &myReg);
    usleep(1) ;
//#endif

//#ifdef raus
    addr = (0x18 & 0xffff) << 8 ;
    data = (0xC0 & 0xff)  ;
    myReg.data = uint_adc_mux_select + addr + data;
    ioctl(device, SIS8300_REG_WRITE, &myReg);
    usleep(1) ;
//#endif

//#ifdef raus
    addr = (0x30 & 0xffff) << 8 ;
    data = (0x00 & 0xff)  ; // Dither disable
    myReg.data = uint_adc_mux_select + addr + data;
    ioctl(device, SIS8300_REG_WRITE, &myReg);
    usleep(1) ;
    //printf("update \n");
//#endif

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

    if (adc_device_no > 4)
    {
        return -1;
    }
    if (testmode > 7)
    {
        testmode=0;
    }
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
    if (adc_device_no > 4)
    {
        return -1;
    }
    uint_adc_mux_select = adc_device_no << 24 ;

    data = uint_adc_mux_select + 0x800000 +  ((spi_addr & 0xffff) << 8) + (0x00 & 0xff)  ;
    SIS8300_Write_Register(device,  SIS8300_ADC_SPI_REG,  data) ;


    do
    {
        SIS8300_Read_Register(device,  SIS8300_ADC_SPI_REG, &data) ;
        //printf("SIS8300_Read_Register  = 0x%08x  pollcounter = %d\n", data, pollcounter);
        pollcounter--;
    }
    while (((data & 0x80000000) == 0x80000000) && (pollcounter > 0));
    //     printf("SIS8300_Read_Register  = 0x%08x  pollcounter = %d\n", data, pollcounter);
    if (pollcounter == 0)
    {
        return -2;
    }
    SIS8300_Read_Register(device,  SIS8300_ADC_SPI_REG, &data) ;
    *read_data = data & 0xff ;
    return 0;
}



/******************************************************************************/
/*                                                                            */
/*     SIS8300 SI526 Clock Multiplier routines                                */
/*                                                                            */
/******************************************************************************/
/*                                                                            */
/* input:                                                                     */
/*                                                                            */
/******************************************************************************/

#define SI5326_SPI_POLL_COUNTER_MAX							1000
#define SI5326_SPI_CALIBRATION_READY_POLL_COUNTER_MAX		10000

int si5326_clk_muliplier_write(int device,  unsigned int addr, unsigned int data)
{
    unsigned int write_data, read_data ;
    unsigned int poll_counter   ;
// write address
    write_data = 0x0000 + (addr & 0xff) ; // write ADDR Instruction + register addr
    SIS8300_Write_Register(device,  SIS8300_CLOCK_MULTIPLIER_SPI_REG,  write_data) ;
    usleep(10) ;

    poll_counter = 0 ;
    do
    {
        poll_counter++;
        SIS8300_Read_Register(device,  SIS8300_CLOCK_MULTIPLIER_SPI_REG, &read_data) ;
    }
    while (((read_data & 0x80000000) == 0x80000000) && (poll_counter < SI5326_SPI_POLL_COUNTER_MAX)) ;
    if (poll_counter == SI5326_SPI_POLL_COUNTER_MAX)
    {
        return -2 ;
    }
    usleep(10) ;

// write data
    write_data = 0x4000 + (data & 0xff) ; // write Instruction + data
    SIS8300_Write_Register(device,  SIS8300_CLOCK_MULTIPLIER_SPI_REG,  write_data) ;
    usleep(10) ;

    poll_counter = 0 ;
    do
    {
        poll_counter++;
        SIS8300_Read_Register(device,  SIS8300_CLOCK_MULTIPLIER_SPI_REG, &read_data) ;
    }
    while (((read_data & 0x80000000) == 0x80000000) && (poll_counter < SI5326_SPI_POLL_COUNTER_MAX)) ;
    if (poll_counter == SI5326_SPI_POLL_COUNTER_MAX)
    {
        return -2 ;
    }

    return 0 ;
}





int si5326_clk_muliplier_read(int device,  unsigned int addr, unsigned int *data)
{
    unsigned int write_data, read_data ;
    unsigned int poll_counter   ;
// read address
    write_data = 0x0000 + (addr & 0xff) ; // read ADDR Instruction + register addr
    SIS8300_Write_Register(device,  SIS8300_CLOCK_MULTIPLIER_SPI_REG,  write_data) ;
    usleep(10) ;

    poll_counter = 0 ;
    do
    {
        poll_counter++;
        SIS8300_Read_Register(device,  SIS8300_CLOCK_MULTIPLIER_SPI_REG, &read_data) ;
    }
    while (((read_data & 0x80000000) == 0x80000000) && (poll_counter < SI5326_SPI_POLL_COUNTER_MAX)) ;
    if (poll_counter == SI5326_SPI_POLL_COUNTER_MAX)
    {
        return -2 ;
    }
    usleep(10) ;

// read data
    write_data = 0x8000  ; // read Instruction + data
    SIS8300_Write_Register(device,  SIS8300_CLOCK_MULTIPLIER_SPI_REG,  write_data) ;
    usleep(10) ;

    poll_counter = 0 ;
    do
    {
        poll_counter++;
        SIS8300_Read_Register(device,  SIS8300_CLOCK_MULTIPLIER_SPI_REG, &read_data) ;
    }
    while (((read_data & 0x80000000) == 0x80000000) && (poll_counter < SI5326_SPI_POLL_COUNTER_MAX)) ;
    if (poll_counter == SI5326_SPI_POLL_COUNTER_MAX)
    {
        return -2 ;
    }
    *data = (read_data ) ;
    return (0) ;
}
/****************************************************************************************************/


int si5326_clk_muliplier_internal_calibration_cmd(int device )
{
    unsigned int return_code ;
    unsigned int write_data, read_data ;
    unsigned int poll_counter, cal_poll_counter  ;
// write address
    write_data = 0x0000 + 136 ; // write ADDR Instruction + register addr
    SIS8300_Write_Register(device,  SIS8300_CLOCK_MULTIPLIER_SPI_REG,  write_data) ;
    usleep(10) ;

    poll_counter = 0 ;
    do
    {
        poll_counter++;
        SIS8300_Read_Register(device,  SIS8300_CLOCK_MULTIPLIER_SPI_REG, &read_data) ;
    }
    while (((read_data & 0x80000000) == 0x80000000) && (poll_counter < SI5326_SPI_POLL_COUNTER_MAX)) ;
    if (poll_counter == SI5326_SPI_POLL_COUNTER_MAX)
    {
        return -2 ;
    }
    usleep(10) ;

// write data
    write_data = 0x4000 + 0x40 ; // write Instruction + data
    SIS8300_Write_Register(device,  SIS8300_CLOCK_MULTIPLIER_SPI_REG,  write_data) ;
    usleep(10) ;

    poll_counter = 0 ;
    do
    {
        poll_counter++;
        SIS8300_Read_Register(device,  SIS8300_CLOCK_MULTIPLIER_SPI_REG, &read_data) ;
    }
    while (((read_data & 0x80000000) == 0x80000000) && (poll_counter < SI5326_SPI_POLL_COUNTER_MAX)) ;
    if (poll_counter == SI5326_SPI_POLL_COUNTER_MAX)
    {
        return -2 ;
    }
    usleep(10) ;

// poll until Calibration is ready
    cal_poll_counter = 0 ;
    do
    {
        cal_poll_counter++;
        // read data
        write_data = 0x8000  ; // read Instruction + data
        SIS8300_Write_Register(device,  SIS8300_CLOCK_MULTIPLIER_SPI_REG,  write_data) ;
        usleep(10) ;

        poll_counter = 0 ;
        do
        {
            poll_counter++;
            SIS8300_Read_Register(device,  SIS8300_CLOCK_MULTIPLIER_SPI_REG, &read_data) ;
        }
        while (((read_data & 0x80000000) == 0x80000000) && (poll_counter < SI5326_SPI_POLL_COUNTER_MAX)) ;
        if (poll_counter == SI5326_SPI_POLL_COUNTER_MAX)
        {
            return -2 ;
        }
        usleep(10) ;

    }
    while (((read_data & 0x40) == 0x40) && (cal_poll_counter < SI5326_SPI_CALIBRATION_READY_POLL_COUNTER_MAX)) ;
    if (cal_poll_counter == SI5326_SPI_CALIBRATION_READY_POLL_COUNTER_MAX)
    {
        return -3 ;
    }

    return 0 ;
}

/****************************************************************************************************/


int si5326_get_status_external_clock_multiplier(int device, unsigned int *status  )
{
    int rc;
    unsigned int data ;
    rc = si5326_clk_muliplier_read(device,  128, &data); //
    *status = data & 0x1 ;
    rc = si5326_clk_muliplier_read(device,  129, &data); //
    *status = *status + (data & 0x2) ;
    return rc ;
}
/****************************************************************************************************/

int si5326_bypass_external_clock_multiplier(int device)
{
    int rc;
    rc = si5326_clk_muliplier_write(device,  21, 0xFE); // override clksel_pin
    rc = si5326_clk_muliplier_write(device,  0, 0x2); // Bypass
    rc = si5326_clk_muliplier_write(device,  3, 0x0); // select clkin1
    rc = si5326_clk_muliplier_write(device, 11, 0x0); //  PowerDown clk2
    return rc ;
}
/****************************************************************************************************/

int si5326_bypass_internal_ref_osc_multiplier(int device)
{
    int rc;
    rc = si5326_clk_muliplier_write(device,  21, 0xFE); // override clksel_pin
    rc = si5326_clk_muliplier_write(device,  0, 0x42); // Bypass + free run
    rc = si5326_clk_muliplier_write(device,  3, 0x40); // select clkin2
    rc = si5326_clk_muliplier_write(device, 11, 0x0); //  PowerDown clk1
    return rc ;
}

/****************************************************************************************************/

int si5326_set_external_clock_multiplier(int device, unsigned int bw_sel, unsigned int n1_hs, unsigned int n1_clk1, unsigned int n1_clk2, unsigned int n2, unsigned int n3, unsigned int clkin1_mhz )
{
    int rc;
    volatile unsigned int n1_val ;
    volatile unsigned int n1_hs_val ;
    volatile unsigned int n2_val ;
    volatile unsigned int n3_val ;

    // input frequency
    if ((clkin1_mhz < 10) || (clkin1_mhz > 250))
    {
        return -2 ;
    }
    // bw_sel : see DSPLLsinm for setting
    if (bw_sel > 15)
    {
        return -3 ;
    }
    // n1_hs
    if ((n1_hs < 4) || (n1_hs > 11))
    {
        return -4 ;
    }


    // n1_clk1
    if (n1_clk1 == 0)
    {
        return -5 ;
    }
    else
    {
        if ((((n1_clk1) & 0x1) == 1) && (n1_clk1 != 1) )   // odd but not 1
        {
            return -5 ;
        }
        if ((n1_clk1 & 0xfff00000) != 0)    // > 2**20
        {
            return -5 ;
        }
    }

    // n1_clk2
    if (n1_clk2 == 0)
    {
        return -6 ;
    }
    else
    {
        if ((((n1_clk2) & 0x1) == 1) && (n1_clk2 != 1) )   // odd but not 1
        {
            return -6 ;
        }
        if ((n1_clk2 & 0xfff00000) != 0)    // > 2**20
        {
            return -6 ;
        }
    }


    // n2
    if ((n2 < 32) || (n2 > 512))
    {
        return -7 ;
    }
    else
    {
        if ((n2 & 0x1) == 1)    // odd
        {
            return -7 ;
        }
    }

    // n3
    if (n3 == 0)
    {
        return -8 ;
    }
    else
    {
        if ((n3 & 0xfff80000) != 0)    // > 2**19
        {
            return -8 ;
        }
    }


    si5326_clk_muliplier_write(device,  0, 0x0); // No Bypass
    si5326_clk_muliplier_write(device, 11, 0x02); //  PowerDown clk2

    // N3 = 1
    n3_val = n3 - 1 ;
    si5326_clk_muliplier_write(device, 43, ((n3_val >> 16) & 0x7) );       //  N3 bits 18:16
    si5326_clk_muliplier_write(device, 44, ((n3_val >> 8) & 0xff));       //  N3 bits 15:8
    si5326_clk_muliplier_write(device, 45, (n3_val  & 0xff) );       //  N3 bits 7:0

    n2_val = n2  ;
    si5326_clk_muliplier_write(device, 40, 0x00);						//    N2_LS bits 19:16
    si5326_clk_muliplier_write(device, 41, ((n2_val >> 8) & 0xff));     //  N2_LS bits 15:8
    si5326_clk_muliplier_write(device, 42, (n2_val  & 0xff));			//  N2_LS bits 7:0

    n1_hs_val = n1_hs - 4 ;
    si5326_clk_muliplier_write(device, 25, (n1_hs_val << 5)); //

    n1_val = n1_clk1 - 1 ;
    si5326_clk_muliplier_write(device, 31, ((n1_val >> 16) & 0xf));       //  NC1_LS (low speed divider) bits 19:16
    si5326_clk_muliplier_write(device, 32, ((n1_val >> 8) & 0xff));       //  NC1_LS (low speed divider) bits 15:8
    si5326_clk_muliplier_write(device, 33, (n1_val  & 0xff) );       //  NC1_LS (low speed divider) bits 7:0

    n1_val = n1_clk2 - 1 ;
    si5326_clk_muliplier_write(device, 34, ((n1_val >> 16) & 0xf));       //  NC2_LS (low speed divider) bits 19:16
    si5326_clk_muliplier_write(device, 35, ((n1_val >> 8) & 0xff));       //  NC2_LS (low speed divider) bits 15:8
    si5326_clk_muliplier_write(device, 36, (n1_val  & 0xff));       //  NC2_LS (low speed divider) bits 7:0


    si5326_clk_muliplier_write(device,  2, (bw_sel << 5) ); //BWSEL_REG

    si5326_clk_muliplier_internal_calibration_cmd(device);

    return 0;
}


