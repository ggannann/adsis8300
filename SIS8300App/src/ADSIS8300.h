/* SIS8300.cpp
 *
 * This is a driver for a Struck SIS8300 digitizer.
 * Based on ADCSimDetector ADExample.
 *
 * Author: Hinko Koceavar
 *         ESS ERIC, Lund, Sweden
 *
 * Created:  September 11, 2016
 *
 */

#include <epicsEvent.h>
#include <epicsTime.h>
#include "asynNDArrayDriver.h"

#include <sis830x.h>

#define SisAcquireString               "SIS_ACQUIRE"
#define SisAcquireTimeString           "SIS_ACQUIRE_TIME"
#define SisElapsedTimeString           "SIS_ELAPSED_TIME"
#define SisNumTimePointsString         "SIS_NUM_TIME_POINTS"
#define SisCountString                 "SIS_COUNT"
#define SisClockSourceString           "SIS_CLOCK_SOURCE"
#define SisClockFreqString             "SIS_CLOCK_FREQ"
#define SisClockDivString              "SIS_CLOCK_DIV"
#define SisTrigSourceString            "SIS_TRIG_SOURCE"
#define SisTrigDoString                "SIS_TRIG_DO"
#define SisTrigDelayString             "SIS_TRIG_DELAY"
#define SisTrigRepeatString            "SIS_TRIG_REPEAT"
#define SisChannelEnableString         "SIS_ENABLE"
#define SisChannelDataString           "SIS_DATA"
#define SisChannelConvFactorString     "SIS_CONV_FACTOR"
#define SisChannelConvOffsetString     "SIS_CONV_OFFSET"
#define SisChannelAttenuationString    "SIS_ATTENUATION"
#define SisChannelDecimFactorString    "SIS_DECIM_FACTOR"
#define SisChannelDecimOffsetString    "SIS_DECIM_OFFSET"

#define MAX_PATH_LEN                   32
#define MAX_ERROR_STR_LEN              32

/* Channel and sampling constants. */
#define SIS8300_NUM_ADCS                5
#define SIS8300_NUM_SIGNALS             (2 * SIS8300_NUM_ADCS)
#define SIS8300_SAMPLE_BYTES            2
#define SIS8300_BLOCK_SAMPLES           16
#define SIS8300_BLOCK_BYTES             (SIS8300_SAMPLE_BYTES * SIS8300_BLOCK_SAMPLES)
#define SIS8300_MAX_PRETRIG             2046
//#define SIS8300DRV_CH_SETUP_FIRST       0x100   /**< Register address of the "Trigger setup register" for the first channel. */
//#define SIS8300DRV_CH_THRESHOLD_FIRST   0x110   /**< Register address of the "Trigger threshold register" for the first channel. */


/* Registers */
#define SIS8300_ID_REG                  0x00
#define SIS8300_SERIALNR_REG            0x01
#define SIS8300_XILINX_JTAG_REG         0x02

#define SIS8300_USER_CTRL_REG           0x04
#define SIS8300_FIRMWARE_OPTIONS_REG    0x05
/* Firmware options bits */
# define SIS8300_FPGA_SX_1GB_MEM        (1<<8)
# define SIS8300_DUAL_CHANNEL_SAMPLING  (1<<2)
# define SIS8300_RINGBUFFER_DELAY_EN    (1<<1)
# define SIS8300_TRIGGER_BLOCK_EN       (1<<0)

#define SIS8300_ACQUISITION_CTRL_REG    0x10
#define SIS8300_SAMPLE_CTRL_REG         0x11

#define SIS8300_ADC_SPI_REG             0x48
#define SIS8300_ADC_INPUT_TAP_DELAY_REG 0x49

#define SIS8300_CH_ADDRESS_FIRST        0x120
#define SIS8300_SAMPLE_ADDRESS_CH1_REG  0x120
#define SIS8300_SAMPLE_ADDRESS_CH2_REG  0x121
#define SIS8300_SAMPLE_ADDRESS_CH3_REG  0x122
#define SIS8300_SAMPLE_ADDRESS_CH4_REG  0x123
#define SIS8300_SAMPLE_ADDRESS_CH5_REG  0x124
#define SIS8300_SAMPLE_ADDRESS_CH6_REG  0x125
#define SIS8300_SAMPLE_ADDRESS_CH7_REG  0x126
#define SIS8300_SAMPLE_ADDRESS_CH8_REG  0x127
#define SIS8300_SAMPLE_ADDRESS_CH9_REG  0x128
#define SIS8300_SAMPLE_ADDRESS_CH10_REG 0x129

#define SIS8300_SAMPLE_LENGTH_REG       0x12A
#define SIS8300_PRETRIGGER_DELAY_REG    0x12B

/** Struck SIS8300 driver; does 1-D waveforms on 10 channels.
  * Inherits from asynNDArrayDriver */
class epicsShareClass ADSIS8300 : public asynNDArrayDriver {
public:
	ADSIS8300(const char *portName, int numTimePoints, NDDataType_t dataType,
                   int maxBuffers, size_t maxMemory,
                   int priority, int stackSize);
	~ADSIS8300();

    /* These are the methods that we override from asynNDArrayDriver */
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    virtual asynStatus writeFloat64(asynUser *pasynUser, epicsFloat64 value);
    virtual void report(FILE *fp, int details);
    /**< Should be private, but gets called from C, so must be public */
    void sisTask();

protected:
    int P_Acquire;
    #define FIRST_SIS8300_PARAM P_Acquire
    int P_AcquireTime;
    int P_ElapsedTime;
    int P_NumTimePoints;
    int P_Count;
    int P_ClockSource;
    int P_ClockFreq;
    int P_ClockDiv;
    int P_TrigSource;
    int P_TrigDo;
    int P_TrigDelay;
    int P_TrigRepeat;
    int P_Enable;
    int P_Data;
    int P_ConvFactor;
    int P_ConvOffset;
    int P_Attenuation;
    int P_DecimFactor;
    int P_DecimOffset;

    int P_Dummy;
    #define LAST_SIS8300_PARAM P_Dummy


private:
    /* These are the methods that are new to this class */
    template <typename epicsType> int acquireArraysT();
    int acquireArrays();
    void setAcquire(int value);
    int initDevice();
    int destroyDevice();
    int getNumberOfSamples(unsigned int *nrSamples);
    int setChannelMask();
    int configureChannels(unsigned int nrSamples, unsigned int channelMask);
    int setNumberOfSamples(unsigned int nrSamples);
    int enableChannel(unsigned int channel);
    int disableChannel(unsigned int channel);
    int initADCs();
    int setPretriggerSamples(unsigned int nrSamples);

    /* low level SIS8300 handling */
    int sisOpenDevice();
    int sisCloseDevice();
    int sisReadReg(unsigned int reg, unsigned int *val);
    int sisWriteReg(unsigned int reg, unsigned int val);

    /* Our data */
    epicsEventId startEventId_;
    epicsEventId stopEventId_;
    int uniqueId_;
    int acquiring_;
    double elapsedTime_;

    char mSisDevicePath[MAX_PATH_LEN];
    PSIS830X_DEVICE mSisDevice;
    char mSisErrorStr[MAX_ERROR_STR_LEN];
    unsigned int mSisDeviceType;
    unsigned int mSisFirmwareVersion;
    unsigned long mSisMemorySize;
    unsigned int mSisSerialNumber;
    unsigned int mSisFirmwareOptions;
    uint32_t mChannelMask;
};


#define NUM_SIS8300_PARAMS ((int)(&LAST_SIS8300_PARAM - &FIRST_SIS8300_PARAM + 1))

