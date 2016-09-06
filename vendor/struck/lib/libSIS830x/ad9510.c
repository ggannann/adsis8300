#include <unistd.h>

#include "sis830x.h"
#include "internal.h"
#include "ad9510.h"

SIS830X_STATUS AD9510_SPI_Setup(PSIS830X_DEVICE device, unsigned int* ch_divider_configuration_array, unsigned int ad9510_synch_cmd )
{
    SIS830X_STATUS ret;
	uint32_t reg;

    // AD9510 (ADC channels 1 to 6)
	// set AD9510 and Soft Reset
	// on 8300 module the bus uses SDIO, on 8300L the bus uses seperate SDI SDO
	if(device->type == SIS8300L){
        reg = AD9510_GENERATE_SPI_RW_CMD + 0x0030;
	}
	if(device->type == SIS8300){
        reg = AD9510_GENERATE_SPI_RW_CMD + 0x00B0;
	}
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, reg);
	if(ret != Stat830xSuccess){
        return ret;
	}
	/*
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + 0x00B0;
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	*/
	usleep(1);

	// set AD9510 out of reset
    if(device->type == SIS8300L){
        reg = AD9510_GENERATE_SPI_RW_CMD + 0x0010;
	}
	if(device->type == SIS8300){
        reg = AD9510_GENERATE_SPI_RW_CMD + 0x0090;
	}
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, reg);
	if(ret != Stat830xSuccess){
        return ret;
	}
	/*
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + 0x0090;
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	*/
	usleep(1);

	// default: Asychrnon PowerDown, no Prescaler
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, AD9510_GENERATE_SPI_RW_CMD + 0x0A01);
	if(ret != Stat830xSuccess){
        return ret;
	}
	/*
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + 0x0A01;
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	*/
	usleep(1);

	// Out 0 (not used) : total Power-Down
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, AD9510_GENERATE_SPI_RW_CMD + 0x3C0B);
	if(ret != Stat830xSuccess){
        return ret;
	}
	/*
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + 0x3c0B;
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	*/
	usleep(1);

	// Out 1 (not used) : total Power-Down
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, AD9510_GENERATE_SPI_RW_CMD + 0x3D0B);
	if(ret != Stat830xSuccess){
        return ret;
	}
	/*
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + 0x3D0B;
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	*/
	usleep(1);

	// Out 2 (not used) : total Power-Down
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, AD9510_GENERATE_SPI_RW_CMD + 0x3E0B);
	if(ret != Stat830xSuccess){
        return ret;
	}
	/*
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + 0x3E0B;
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	*/
	usleep(1);

	// Out 3 (not used) : total Power-Down
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, AD9510_GENERATE_SPI_RW_CMD + 0x3F0B);
	if(ret != Stat830xSuccess){
        return ret;
	}
	/*
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + 0x3F0B;
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	*/
	usleep(1);


	// Out 4  (FPGA DIV-CLK05) : LVDS 3.5 mA
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, AD9510_GENERATE_SPI_RW_CMD + 0x4002);
	if(ret != Stat830xSuccess){
        return ret;
	}
	/*
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + 0x4002;
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	*/
	usleep(1);

	// Out 5  (ADC3-CLK, ch4/5) : LVDS 3.5 mA
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, AD9510_GENERATE_SPI_RW_CMD + 0x4102);
	if(ret != Stat830xSuccess){
        return ret;
	}
	/*
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + 0x4102;
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	*/
	usleep(1);

	// Out 6  (ADC2-CLK, ch3/4) : LVDS 3.5 mA
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, AD9510_GENERATE_SPI_RW_CMD + 0x4202);
	if(ret != Stat830xSuccess){
        return ret;
	}
	/*
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + 0x4202;
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	*/
	usleep(1);

	// Out 7  (ADC1-CLK, ch1/2) : LVDS 3.5 mA
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, AD9510_GENERATE_SPI_RW_CMD + 0x4302);
	if(ret != Stat830xSuccess){
        return ret;
	}
	/*
	myReg.offset = SIS8300_AD9510_SPI_REG;
	myReg.data = AD9510_GENERATE_SPI_RW_CMD + 0x4302;
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	*/
	usleep(1);



	//myReg.offset = SIS8300_AD9510_SPI_REG;
	reg = AD9510_GENERATE_SPI_RW_CMD + 0x4500; // addr
	//myReg.data = myReg.data + 0x1D ;
	reg += 0x10 ; // Power-Down RefIn
	reg += 0x08 ; // Shut Down Clk to PLL Prescaler
	reg += 0x04 ; // Power-Down CLK2
	reg += 0x01 ; // CLK1 Drives Distribution Section

	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, reg);
	if(ret != Stat830xSuccess){
        return ret;
	}
	/*
	ioctl(device, SIS8300_REG_WRITE, &myReg);
	*/
	usleep(1);





    // Out 4  (FPGA DIV-CLK05) : Divider Low/High
	//myReg.offset = SIS8300_AD9510_SPI_REG;
	reg = AD9510_GENERATE_SPI_RW_CMD + 0x5000; // addr
	reg += ch_divider_configuration_array[6] & 0xff;
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, reg);
	if(ret != Stat830xSuccess){
        return ret;
	}
	//ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1);

	// Out 4  (FPGA DIV-CLK05) : Bypasse Diver (7), No Sychn (6), Force Individual Start (5), Start High (4), Phase Offset (3:0)
	//myReg.offset = SIS8300_AD9510_SPI_REG;
	reg = AD9510_GENERATE_SPI_RW_CMD + 0x5100; // addr
	reg += (ch_divider_configuration_array[6] >> 8) & 0xff;
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, reg);
	if(ret != Stat830xSuccess){
        return ret;
	}
	//ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1);

	// Out 5  (ADC3-CLK, ch4/5) : Divider Low/High
	//myReg.offset = SIS8300_AD9510_SPI_REG;
	reg = AD9510_GENERATE_SPI_RW_CMD + 0x5200; // addr
	reg += ch_divider_configuration_array[2] & 0xff;
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, reg);
	if(ret != Stat830xSuccess){
        return ret;
	}
	//ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1);

	// Out 5  (ADC3-CLK, ch4/5) : Bypasse Diver (7), No Sychn (6), Force Individual Start (5), Start High (4), Phase Offset (3:0)
	//myReg.offset = SIS8300_AD9510_SPI_REG;
	reg = AD9510_GENERATE_SPI_RW_CMD + 0x5300; // addr
	reg += (ch_divider_configuration_array[2] >> 8) & 0xff;
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, reg);
	if(ret != Stat830xSuccess){
        return ret;
	}
	//ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1);

	// Out 6  (ADC2-CLK, ch2/3) : Divider Low/High
	//myReg.offset = SIS8300_AD9510_SPI_REG;
	reg = AD9510_GENERATE_SPI_RW_CMD + 0x5400; // addr
	reg += ch_divider_configuration_array[1] & 0xff;
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, reg);
	if(ret != Stat830xSuccess){
        return ret;
	}
	//ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1);

	// Out 6  (ADC2-CLK, ch2/3) : Bypasse Diver (7), No Sychn (6), Force Individual Start (5), Start High (4), Phase Offset (3:0)
	//myReg.offset = SIS8300_AD9510_SPI_REG;
	reg = AD9510_GENERATE_SPI_RW_CMD + 0x5500; // addr
	reg += (ch_divider_configuration_array[1] >> 8) & 0xff;
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, reg);
	if(ret != Stat830xSuccess){
        return ret;
	}
	//ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1);

	// Out 7  (ADC1-CLK, ch1/2) : Divider Low/High
	//myReg.offset = SIS8300_AD9510_SPI_REG;
	reg = AD9510_GENERATE_SPI_RW_CMD + 0x5600; // addr
	reg += ch_divider_configuration_array[0] & 0xff;
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, reg);
	if(ret != Stat830xSuccess){
        return ret;
	}
	//ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1);

	// Out 7  (ADC1-CLK, ch1/2) : Bypasse Diver (7), No Sychn (6), Force Individual Start (5), Start High (4), Phase Offset (3:0)
	//myReg.offset = SIS8300_AD9510_SPI_REG;
	reg = AD9510_GENERATE_SPI_RW_CMD + 0x5700; // addr
	reg += (ch_divider_configuration_array[0] >> 8) & 0xff;
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, reg);
	if(ret != Stat830xSuccess){
        return ret;
	}
	//ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1);

	// update command
	//myReg.offset = SIS8300_AD9510_SPI_REG;
	//myReg.data = AD9510_GENERATE_SPI_RW_CMD + 0x5A01;
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, AD9510_GENERATE_SPI_RW_CMD + 0x5A01);
	if(ret != Stat830xSuccess){
        return ret;
	}
	//ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1);


    // AD9510 (ADC channels 7 to 10)
	// set AD9510 to Soft Reset
	//myReg.offset = SIS8300_AD9510_SPI_REG;
	//myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x00B0;
    if(device->type == SIS8300L){
        reg = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x0030;
	}
	if(device->type == SIS8300){
        reg = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x00B0;
	}
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, reg);
	if(ret != Stat830xSuccess){
        return ret;
	}
	//ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1);

	// set AD9510 out of reset
	//myReg.offset = SIS8300_AD9510_SPI_REG;
	//myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x0090;
    if(device->type == SIS8300L){
        reg = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x0010;
	}
	if(device->type == SIS8300){
        reg = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x0090;
	}
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, reg);
	if(ret != Stat830xSuccess){
        return ret;
	}
	//ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1);

	//myReg.offset = SIS8300_AD9510_SPI_REG;
	//myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x0A01;
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x0A01);
	if(ret != Stat830xSuccess){
        return ret;
	}
	//ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1);

	// Out 0 (not used) : total Power-Down
	//myReg.offset = SIS8300_AD9510_SPI_REG;
	//myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x3c0B;
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x3C0B);
	if(ret != Stat830xSuccess){
        return ret;
	}
	//ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1);


	// Out 1 (DIV1_OUT1: MGT CLK SYNTH IN4/5) : LVDS 3.5 mA
	//myReg.offset = SIS8300_AD9510_SPI_REG;
	//myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x3D0B;
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x3D02);
	if(ret != Stat830xSuccess){
        return ret;
	}
	//ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1);

	// Out 2 (not used) : total Power-Down
	//myReg.offset = SIS8300_AD9510_SPI_REG;
	//myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x3E0B;
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x3E0B);
	if(ret != Stat830xSuccess){
        return ret;
	}
	//ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1);

	// Out 3 (not used) : total Power-Down
	//myReg.offset = SIS8300_AD9510_SPI_REG;
	//myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x3F0B;
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x3F0B);
	if(ret != Stat830xSuccess){
        return ret;
	}
	//ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1);


	// Out 4  (FPGA DIV-CLK69) : LVDS 3.5 mA
	//myReg.offset = SIS8300_AD9510_SPI_REG;
	//myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x4002;
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x4002);
	if(ret != Stat830xSuccess){
        return ret;
	}
	//ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1);

	// Out 5  (ADC5-CLK, ch8/9) : LVDS 3.5 mA
	//myReg.offset = SIS8300_AD9510_SPI_REG;
	//myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x4102;
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x4102);
	if(ret != Stat830xSuccess){
        return ret;
	}
	//ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1);

	// Out 6  (ADC4-CLK, ch6/7) : LVDS 3.5 mA
	//myReg.offset = SIS8300_AD9510_SPI_REG;
	//myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x4202;
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x4202);
	if(ret != Stat830xSuccess){
        return ret;
	}
	//ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1);

	// Out 7  (Frontpanel Clk, Harlink) : LVDS 3.5 mA
	//myReg.offset = SIS8300_AD9510_SPI_REG;
	//myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x4302; // on
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x4302);
	if(ret != Stat830xSuccess){
        return ret;
	}
	//ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1);





	//myReg.offset = SIS8300_AD9510_SPI_REG;
	reg = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x4500; // addr
	reg += 0x10 ; // Power-Down RefIn
	reg += 0x08 ; // Shut Down Clk to PLL Prescaler
	reg += 0x04 ; // Power-Down CLK2
	reg += 0x01 ; // CLK1 Drives Distribution Section
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, reg);
	if(ret != Stat830xSuccess){
        return ret;
	}
	//ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1);

	// Out 4  (FPGA DIV-CLK69) : Divider Low/High
	//myReg.offset = SIS8300_AD9510_SPI_REG;
	reg = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x5000; // addr
	reg += ch_divider_configuration_array[7] & 0xff;
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, reg);
	if(ret != Stat830xSuccess){
        return ret;
	}
	//ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1);

	// Out 4  (FPGA DIV-CLK69) : Bypasse Diver (7), No Sychn (6), Force Individual Start (5), Start High (4), Phase Offset (3:0)
	//myReg.offset = SIS8300_AD9510_SPI_REG;
	reg = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x5100; // addr
	reg += (ch_divider_configuration_array[7] >> 8) & 0xff;
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, reg);
	if(ret != Stat830xSuccess){
        return ret;
	}
	//ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1);

	// Out 5  (ADC5-CLK, ch8/9) : Divider Low/High
	//myReg.offset = SIS8300_AD9510_SPI_REG;
	reg = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x5200; // addr
	reg += ch_divider_configuration_array[4] & 0xff;
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, reg);
	if(ret != Stat830xSuccess){
        return ret;
	}
	//ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1);

	// Out 5  (ADC5-CLK, ch8/9) : Bypasse Diver (7), No Sychn (6), Force Individual Start (5), Start High (4), Phase Offset (3:0)
	//myReg.offset = SIS8300_AD9510_SPI_REG;
	reg = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x5300; // addr
	reg += (ch_divider_configuration_array[4] >> 8) & 0xff;
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, reg);
	if(ret != Stat830xSuccess){
        return ret;
	}
	//ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1);


	// Out 6  (ADC4-CLK, ch6/7) : Divider Low/High
	//myReg.offset = SIS8300_AD9510_SPI_REG;
	reg = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x5400; // addr
	reg += ch_divider_configuration_array[3] & 0xff;
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, reg);
	if(ret != Stat830xSuccess){
        return ret;
	}
	//ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1);

	// Out 6  (ADC4-CLK, ch6/7) : Bypasse Diver (7), No Sychn (6), Force Individual Start (5), Start High (4), Phase Offset (3:0)
	//myReg.offset = SIS8300_AD9510_SPI_REG;
	reg = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x5500; // addr
	reg += (ch_divider_configuration_array[3] >> 8) & 0xff;
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, reg);
	if(ret != Stat830xSuccess){
        return ret;
	}
	//ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1);

	// Out 7  (Frontpanel Clk, Harlink) : Divider Low/High
	//myReg.offset = SIS8300_AD9510_SPI_REG;
	reg = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x5600; // addr
	reg += ch_divider_configuration_array[5] & 0xff;
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, reg);
	if(ret != Stat830xSuccess){
        return ret;
	}
	//ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1);

	// Out 7  (Frontpanel Clk, Harlink) : Bypasse Diver (7), No Sychn (6), Force Individual Start (5), Start High (4), Phase Offset (3:0)
	//myReg.offset = SIS8300_AD9510_SPI_REG;
	reg = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x5700;// addr
	reg += (ch_divider_configuration_array[5] >> 8) & 0xff;
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, reg);
	if(ret != Stat830xSuccess){
        return ret;
	}
	//ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1);

	// update command
	//myReg.offset = SIS8300_AD9510_SPI_REG;
	//myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x5A01;
	ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x5A01);
	if(ret != Stat830xSuccess){
        return ret;
	}
	//ioctl(device, SIS8300_REG_WRITE, &myReg);
	usleep(1);

	// synch Cmd
	if(ad9510_synch_cmd == 1){
	    // set Function of "Function pin" to SYNCB (Default Reset)
	    //myReg.offset = SIS8300_AD9510_SPI_REG;
	    //myReg.data =  AD9510_GENERATE_SPI_RW_CMD + 0x5822;
	    ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, AD9510_GENERATE_SPI_RW_CMD + 0x5822);
        if(ret != Stat830xSuccess){
            return ret;
        }
	    //ioctl(device, SIS8300_REG_WRITE, &myReg);
	    usleep(1);
	    // update command
	    //myReg.offset = SIS8300_AD9510_SPI_REG;
	    //myReg.data =   AD9510_GENERATE_SPI_RW_CMD + 0x5A01;
	    ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, AD9510_GENERATE_SPI_RW_CMD + 0x5A01);
        if(ret != Stat830xSuccess){
            return ret;
        }
	    //ioctl(device, SIS8300_REG_WRITE, &myReg);
	    usleep(1);


	    // set Function of "Function pin" to SYNCB (Default Reset)
	    //myReg.offset = SIS8300_AD9510_SPI_REG;
	    //myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x5822;
	    ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x5822);
        if(ret != Stat830xSuccess){
            return ret;
        }
	    //ioctl(device, SIS8300_REG_WRITE, &myReg);
	    usleep(1);
	    // update command
	    //myReg.offset = SIS8300_AD9510_SPI_REG;
	    //myReg.data = AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x5A01;
	    ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, AD9510_GENERATE_SPI_RW_CMD + AD9510_SPI_SELECT_NO2 + 0x5A01);
        if(ret != Stat830xSuccess){
            return ret;
        }
	    //ioctl(device, SIS8300_REG_WRITE, &myReg);
	    usleep(1);

	    // set use "FPGA DIV-CLK69" for Sychn Pulse  - Logic (VIRTEX5)
	    //myReg.offset = SIS8300_AD9510_SPI_REG;
	    //myReg.data = AD9510_SPI_SET_FUNCTION_SYNCH_FPGA_CLK69  ; //set clock
	    ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, AD9510_SPI_SET_FUNCTION_SYNCH_FPGA_CLK69);
        if(ret != Stat830xSuccess){
            return ret;
        }
	    //ioctl(device, SIS8300_REG_WRITE, &myReg);
	    usleep(1) ;

	    // generate sych Pulse   (pulse Function pins) and hold FPGA_CLK69 to sycnh pulse
	    //myReg.offset = SIS8300_AD9510_SPI_REG;
	    //myReg.data = AD9510_GENERATE_FUNCTION_PULSE_CMD + AD9510_SPI_SET_FUNCTION_SYNCH_FPGA_CLK69   ; //
	    ret = intRegWrite(device, SIS830X_AD9510_SPI_REG, AD9510_GENERATE_FUNCTION_PULSE_CMD + AD9510_SPI_SET_FUNCTION_SYNCH_FPGA_CLK69);
        if(ret != Stat830xSuccess){
            return ret;
        }
	    //ioctl(device, SIS8300_REG_WRITE, &myReg);
	    usleep(1);
	}

	return 0;
}
