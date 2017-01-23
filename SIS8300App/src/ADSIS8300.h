/* SIS8300.h
 *
 * This is a driver for a Struck SIS8300 digitizer.
 * Based on ADCSimDetector ADExample.
 *
 * Author: Hinko Kocevar
 *         ESS ERIC, Lund, Sweden
 *
 * Created:  September 11, 2016
 *
 */

#include <stdint.h>
#include <epicsEvent.h>
#include <epicsTime.h>

#include <asynNDArrayDriver.h>

#include <sis8300drv.h>

#define SisMessageString               "SIS_MESSAGE"
#define SisAcquireString               "SIS_ACQUIRE"
#define SisAcquireTimeString           "SIS_ACQUIRE_TIME"
#define SisElapsedTimeString           "SIS_ELAPSED_TIME"
#define SisTimeStepString              "SIS_TIME_STEP"
#define SisNumTimePointsString         "SIS_NUM_TIME_POINTS"
#define SisClockSourceString           "SIS_CLOCK_SOURCE"
#define SisClockFreqString             "SIS_CLOCK_FREQ"
#define SisClockDivString              "SIS_CLOCK_DIV"
#define SisTrigSourceString            "SIS_TRIG_SOURCE"
#define SisTrigLineString              "SIS_TRIG_LINE"
#define SisTrigDoString                "SIS_TRIG_DO"
#define SisTrigDelayString             "SIS_TRIG_DELAY"
#define SisTrigRepeatString            "SIS_TRIG_REPEAT"
#define SisChannelEnableString         "SIS_ENABLE"
#define SisChannelConvFactorString     "SIS_CONV_FACTOR"
#define SisChannelConvOffsetString     "SIS_CONV_OFFSET"
#define SisChannelAttenuationString    "SIS_ATTENUATION"
#define SisChannelDecimFactorString    "SIS_DECIM_FACTOR"
#define SisChannelDecimOffsetString    "SIS_DECIM_OFFSET"
#define SisResetString                 "SIS_RESET"
#define SisFirmwareVersionString       "SIS_FW_VERSION"
#define SisDeviceTypeString            "SIS_DEVICE_TYPE"
#define SisSerialNumberString          "SIS_SERIAL_NUMBER"
#define SisMemorySizeString            "SIS_MEMORY_SIZE"
#define SisRTMTypeString               "SIS_RTM_TYPE"
#define SisRTMTempGetString            "SIS_RTM_TEMP_GET"
#define SisRTMTemp1String              "SIS_RTM_TEMP1"
#define SisRTMTemp2String              "SIS_RTM_TEMP2"

#define MAX_PATH_LEN                   32
#define MAX_LOG_STR_LEN                256
#define ADSIS8300_NUM_CHANNELS         10


#define ADSIS8300_LOG(p, s, t, r) ({\
	if (t == 0) { \
		snprintf(mSisLogStr, MAX_LOG_STR_LEN, "%s %s::%s: %s", \
				p, driverName, __func__, s); \
	} else if (t == 1) { \
        snprintf(mSisLogStr, MAX_LOG_STR_LEN, "[ERR] %s::%s: %s() failed with '%s' (%d)", \
                driverName, __func__, s, sis8300drv_strerror(r), r); \
	} \
	asynPrint(pasynUserSelf, ASYN_TRACE_FLOW, \
			  "%s\n", mSisLogStr); \
	setStringParam(P_Message, mSisLogStr); \
})

#define ADSIS8300_INF(s) ({\
	ADSIS8300_LOG("[INF]", s, 0, 0); \
})

#define ADSIS8300_ERR(s) ({\
	ADSIS8300_LOG("[ERR]", s, 0, 0); \
})

#define SIS8300DRV_CALL_0(s, x) ({\
	int __ret = x; \
	if (__ret) {\
		ADSIS8300_LOG("[ERR]", s, 1, __ret); \
	} \
	__ret; \
})

#define SIS8300DRV_CALL(s, x) ({\
	int __ret = SIS8300DRV_CALL_0(s, x); \
	__ret; \
})

#define SIS8300DRV_CALL_VOID(s, x) ({\
	SIS8300DRV_CALL_0(s, x); \
})

#define SIS8300DRV_CALL_RET(s, x) ({\
	int __ret = SIS8300DRV_CALL_0(s, x); \
	if (__ret) { return __ret; } \
})

/** Struck SIS8300 driver; does 1-D waveforms on 10 channels.
  * Inherits from asynNDArrayDriver */
class epicsShareClass ADSIS8300 : public asynNDArrayDriver {
public:
	ADSIS8300(const char *portName, const char *devicePath,
			int maxAddr, int numParams, int numTimePoints, NDDataType_t dataType,
			int maxBuffers, size_t maxMemory, int priority, int stackSize);
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
    int P_TimeStep;
    int P_NumTimePoints;
    int P_ClockSource;
    int P_ClockFreq;
    int P_ClockDiv;
    int P_TrigSource;
    int P_TrigLine;
    int P_TrigDo;
    int P_TrigDelay;
    int P_TrigRepeat;
    int P_Enable;
    int P_ConvFactor;
    int P_ConvOffset;
    int P_Attenuation;
    int P_DecimFactor;
    int P_DecimOffset;
    int P_Reset;
    int P_Message;
    int P_FirmwareVersion;
    int P_SerialNumber;
    int P_DeviceType;
    int P_MemorySize;
    int P_RTMType;
    int P_RTMTempGet;
    int P_RTMTemp1;
    int P_RTMTemp2;

    int P_Dummy;
    #define LAST_SIS8300_PARAM P_Dummy

    /* These are the methods that are new to this class */
    int acquireRawArrays();
    template <typename epicsType> int convertArraysT();
    virtual int acquireArrays();
    void setAcquire(int value);
    virtual int initDevice();
    virtual int destroyDevice();
    virtual int enableChannel(unsigned int channel);
    virtual int disableChannel(unsigned int channel);
    virtual int initDeviceDone();
    virtual int armDevice();
    virtual int disarmDevice();
    virtual int waitForDevice();
    virtual int deviceDone();
    virtual int updateParameters();

    sis8300drv_usr *mSisDevice;
    uint32_t mChannelMask;
    NDArray *mRawDataArray;
    char mSisLogStr[MAX_LOG_STR_LEN];
    int mNumArrays;

private:

    /* Our data */
    epicsEventId startEventId_;
    epicsEventId stopEventId_;
    int uniqueId_;
    int acquiring_;
    double elapsedTime_;

    char mSisDevicePath[MAX_PATH_LEN];
    unsigned int mSisFirmwareOptions;
};


#define NUM_SIS8300_PARAMS ((int)(&LAST_SIS8300_PARAM - &FIRST_SIS8300_PARAM + 1))

