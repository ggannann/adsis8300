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
#define SisNumAiSamplesString          "SIS_NUM_AI_SAMPLES"
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
#define SisFwVersionString             "SIS_FW_VERSION"
#define SisDeviceTypeString            "SIS_DEVICE_TYPE"
#define SisSerialNumberString          "SIS_SERIAL_NUMBER"
#define SisMemorySizeString            "SIS_MEMORY_SIZE"
#define SisRTMTypeString               "SIS_RTM_TYPE"
#define SisRTMTempGetString            "SIS_RTM_TEMP_GET"
#define SisRTMTemp1String              "SIS_RTM_TEMP1"
#define SisRTMTemp2String              "SIS_RTM_TEMP2"

#define MAX_PATH_LEN                   32
#define MAX_LOG_STR_LEN                256
#define SIS8300_NUM_CHANNELS           10

#define SIS8300_LOG(p, s, t, r) ({\
	if (t == 0) { \
		snprintf(mSisLogStr, MAX_LOG_STR_LEN, "%s %s::%s: %s", \
				p, driverName, __func__, s); \
	} else if (t == 1) { \
        snprintf(mSisLogStr, MAX_LOG_STR_LEN, "%s %s::%s: %s() failed with '%s' (%d)", \
                p, driverName, __func__, s, sis8300drv_strerror(r), r); \
	} \
	asynPrint(pasynUserSelf, ASYN_TRACE_FLOW, \
			  "%s\n", mSisLogStr); \
	setStringParam(mSISMessage, mSisLogStr); \
	printf("%s\n", mSisLogStr); \
})

#define SIS8300_INF(s) ({\
	SIS8300_LOG("[INF]", s, 0, 0); \
})

#define SIS8300_ERR(s) ({\
	SIS8300_LOG("[ERR]", s, 0, 0); \
})

#define SIS8300DRV_CALL_0(s, x) ({\
	int __ret = x; \
	if (__ret) {\
		SIS8300_LOG("[ERR]", s, 1, __ret); \
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

#if _DBG == 1
	#undef D0
	#define D0(x) { printf("\n"); }
	#undef D
	#define D(x) { \
    printf("[DBG] %s::%s: ", driverName, __func__); \
	x; \
}
#else
	#undef D0
	#define D0(x)
	#undef D
	#define D(x)
#endif
#define E(x) { \
    printf("[ERR] %s::%s: ", driverName, __func__); \
	x; \
}
#define I(x) { \
    printf("[INF] %s::%s: ", driverName, __func__); \
	x; \
}

/** Struck SIS8300 driver */
class epicsShareClass SIS8300 : public asynNDArrayDriver {
public:
	SIS8300(const char *portName, const char *devicePath,
			int maxAddr, int numParams, int numAiSamples, NDDataType_t dataType,
			int maxBuffers, size_t maxMemory, int priority, int stackSize);
	virtual ~SIS8300();

    /* These are the methods that we override from asynNDArrayDriver */
    virtual asynStatus writeInt32(asynUser *pasynUser, epicsInt32 value);
    virtual asynStatus writeFloat64(asynUser *pasynUser, epicsFloat64 value);
    virtual void report(FILE *fp, int details);
    /**< Should be private, but gets called from C, so must be public */
    void sisTask();

protected:
    int mSISAcquire;
    #define SIS8300_FIRST_PARAM mSISAcquire
    int mSISNumAiSamples;
    int mSISClockSource;
    int mSISClockFreq;
    int mSISClockDiv;
    int mSISTrigSource;
    int mSISTrigLine;
    int mSISTrigDo;
    int mSISTrigDelay;
    int mSISTrigRepeat;
    int mSISEnable;
    int mSISConvFactor;
    int mSISConvOffset;
    int mSISAttenuation;
    int mSISDecimFactor;
    int mSISDecimOffset;
    int mSISReset;
    int mSISMessage;
    int mSISFwVersion;
    int mSISSerialNumber;
    int mSISDeviceType;
    int mSISMemorySize;
    int mSISRTMType;
    int mSISRTMTempGet;
    int mSISRTMTemp1;
    int mSISRTMTemp2;

    int mSISDummy;
    #define SIS8300_LAST_PARAM mSISDummy

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

#define SIS8300_NUM_PARAMS ((int)(&SIS8300_LAST_PARAM - &SIS8300_FIRST_PARAM + 1))
