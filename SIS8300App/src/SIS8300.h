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

#define MAX_SIGNALS 10

/** Struck SIS8300 driver; does 1-D waveforms on 10 channels.
  * Inherits from asynNDArrayDriver */
class epicsShareClass SIS8300 : public asynNDArrayDriver {
public:
	SIS8300(const char *portName, int numTimePoints, NDDataType_t dataType,
                   int maxBuffers, size_t maxMemory,
                   int priority, int stackSize);

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

    /* Our data */
    epicsEventId startEventId_;
    epicsEventId stopEventId_;
    int uniqueId_;
    int acquiring_;
    double elapsedTime_;
};


#define NUM_SIS8300_PARAMS ((int)(&LAST_SIS8300_PARAM - &FIRST_SIS8300_PARAM + 1))

