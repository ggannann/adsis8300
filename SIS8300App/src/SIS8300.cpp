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


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <assert.h>

#include <epicsTime.h>
#include <epicsThread.h>
#include <epicsEvent.h>
#include <iocsh.h>

#include "asynNDArrayDriver.h"
#include <epicsExport.h>
#include "SIS8300.h"

static const char *driverName = "SIS8300";

static void sisTaskC(void *drvPvt)
{
    SIS8300 *pPvt = (SIS8300 *)drvPvt;
    pPvt->sisTask();
}

/** Constructor for SIS8300; most parameters are simply passed to ADDriver::ADDriver.
  * After calling the base class constructor this method creates a thread to compute the simulated detector data,
  * and sets reasonable default values for parameters defined in this class, asynNDArrayDriver and ADDriver.
  * \param[in] portName The name of the asyn port driver to be created.
  * \param[in] numTimePoints The initial number of time points.
  * \param[in] dataType The initial data type (NDDataType_t) of the arrays that this driver will create.
  * \param[in] maxBuffers The maximum number of NDArray buffers that the NDArrayPool for this driver is
  *            allowed to allocate. Set this to -1 to allow an unlimited number of buffers.
  * \param[in] maxMemory The maximum amount of memory that the NDArrayPool for this driver is
  *            allowed to allocate. Set this to -1 to allow an unlimited amount of memory.
  * \param[in] priority The thread priority for the asyn port driver thread if ASYN_CANBLOCK is set in asynFlags.
  * \param[in] stackSize The stack size for the asyn port driver thread if ASYN_CANBLOCK is set in asynFlags.
  */
SIS8300::SIS8300(const char *portName, int numTimePoints, NDDataType_t dataType,
                               int maxBuffers, size_t maxMemory, int priority, int stackSize)

    : asynNDArrayDriver(portName, MAX_SIGNALS, NUM_SIS8300_PARAMS, maxBuffers, maxMemory,
    		asynFloat32ArrayMask,
			asynFloat32ArrayMask,
			ASYN_CANBLOCK | ASYN_MULTIDEVICE, /* asyn flags*/
			1,                                /* autoConnect=1 */
			priority,
			stackSize),
    uniqueId_(0), acquiring_(0)

{
    int status = asynSuccess;
    const char *functionName = "SIS8300";

    /* Create the epicsEvents for signaling to the acquisition
     * task when acquisition starts and stops */
    this->startEventId_ = epicsEventCreate(epicsEventEmpty);
    if (!this->startEventId_) {
        printf("%s:%s epicsEventCreate failure for start event\n",
            driverName, functionName);
        return;
    }
    this->stopEventId_ = epicsEventCreate(epicsEventEmpty);
    if (!this->stopEventId_) {
        printf("%s:%s epicsEventCreate failure for stop event\n",
            driverName, functionName);
        return;
    }

    createParam(SisAcquireString,               asynParamInt32, &P_Acquire);
    createParam(SisAcquireTimeString,         asynParamFloat64, &P_AcquireTime);
    createParam(SisElapsedTimeString,         asynParamFloat64, &P_ElapsedTime);
    createParam(SisNumTimePointsString,         asynParamInt32, &P_NumTimePoints);
    createParam(SisCountString,                 asynParamInt32, &P_Count);
    createParam(SisClockSourceString,           asynParamInt32, &P_ClockSource);
    createParam(SisClockFreqString,           asynParamFloat64, &P_ClockFreq);
    createParam(SisClockDivString,              asynParamInt32, &P_ClockDiv);
    createParam(SisTrigSourceString,            asynParamInt32, &P_TrigSource);
    createParam(SisTrigDoString,                asynParamInt32, &P_TrigDo);
    createParam(SisTrigDelayString,             asynParamInt32, &P_TrigDelay);
    createParam(SisTrigRepeatString,            asynParamInt32, &P_TrigRepeat);
    createParam(SisChannelEnableString,         asynParamInt32, &P_Enable);
    createParam(SisChannelDataString,    asynParamFloat32Array, &P_Data);
    createParam(SisChannelConvFactorString,   asynParamFloat64, &P_ConvFactor);
    createParam(SisChannelConvOffsetString,   asynParamFloat64, &P_ConvOffset);
    createParam(SisChannelAttenuationString,  asynParamFloat64, &P_Attenuation);
    createParam(SisChannelDecimFactorString,    asynParamInt32, &P_DecimFactor);
    createParam(SisChannelDecimOffsetString,    asynParamInt32, &P_DecimOffset);

    status |= setIntegerParam(P_NumTimePoints, numTimePoints);
    status |= setIntegerParam(NDDataType, dataType);

    if (status) {
        printf("%s: unable to set parameters\n", functionName);
        return;
    }

    /* Create the thread that updates the images */
    status = (epicsThreadCreate("SisTask",
                                epicsThreadPriorityMedium,
                                epicsThreadGetStackSize(epicsThreadStackMedium),
                                (EPICSTHREADFUNC)sisTaskC,
                                this) == NULL);
    if (status) {
        printf("%s:%s epicsThreadCreate failure for acquisition task\n",
            driverName, functionName);
        return;
    }

	printf("%s:%s: Init done...\n", driverName, functionName);
}

/** Template function to compute the simulated detector data for any data type */
template <typename epicsType> int SIS8300::acquireArraysT()
{
    size_t dims[2];
    int numTimePoints;
    NDDataType_t dataType;
    epicsType *pData;
    double acquireTime;
    
    getIntegerParam(NDDataType, (int *)&dataType);
    getIntegerParam(P_NumTimePoints, &numTimePoints);
    getDoubleParam(P_AcquireTime, &acquireTime);

    dims[0] = MAX_SIGNALS;
    dims[1] = numTimePoints;

    if (this->pArrays[0]) this->pArrays[0]->release();
    this->pArrays[0] = pNDArrayPool->alloc(2, dims, dataType, 0, 0);
    pData = (epicsType *)this->pArrays[0]->pData;
    memset(pData, 0, MAX_SIGNALS * numTimePoints * sizeof(epicsType));

    /* XXX: Implement data acquisition */

    // XXX: Remove once data acquisition added
    this->unlock();
    sleep(2);
    this->lock();

    return 0;
}

/** Computes the new image data */
int SIS8300::acquireArrays()
{
    int dataType;
    getIntegerParam(NDDataType, &dataType); 

    switch (dataType) {
        case NDInt8:
            return acquireArraysT<epicsInt8>();
            break;
        case NDUInt8:
        	return acquireArraysT<epicsUInt8>();
            break;
        case NDInt16:
        	return acquireArraysT<epicsInt16>();
            break;
        case NDUInt16:
        	return acquireArraysT<epicsUInt16>();
            break;
        case NDInt32:
        	return acquireArraysT<epicsInt32>();
            break;
        case NDUInt32:
        	return acquireArraysT<epicsUInt32>();
            break;
        case NDFloat32:
        	return acquireArraysT<epicsFloat32>();
            break;
        case NDFloat64:
        	return acquireArraysT<epicsFloat64>();
            break;
        default:
        	return -1;
        	break;
    }
}

void SIS8300::setAcquire(int value)
{
    if (value && !acquiring_) {
        /* Send an event to wake up the simulation task */
        epicsEventSignal(this->startEventId_); 
    }
    if (!value && acquiring_) {
        /* This was a command to stop acquisition */
        /* Send the stop event */
        epicsEventSignal(this->stopEventId_); 
    }
}

/** This thread calls computeImage to compute new image data and does the callbacks to send it to higher layers.
  * It implements the logic for single, multiple or continuous acquisition. */
void SIS8300::sisTask()
{
    int status = asynSuccess;
    NDArray *pImage;
    epicsTimeStamp startTime;
    epicsTimeStamp frameTime;
//    int numTimePoints;
    int arrayCounter;
//    double timeStep;
    int i;
    double elapsed;
    const char *functionName = "sisTask";

	sleep(1);

    this->lock();

//	if (mHandle == -1) {
//		printf("%s:%s: Data thread will not start...\n", driverName, functionName);
//		this->unlock();
//		return;
//	}

	printf("%s:%s: Data thread started...\n", driverName, functionName);

	startTime.secPastEpoch = 0;
	startTime.nsec = 0;

	/* Loop forever */
    while (1) {
        /* Has acquisition been stopped? */
        status = epicsEventTryWait(this->stopEventId_);
        if (status == epicsEventWaitOK) {
            acquiring_ = 0;
            startTime.secPastEpoch = 0;
            startTime.nsec = 0;
        }
        printf("%s: 1 Acquring = %d..\n", __func__, acquiring_);
       
        /* If we are not acquiring then wait for a semaphore that is given when acquisition is started */
        if (!acquiring_) {
          /* Release the lock while we wait for an event that says acquire has started, then lock again */
            asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW,
                "%s:%s: waiting for acquire to start\n", driverName, functionName);
            this->unlock();
            status = epicsEventWait(this->startEventId_);
            this->lock();
            acquiring_ = 1;
            elapsedTime_ = 0.0;
        }
        printf("%s: 2 Acquring = %d..\n", __func__, acquiring_);

        /* Get the data */
        acquireArrays();

        pImage = this->pArrays[0];

        /* Put the frame number and time stamp into the buffer */
        pImage->uniqueId = uniqueId_++;
        getIntegerParam(NDArrayCounter, &arrayCounter);
        arrayCounter++;
        setIntegerParam(NDArrayCounter, arrayCounter);
        epicsTimeGetCurrent(&frameTime);
        pImage->timeStamp = frameTime.secPastEpoch + frameTime.nsec / 1.e9;
        updateTimeStamp(&pImage->epicsTS);
        if (startTime.secPastEpoch == 0) {
        	startTime = frameTime;
        }
        elapsed = (double)(frameTime.secPastEpoch + frameTime.nsec / 1.e9) -
        		(double)(startTime.secPastEpoch + startTime.nsec / 1.e9);
        setDoubleParam(P_ElapsedTime, elapsed);

        /* Get any attributes that have been defined for this driver */
        this->getAttributes(pImage->pAttributeList);

        /* Call the NDArray callback */
        /* Must release the lock here, or we can get into a deadlock, because we can
         * block on the plugin lock, and the plugin can be calling us */
        this->unlock();
        doCallbacksGenericPointer(pImage, NDArrayData, 0);
        this->lock();

        /* Call the callbacks to update any changes */
        for (i=0; i<MAX_SIGNALS; i++) {
            callParamCallbacks(i);
        }
    }

	printf("Data thread is down!\n");
}

/** Called when asyn clients call pasynInt32->write().
  * This function performs actions for some parameters.
  * For all parameters it sets the value in the parameter library and calls any registered callbacks..
  * \param[in] pasynUser pasynUser structure that encodes the reason and address.
  * \param[in] value Value to write. */
asynStatus SIS8300::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    int function = pasynUser->reason;
    int addr;
    asynStatus status = asynSuccess;

    getAddress(pasynUser, &addr);
    printf("%s: ENTER %d (%d) = %d\n", __func__, function, addr, value);
 
    /* Set the parameter and readback in the parameter library.  This may be overwritten when we read back the
     * status at the end, but that's OK */
    status = setIntegerParam(addr, function, value);

    if (function == P_Acquire) {
        setAcquire(value);
    } else {
        /* If this parameter belongs to a base class call its method */
        if (function < FIRST_SIS8300_PARAM) status = asynNDArrayDriver::writeInt32(pasynUser, value);
    }

    /* Do callbacks so higher layers see any changes */
    callParamCallbacks(addr);

    if (status)
        asynPrint(pasynUser, ASYN_TRACE_ERROR,
              "%s:writeInt32 error, status=%d function=%d, value=%d\n",
              driverName, status, function, value);
    else
        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
              "%s:writeInt32: function=%d, value=%d\n",
              driverName, function, value);
    return status;
}

/** Called when asyn clients call pasynFloat64->write().
  * This function performs actions for some parameters.
  * For all parameters it sets the value in the parameter library and calls any registered callbacks..
  * \param[in] pasynUser pasynUser structure that encodes the reason and address.
  * \param[in] value Value to write. */
asynStatus SIS8300::writeFloat64(asynUser *pasynUser, epicsFloat64 value)
{
    int function = pasynUser->reason;
    int addr;
    asynStatus status = asynSuccess;

    getAddress(pasynUser, &addr);
    printf("%s: ENTER %d (%d) = %f\n", __func__, function, addr, value);

    /* Set the parameter and readback in the parameter library.  This may be overwritten when we read back the
     * status at the end, but that's OK */
    status = setDoubleParam(addr, function, value);

    if (function == P_ClockFreq) {
        // XXX: Add code
    } else {
        /* If this parameter belongs to a base class call its method */
        if (function < FIRST_SIS8300_PARAM) status = asynNDArrayDriver::writeFloat64(pasynUser, value);
    }

    /* Do callbacks so higher layers see any changes */
    callParamCallbacks(addr);

    if (status)
        asynPrint(pasynUser, ASYN_TRACE_ERROR,
              "%s:writeFloat64 error, status=%d function=%d, value=%f\n",
              driverName, status, function, value);
    else
        asynPrint(pasynUser, ASYN_TRACEIO_DRIVER,
              "%s:writeFloat64: function=%d, value=%f\n",
              driverName, function, value);
    return status;
}

/** Report status of the driver.
  * Prints details about the driver if details>0.
  * It then calls the ADDriver::report() method.
  * \param[in] fp File pointed passed by caller where the output is written to.
  * \param[in] details If >0 then driver details are printed.
  */
void SIS8300::report(FILE *fp, int details)
{

    fprintf(fp, "Struck SIS8300 %s\n", this->portName);
    if (details > 0) {
        int numTimePoints, dataType;
        getIntegerParam(P_NumTimePoints, &numTimePoints);
        getIntegerParam(NDDataType, &dataType);
        fprintf(fp, "  # time points:   %d\n", numTimePoints);
        fprintf(fp, "      Data type:   %d\n", dataType);
    }
    /* Invoke the base class method */
    asynNDArrayDriver::report(fp, details);
}


/** Configuration command, called directly or from iocsh */
extern "C" int SIS8300Config(const char *portName, int numTimePoints, int dataType,
                                 int maxBuffers, int maxMemory, int priority, int stackSize)
{
    new SIS8300(portName, numTimePoints, (NDDataType_t)dataType,
                    (maxBuffers < 0) ? 0 : maxBuffers,
                    (maxMemory < 0) ? 0 : maxMemory, 
                    priority, stackSize);
    return(asynSuccess);
}

/** Code for iocsh registration */
static const iocshArg SIS8300ConfigArg0 = {"Port name",     iocshArgString};
static const iocshArg SIS8300ConfigArg1 = {"# time points", iocshArgInt};
static const iocshArg SIS8300ConfigArg2 = {"Data type",     iocshArgInt};
static const iocshArg SIS8300ConfigArg3 = {"maxBuffers",    iocshArgInt};
static const iocshArg SIS8300ConfigArg4 = {"maxMemory",     iocshArgInt};
static const iocshArg SIS8300ConfigArg5 = {"priority",      iocshArgInt};
static const iocshArg SIS8300ConfigArg6 = {"stackSize",     iocshArgInt};
static const iocshArg * const SIS8300ConfigArgs[] = {&SIS8300ConfigArg0,
                                                            &SIS8300ConfigArg1,
                                                            &SIS8300ConfigArg2,
                                                            &SIS8300ConfigArg3,
                                                            &SIS8300ConfigArg4,
                                                            &SIS8300ConfigArg5,
                                                            &SIS8300ConfigArg6};
static const iocshFuncDef configSIS8300 = {"SIS8300Config", 7, SIS8300ConfigArgs};
static void configSIS8300CallFunc(const iocshArgBuf *args)
{
    SIS8300Config(args[0].sval, args[1].ival, args[2].ival, args[3].ival,
                         args[4].ival, args[5].ival, args[6].ival);
}


static void SIS8300Register(void)
{

    iocshRegister(&configSIS8300, configSIS8300CallFunc);
}

extern "C" {
epicsExportRegistrar(SIS8300Register);
}
