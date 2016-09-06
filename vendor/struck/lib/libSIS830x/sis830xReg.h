#ifndef SIS830XREG_H_INCLUDED
#define SIS830XREG_H_INCLUDED

#define SIS830X_MODULE_VERSION_REG      0x0

#define SIS830X_AD9510_SPI_REG          0x41
 #define AD9510_SPI_READ_CYCLE                      0x00800000
 #define AD9510_SPI_SELECT_NO2                      0x01000000
 #define AD9510_SPI_SET_FUNCTION_SYNCH_FPGA_CLK69	0x10000000
 #define AD9510_GENERATE_SPI_RW_CMD                 0x40000000
 #define AD9510_GENERATE_FUNCTION_PULSE_CMD         0x80000000
 #define AD9510_SPI_LOGIC_BUSY                      0x80000000

 #define SIS830X_ADC_TEMP_I2C_REG            0x6
 #define SIS830X_MGTCLK_SYNTH_I2C_REG       0x43
 #define SIS830X_RTM_I2C_REG                0x47
 #define I2C_ACK            8
 #define I2C_START          9
 #define I2C_REPEATSTART    10
 #define I2C_STOP           11
 #define I2C_WRITE_CYCLE    12
 #define I2C_READ_CYCLE     13
 #define I2C_BUSY           31

#define SIS830X_SPI_FLASH_CSR           0x44
 #define SPI_FLASH_RD_BLK_FIFO 9
 #define SPI_FLASH_RD_BLK_EN   10
 #define SPI_FLASH_WR_BLK_FILL 11
 #define SPI_FLASH_EXCH        12
 #define SPI_FLASH_WR_BLK_EN   13   // JK
 #define SPI_FLASH_CS          14   // JK
 #define SPI_FLASH_MUX_EN      15   // JK
 #define SPI_FLASH_BUSY        31

#endif // SIS830XREG_H_INCLUDED
