/* SIS8300.cpp
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


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <assert.h>
#include <string>

#include <epicsTime.h>
#include <epicsThread.h>
#include <epicsEvent.h>
#include <epicsExit.h>
#include <iocsh.h>

#include <asynNDArrayDriver.h>
#include <epicsExport.h>

#include <ADSIS8300.h>

static const char *driverName = "ADSIS8300";

/**
 * Exit handler, delete the ADSIS8300 object.
 */
static void exitHandler(void *drvPvt) {
	ADSIS8300 *pPvt = (ADSIS8300 *) drvPvt;
	delete pPvt;
}

static void sisTaskC(void *drvPvt)
{
    ADSIS8300 *pPvt = (ADSIS8300 *)drvPvt;
    pPvt->sisTask();
}

/** Constructor for SIS8300; most parameters are simply passed to ADDriver::ADDriver.
  * After calling the base class constructor this method creates a thread to compute the simulated detector data,
  * and sets reasonable default values for parameters defined in this class, asynNDArrayDriver and ADDriver.
  * \param[in] portName The name of the asyn port driver to be created.
  * \param[in] devicePath The path to the /dev entry.
  * \param[in] maxAddr The maximum  number of asyn addr addresses this driver supports. 1 is minimum.
  * \param[in] numParams The number of parameters in the derived class.
  * \param[in] numAiSamples The initial number of AI samples.
  * \param[in] dataType The initial data type (NDDataType_t) of the arrays that this driver will create.
  * \param[in] maxBuffers The maximum number of NDArray buffers that the NDArrayPool for this driver is
  *            allowed to allocate. Set this to -1 to allow an unlimited number of buffers.
  * \param[in] maxMemory The maximum amount of memory that the NDArrayPool for this driver is
  *            allowed to allocate. Set this to -1 to allow an unlimited amount of memory.
  * \param[in] priority The thread priority for the asyn port driver thread if ASYN_CANBLOCK is set in asynFlags.
  * \param[in] stackSize The stack size for the asyn port driver thread if ASYN_CANBLOCK is set in asynFlags.
  */
ADSIS8300::ADSIS8300(const char *portName, const char *devicePath,
		int maxAddr, int numParams, int numAiSamples, NDDataType_t dataType,
		int maxBuffers, size_t maxMemory, int priority, int stackSize)

    : asynNDArrayDriver(portName,
    		maxAddr,
    		NUM_SIS8300_PARAMS+numParams,
			maxBuffers, maxMemory,
    		asynFloat32ArrayMask,
			asynFloat32ArrayMask,
			ASYN_CANBLOCK | ASYN_MULTIDEVICE, /* asyn flags*/
			1,                                /* autoConnect=1 */
			priority,
			stackSize),
    uniqueId_(0), acquiring_(0)

{
    int status = asynSuccess;

    D(printf("%d addresses, %d parameters\n", maxAddr, NUM_SIS8300_PARAMS+numParams));

    mRawDataArray = NULL;
    mNumArrays = 1;

	/* Create an EPICS exit handler */
	epicsAtExit(exitHandler, this);


    snprintf(mSisDevicePath, MAX_PATH_LEN, "%s", devicePath);
    mSisDevice = (sis8300drv_usr*)calloc(1, sizeof(sis8300drv_usr));
    mSisDevice->file = mSisDevicePath;

    /* Create the epicsEvents for signaling to the acquisition
     * task when acquisition starts and stops */
    this->startEventId_ = epicsEventCreate(epicsEventEmpty);
    if (!this->startEventId_) {
        E(printf("epicsEventCreate failure for start event\n"));
        return;
    }
    this->stopEventId_ = epicsEventCreate(epicsEventEmpty);
    if (!this->stopEventId_) {
        E(printf("epicsEventCreate failure for stop event\n"));
        return;
    }

    createParam(SisAcquireString,               asynParamInt32, &P_Acquire);
    createParam(SisAcquireTimeString,         asynParamFloat64, &P_AcquireTime);
    createParam(SisElapsedTimeString,         asynParamFloat64, &P_ElapsedTime);
    createParam(SisTimeStepString,            asynParamFloat64, &P_TimeStep);
    createParam(SisNumAiSamplesString,         asynParamInt32, &P_NumAiSamples);
    createParam(SisClockSourceString,           asynParamInt32, &P_ClockSource);
    createParam(SisClockFreqString,           asynParamFloat64, &P_ClockFreq);
    createParam(SisClockDivString,              asynParamInt32, &P_ClockDiv);
    createParam(SisTrigSourceString,            asynParamInt32, &P_TrigSource);
    createParam(SisTrigLineString,              asynParamInt32, &P_TrigLine);
    createParam(SisTrigDoString,                asynParamInt32, &P_TrigDo);
    createParam(SisTrigDelayString,             asynParamInt32, &P_TrigDelay);
    createParam(SisTrigRepeatString,            asynParamInt32, &P_TrigRepeat);
    createParam(SisChannelEnableString,         asynParamInt32, &P_Enable);
    createParam(SisChannelConvFactorString,   asynParamFloat64, &P_ConvFactor);
    createParam(SisChannelConvOffsetString,   asynParamFloat64, &P_ConvOffset);
    createParam(SisChannelAttenuationString,  asynParamFloat64, &P_Attenuation);
    createParam(SisChannelDecimFactorString,    asynParamInt32, &P_DecimFactor);
    createParam(SisChannelDecimOffsetString,    asynParamInt32, &P_DecimOffset);
    createParam(SisResetString,                 asynParamInt32, &P_Reset);
    createParam(SisMessageString,               asynParamOctet, &P_Message);
    createParam(SisFirmwareVersionString,       asynParamInt32, &P_FirmwareVersion);
    createParam(SisSerialNumberString,          asynParamInt32, &P_SerialNumber);
    createParam(SisMemorySizeString,            asynParamInt32, &P_MemorySize);
    createParam(SisDeviceTypeString,            asynParamInt32, &P_DeviceType);
    createParam(SisRTMTypeString,               asynParamInt32, &P_RTMType);
    createParam(SisRTMTemp1String,            asynParamFloat64, &P_RTMTemp1);
    createParam(SisRTMTemp2String,            asynParamFloat64, &P_RTMTemp2);
    createParam(SisRTMTempGetString,            asynParamInt32, &P_RTMTempGet);

    status |= setIntegerParam(P_NumAiSamples, numAiSamples);
    status |= setIntegerParam(NDDataType, dataType);
    status |= setDoubleParam(P_TimeStep, 0.001);
    status |= setIntegerParam(P_Acquire, 0);
    status |= setIntegerParam(P_FirmwareVersion, 0);
    status |= setIntegerParam(P_SerialNumber, 0);
    status |= setIntegerParam(P_MemorySize, 0);
    status |= setIntegerParam(P_DeviceType, 0);
    status |= setIntegerParam(P_RTMType, 0);
    status |= setDoubleParam(P_RTMTemp1, 0);
    status |= setDoubleParam(P_RTMTemp2, 0);
    ADSIS8300_INF("No error");

    if (status) {
        E(printf("unable to set parameters\n"));
        return;
    }

    /* Create the thread that updates the images */
    status = (epicsThreadCreate("SisTask",
                                epicsThreadPriorityHigh,
                                epicsThreadGetStackSize(epicsThreadStackMedium),
                                (EPICSTHREADFUNC)sisTaskC,
                                this) == NULL);
    if (status) {
        E(printf("epicsThreadCreate failure for acquisition task\n"));
        return;
    }

    this->lock();
    initDevice();
    this->unlock();

	I(printf("Init done...\n"));
}

ADSIS8300::~ADSIS8300() {
	D(printf("Shutdown and freeing up memory...\n"));

	this->lock();
	D(printf("Data thread is already down!\n"));
	destroyDevice();
	free(mSisDevice);
	this->unlock();
	I(printf("Shutdown complete!\n"));
}

/** Template function to compute the simulated detector data for any data type */
int ADSIS8300::acquireRawArrays()
{
    size_t dims[2];
    int numAiSamples;
    epicsUInt16 *pRaw, *pChRaw;
    int aich, i;

	D(printf("Enter\n"));

    getIntegerParam(P_NumAiSamples, &numAiSamples);

    /* raw data samples of a given channel are stored in sequence */
    dims[0] = numAiSamples;
    dims[1] = ADSIS8300_NUM_CHANNELS;

    /* local NDArray is for raw data samples */
    if (mRawDataArray) {
    	mRawDataArray->release();
    }
    mRawDataArray = pNDArrayPool->alloc(2, dims, NDUInt16, 0, 0);
    pRaw = (epicsUInt16 *)mRawDataArray->pData;
    memset(pRaw, 0, ADSIS8300_NUM_CHANNELS * numAiSamples * sizeof(epicsUInt16));

    for (aich = 0; aich < ADSIS8300_NUM_CHANNELS; aich++) {
        if (!(mChannelMask & (1 << aich))) {
            continue;
        }
    	pChRaw = pRaw + (aich * numAiSamples);

		SIS8300DRV_CALL_RET("sis8300drv_read_ai", sis8300drv_read_ai(mSisDevice, aich, pChRaw));

		/* XXX DEBUG */
//		char fname[32];
//		sprintf(fname, "/tmp/raw_%d.txt", aich);
//		FILE *fp = fopen(fname, "w");
		D(printf("CH %d [%d]: ", aich, numAiSamples));
		for (i = 0; i < numAiSamples; i++) {
//			printf("%u ", *(pChRaw + i));
//		fprintf(fp, "%u\n", *(pChRaw + i));
		}
		D0(printf("\n"));
//		fclose(fp);
    }

    return 0;
}

/** Template function to compute the simulated detector data for any data type */
template <typename epicsType> int ADSIS8300::convertArraysT()
{
    size_t dims[2];
    int numAiSamples;
    NDDataType_t dataType;
    epicsType *pData, *pVal;
    epicsUInt16 *pRaw, *pChRaw;
    int aich, i;
    double convFactor, convOffset;
    
	D(printf("Enter\n"));

    getIntegerParam(NDDataType, (int *)&dataType);
    getIntegerParam(P_NumAiSamples, &numAiSamples);

    /* local NDArray is for raw AI data samples */
    if (! mRawDataArray) {
    	return -1;
    }
    pRaw = (epicsUInt16 *)mRawDataArray->pData;

    /* converted AI data samples of all channel are interleaved */
    dims[0] = ADSIS8300_NUM_CHANNELS;
    dims[1] = numAiSamples;

    /* 0th NDArray is for converted AI data samples */
    if (this->pArrays[0]) {
    	this->pArrays[0]->release();
    }
    this->pArrays[0] = pNDArrayPool->alloc(2, dims, dataType, 0, 0);
    pData = (epicsType *)this->pArrays[0]->pData;
    memset(pData, 0, ADSIS8300_NUM_CHANNELS * numAiSamples * sizeof(epicsType));

    for (aich = 0; aich < ADSIS8300_NUM_CHANNELS; aich++) {
    	if (!(mChannelMask & (1 << aich))) {
            continue;
        }
    	pChRaw = pRaw + (aich * numAiSamples);
    	pVal = pData + aich;

		getDoubleParam(aich, P_ConvFactor, &convFactor);
		getDoubleParam(aich, P_ConvOffset, &convOffset);

//		char fname[32];
//		sprintf(fname, "/tmp/%d.txt", aich);
//		FILE *fp = fopen(fname, "w");
		D(printf("CH %d [%d] CF %f, CO %f: ", aich, numAiSamples, convFactor, convOffset));
		for (i = 0; i < numAiSamples; i++) {
			*pVal = (epicsType)((double)*(pChRaw + i) * convFactor + convOffset);
//			printf("%f ", (double)*pVal);
//			fprintf(fp, "%f\n", (double)*pVal);
			pVal += ADSIS8300_NUM_CHANNELS;
		}
		D0(printf("\n"));
//		fclose(fp);
    }

    return 0;
}

/** Computes the new image data */
int ADSIS8300::acquireArrays()
{
    int dataType;
    int ret;

	D(printf("Enter\n"));

    ret = acquireRawArrays();
    if (ret) {
    	return ret;
    }

    getIntegerParam(NDDataType, &dataType); 
    switch (dataType) {
        case NDInt8:
            return convertArraysT<epicsInt8>();
            break;
        case NDUInt8:
        	return convertArraysT<epicsUInt8>();
            break;
        case NDInt16:
        	return convertArraysT<epicsInt16>();
            break;
        case NDUInt16:
        	return convertArraysT<epicsUInt16>();
            break;
        case NDInt32:
        	return convertArraysT<epicsInt32>();
            break;
        case NDUInt32:
        	return convertArraysT<epicsUInt32>();
            break;
        case NDFloat32:
        	return convertArraysT<epicsFloat32>();
            break;
        case NDFloat64:
        	return convertArraysT<epicsFloat64>();
            break;
        default:
        	return -1;
        	break;
    }
}

void ADSIS8300::setAcquire(int value)
{
    if (value && !acquiring_) {
        /* Send an event to wake up the simulation task */
        epicsEventSignal(this->startEventId_); 
    }
    if (!value && acquiring_) {
        /* This was a command to stop acquisition */
        /* Send the stop event */

    	disarmDevice();

        epicsEventSignal(this->stopEventId_); 
    }
}

int ADSIS8300::initDeviceDone()
{
	int ret = 0;

	D(printf("Enter\n"));

	return ret;
}

int ADSIS8300::armDevice()
{
	int ret;

	D(printf("Enter\n"));

	ret = SIS8300DRV_CALL("sis8300drv_arm_device", sis8300drv_arm_device(mSisDevice));

	return ret;
}

int ADSIS8300::disarmDevice()
{
	int ret;

	D(printf("Enter\n"));

	ret = SIS8300DRV_CALL("sis8300drv_disarm_device", sis8300drv_disarm_device(mSisDevice));
	if (ret) {
//		return ret;
	}
	ret = SIS8300DRV_CALL("sis8300drv_release_irq", sis8300drv_release_irq(mSisDevice, irq_type_usr));

	return ret;
}

int ADSIS8300::waitForDevice()
{
	int ret;

	D(printf("Enter\n"));

   	ret = SIS8300DRV_CALL("sis8300drv_wait_acq_end", sis8300drv_wait_acq_end(mSisDevice));

	return ret;
}

int ADSIS8300::deviceDone()
{
	int ret = 0;

	D(printf("Enter\n"));

	return ret;
}

int ADSIS8300::updateParameters()
{
	int ret = 0;

	D(printf("Enter\n"));

	return ret;
}

/** This thread calls computeImage to compute new image data and does the callbacks to send it to higher layers.
  * It implements the logic for single, multiple or continuous acquisition. */
void ADSIS8300::sisTask()
{
    int status = asynSuccess;
    NDArray *pData;
    epicsTimeStamp frameTime;
//    int numAiSamples;
    int arrayCounter;
//    double timeStep;
    int i, a;
//    int trgSource;
    int trgRepeat;
    int trgCount;
//    double acquireTime;
    int ret;

	sleep(1);

    this->lock();

	if (!sis8300drv_is_device_open(mSisDevice)) {
		E(printf("No SIS8300 device - data thread will not start...\n"));
		this->unlock();
		return;
	}

	I(printf("Data thread started...\n"));

	trgCount = 0;

	/* Loop forever */
    while (1) {

taskStart:
//		callParamCallbacks(0);

		D(printf("Calling update parameters..\n"));
		ret = updateParameters();
		if (ret) {
			acquiring_ = 0;
			setIntegerParam(P_Acquire, 0);
		//			this->unlock();
		//			break;
//			goto taskStart;
		}

		D(printf("0 Acquiring = %d..\n", acquiring_));

        /* Has acquisition been stopped? */
        status = epicsEventTryWait(this->stopEventId_);
        if (status == epicsEventWaitOK) {
        	D(printf("1 Acquiring = %d..\n", acquiring_));
        	trgCount = 0;
            acquiring_ = 0;
        }

        if (acquiring_) {
        	D(printf("2 Acquiring = %d..\n", acquiring_));
			getIntegerParam(P_TrigRepeat, &trgRepeat);
			/* Stop the acquisition if set number of triggers has been reached */
			if (trgRepeat == 0) {
				acquiring_ = 0;
				setIntegerParam(P_Acquire, 0);
			} else if ((trgRepeat > 0) && (trgCount >= trgRepeat)) {
				acquiring_ = 0;
				setIntegerParam(P_Acquire, 0);
			}
        }
       
        /* If we are not acquiring then wait for a semaphore that is given when acquisition is started */
        if (!acquiring_) {
        	D(printf("2a Acquiring = %d..\n", acquiring_));
//        	ret = disarmDevice();

        	callParamCallbacks(0);
          /* Release the lock while we wait for an event that says acquire has started, then lock again */
            asynPrint(this->pasynUserSelf, ASYN_TRACE_FLOW,
                "%s::%s: waiting for acquire to start\n", driverName, __func__);

            D(printf("2b Acquiring = %d..\n", acquiring_));
            this->unlock();
            status = epicsEventWait(this->startEventId_);
            this->lock();
            acquiring_ = 1;
            elapsedTime_ = 0.0;
        	trgCount = 0;
        	D(printf("2c Acquiring = %d..\n", acquiring_));

    		ret = initDeviceDone();
    		if (ret) {
    			acquiring_ = 0;
    			setIntegerParam(P_Acquire, 0);
    			this->unlock();
    			break;
    		}
    		D(printf("2d Acquiring = %d..\n", acquiring_));
        }

        D(printf("3 Acquiring = %d..\n", acquiring_));
		ret = armDevice();
		if (ret) {
			acquiring_ = 0;
			setIntegerParam(P_Acquire, 0);
//			this->unlock();
//			break;
			goto taskStart;
		}
		callParamCallbacks(0);

		/* Unlock while waiting for the device. */
		this->unlock();

		D(printf("4 Acquiring = %d..\n", acquiring_));
       	ret = waitForDevice();
		if (ret) {
			/* Lock after the wait failed. */
			this->lock();
			acquiring_ = 0;
			setIntegerParam(P_Acquire, 0);
//			this->unlock();
//			break;
			goto taskStart;
		}

		/* Lock after the device has finished acquisition. */
		this->lock();

		D(printf("5 Acquiring = %d..\n", acquiring_));
       	ret = deviceDone();
		if (ret) {
			acquiring_ = 0;
			setIntegerParam(P_Acquire, 0);
//			this->unlock();
//			break;
			goto taskStart;
		}

        /* Trigger arrived */
        trgCount++;
        D(printf("6 Acquiring = %d..\n", acquiring_));

        /* Get the data */
        ret = acquireArrays();
		if (ret) {
			acquiring_ = 0;
			setIntegerParam(P_Acquire, 0);
//			this->unlock();
//			break;
			goto taskStart;
		}
		D(printf("7 Acquiring = %d..\n", acquiring_));

        ADSIS8300_INF("No error");

        epicsTimeGetCurrent(&frameTime);
        getIntegerParam(NDArrayCounter, &arrayCounter);
        arrayCounter++;
        setIntegerParam(NDArrayCounter, arrayCounter);
        for (a = 0; a < mNumArrays; a++) {
            if (! this->pArrays[a]) {
            	continue;
            }
            pData = this->pArrays[a];

            /* Put the frame number and time stamp into the buffer */
            pData->uniqueId = uniqueId_++;
            pData->timeStamp = frameTime.secPastEpoch + frameTime.nsec / 1.e9;
            updateTimeStamp(&pData->epicsTS);

            /* Get any attributes that have been defined for this driver */
            this->getAttributes(pData->pAttributeList);

            /* Call the NDArray callback */
            /* Must release the lock here, or we can get into a deadlock, because we can
             * block on the plugin lock, and the plugin can be calling us */
            this->unlock();
            doCallbacksGenericPointer(pData, NDArrayData, a);
            this->lock();
        }
//
//        pData = this->pArrays[1];
//
//        /* Put the frame number and time stamp into the buffer */
//        pData->uniqueId = uniqueId_++;
//        getIntegerParam(NDArrayCounter, &arrayCounter);
//        arrayCounter++;
//        setIntegerParam(NDArrayCounter, arrayCounter);
//        epicsTimeGetCurrent(&frameTime);
//        pData->timeStamp = frameTime.secPastEpoch + frameTime.nsec / 1.e9;
//        updateTimeStamp(&pData->epicsTS);
//
//        /* Get any attributes that have been defined for this driver */
//        this->getAttributes(pData->pAttributeList);
//
//        /* Call the NDArray callback */
//        /* Must release the lock here, or we can get into a deadlock, because we can
//         * block on the plugin lock, and the plugin can be calling us */
//        this->unlock();
//        doCallbacksGenericPointer(pData, NDArrayData, 0);
//        this->lock();

        /* Call the callbacks to update any changes */
        for (i=0; i<maxAddr; i++) {
            callParamCallbacks(i);
        }

//		getIntegerParam(P_numAiSamples, &numAiSamples);
//        getDoubleParam(P_TimeStep, &timeStep);
//        getDoubleParam(P_AcquireTime, &acquireTime);
//        elapsedTime_ += (timeStep * numAiSamples);
//        setDoubleParam(P_ElapsedTime, elapsedTime_);
//        if ((acquireTime > 0) && (elapsedTime_ > acquireTime)) {
//            setAcquire(0);
//            setIntegerParam(P_Acquire, 0);
//            //break;
//        }
//        callParamCallbacks(0);

//		getIntegerParam(P_TrigSource, &trgSource);
//        if ((sis8300drv_trg_src)trgSource == trg_src_soft) {
//			/* XXX: Sleep for numTimePoint * timeStep seconds */
//			this->unlock();
//			epicsThreadSleep(numAiSamples * timeStep);
//			this->lock();
//        }
    }

    callParamCallbacks(0);
	I(printf("Data thread is down!\n"));
	sleep(1);
}

/** Called when asyn clients call pasynInt32->write().
  * This function performs actions for some parameters.
  * For all parameters it sets the value in the parameter library and calls any registered callbacks..
  * \param[in] pasynUser pasynUser structure that encodes the reason and address.
  * \param[in] value Value to write. */
asynStatus ADSIS8300::writeInt32(asynUser *pasynUser, epicsInt32 value)
{
    int function = pasynUser->reason;
    int addr;
    int ret;
    asynStatus status = asynSuccess;

    getAddress(pasynUser, &addr);
    D(printf("Enter %d (%d) = %d\n", function, addr, value));
 
    /* Set the parameter and readback in the parameter library.  This may be overwritten when we read back the
     * status at the end, but that's OK */
    status = setIntegerParam(addr, function, value);

    if (function == P_Acquire) {
        setAcquire(value);
    } else if (function == P_ClockSource) {
   		ret = SIS8300DRV_CALL("sis8300drv_set_clock_source", sis8300drv_set_clock_source(mSisDevice, (sis8300drv_clk_src)value));
   		if (ret) {
   			status = asynError;
    	}
    } else if (function == P_ClockDiv) {
		ret = SIS8300DRV_CALL("sis8300drv_set_clock_divider", sis8300drv_set_clock_divider(mSisDevice, (sis8300drv_clk_div)value));
		if (ret) {
			status = asynError;
		}
    } else if (function == P_TrigSource) {
   		ret = SIS8300DRV_CALL("sis8300drv_set_trigger_source", sis8300drv_set_trigger_source(mSisDevice, (sis8300drv_trg_src)value));
		if (ret) {
			status = asynError;
		}
    } else if (function == P_TrigLine) {
		sis8300drv_trg_ext trgext = trg_ext_harlink;
		unsigned int trgmask = (1 << value);
		if (value >= SIS8300DRV_NUM_FP_TRG) {
			trgext = trg_ext_mlvds;
			trgmask = (1 << (value - SIS8300DRV_NUM_FP_TRG));
		}
   		ret = SIS8300DRV_CALL("sis8300drv_set_external_setup", sis8300drv_set_external_setup(mSisDevice, trgext, trgmask, 0));
		if (ret) {
			status = asynError;
		}
    } else if (function == P_TrigDelay) {
   		ret = SIS8300DRV_CALL("sis8300drv_set_npretrig", sis8300drv_set_npretrig(mSisDevice, value));
		if (ret) {
			status = asynError;
		}
    } else if (function == P_TrigDo) {
    	setAcquire(1);
    } else if (function == P_NumAiSamples) {
   		ret = SIS8300DRV_CALL("sis8300drv_set_nsamples", sis8300drv_set_nsamples(mSisDevice, value));
		if (ret) {
			status = asynError;
		}
    } else if (function == P_Enable) {
    	if (value) {
    		enableChannel(addr);
    	} else {
    		disableChannel(addr);
    	}
   		ret = SIS8300DRV_CALL("sis8300drv_set_channel_mask", sis8300drv_set_channel_mask(mSisDevice, mChannelMask));
		if (ret) {
			status = asynError;
		}
    } else if (function == P_Reset) {
   		ret = SIS8300DRV_CALL("sis8300drv_master_reset", sis8300drv_master_reset(mSisDevice));
		if (ret) {
			status = asynError;
		}
    } else if (function == P_RTMTempGet) {
		int RTMType = 0;
		getIntegerParam(P_RTMType, &RTMType);
		/* Only DWC8VM1 has attenuators */
		if ((sis8300drv_rtm)RTMType == rtm_dwc8vm1) {
			double temp;
			ret = SIS8300DRV_CALL("sis8300drv_i2c_rtm_temperature_get", sis8300drv_i2c_rtm_temperature_get(mSisDevice, (sis8300drv_rtm)RTMType, rtm_temp_ad8363, &temp));
			if (ret) {
				status = asynError;
			}
			usleep(200000);
			setDoubleParam(P_RTMTemp1, temp);
			ret = SIS8300DRV_CALL("sis8300drv_i2c_rtm_temperature_get", sis8300drv_i2c_rtm_temperature_get(mSisDevice, (sis8300drv_rtm)RTMType, rtm_temp_ltc2493, &temp));
			if (ret) {
				status = asynError;
			}
			setDoubleParam(P_RTMTemp2, temp);
		}
    } else {
        /* If this parameter belongs to a base class call its method */
        if (function < FIRST_SIS8300_PARAM) {
        	status = asynNDArrayDriver::writeInt32(pasynUser, value);
        }
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
asynStatus ADSIS8300::writeFloat64(asynUser *pasynUser, epicsFloat64 value)
{
    int function = pasynUser->reason;
    int addr;
    int ret;
    asynStatus status = asynSuccess;

    getAddress(pasynUser, &addr);
    D(printf("Enter %d (%d) = %f\n", function, addr, value));

    /* Set the parameter and readback in the parameter library.  This may be overwritten when we read back the
     * status at the end, but that's OK */
    status = setDoubleParam(addr, function, value);

    if (function == P_ClockFreq) {
		sis8300drv_clk_div clkdiv = 250000000.0 / value;
		ret = SIS8300DRV_CALL("sis8300drv_set_clock_divider", sis8300drv_set_clock_divider(mSisDevice, clkdiv));
		if (ret) {
			status = asynError;
		}
    } else if (function == P_Attenuation) {
		int RTMType = 0;
		getIntegerParam(P_RTMType, &RTMType);
		/* Only DWC8VM1 has attenuators */
		if ((sis8300drv_rtm)RTMType == rtm_dwc8vm1) {
			int val = (int)((value + 31.5) * 2);
			ret = SIS8300DRV_CALL("sis8300drv_i2c_rtm_attenuator_set", sis8300drv_i2c_rtm_attenuator_set(mSisDevice, (sis8300drv_rtm)RTMType, addr, val));
			if (ret) {
				status = asynError;
			}
		}
    } else {
        /* If this parameter belongs to a base class call its method */
        if (function < FIRST_SIS8300_PARAM) {
        	status = asynNDArrayDriver::writeFloat64(pasynUser, value);
        }
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
void ADSIS8300::report(FILE *fp, int details)
{
    int deviceType;
    int firmwareVersion;
    int memorySizeMb;
    int serialNumber;

    fprintf(fp, "Struck           : %s\n", this->portName);
    fprintf(fp, "Device path      : %s\n", mSisDevicePath);
	getIntegerParam(P_FirmwareVersion, &firmwareVersion);
	getIntegerParam(P_SerialNumber, &serialNumber);
	getIntegerParam(P_MemorySize, &memorySizeMb);
	getIntegerParam(P_DeviceType, &deviceType);
	fprintf(fp,
			"Device type      : %X\n"
			"Serial number    : %d\n"
			"Firmware version : 0x%4X\n"
			"Memory size      : %d MB\n",
			deviceType,
			serialNumber,
			firmwareVersion,
			memorySizeMb);
    if (details > 0) {
        int numAiSamples, dataType;
        getIntegerParam(P_NumAiSamples, &numAiSamples);
        getIntegerParam(NDDataType, &dataType);
        fprintf(fp, "  # samples:       %d\n", numAiSamples);
        fprintf(fp, "      Data type:   %d\n", dataType);
    }
    /* Invoke the base class method */
    asynNDArrayDriver::report(fp, details);
}

int ADSIS8300::initDevice()
{
    unsigned int deviceType;
    unsigned int firmwareVersion;
    unsigned long memorySizeMb;
    unsigned int serialNumber;
    int ret;

    D(printf("Enter\n"));

	ret = SIS8300DRV_CALL("sis8300drv_open_device", sis8300drv_open_device(mSisDevice));
	if (ret) {
		return ret;
	}
	ret = SIS8300DRV_CALL("sis8300drv_get_serial", sis8300drv_get_serial(mSisDevice, &serialNumber));
	if (ret) {
		return ret;
	}
	ret = SIS8300DRV_CALL("sis8300drv_get_fw_version", sis8300drv_get_fw_version(mSisDevice, &firmwareVersion));
	if (ret) {
		return ret;
	}
	firmwareVersion &= 0x0000FFFF;
	ret = SIS8300DRV_CALL("sis8300drv_get_device_type", sis8300drv_get_device_type(mSisDevice, &deviceType));
	if (ret) {
		return ret;
	}
	ret = SIS8300DRV_CALL("sis8300drv_get_memory_size", sis8300drv_get_memory_size(mSisDevice, &memorySizeMb));
	if (ret) {
		return ret;
	}
	memorySizeMb /= (1024*1024);

	ret = SIS8300DRV_CALL("sis8300drv_init_adc", sis8300drv_init_adc(mSisDevice));
	if (ret) {
		return ret;
	}

	setIntegerParam(P_FirmwareVersion, firmwareVersion);
	setIntegerParam(P_SerialNumber, serialNumber);
	setIntegerParam(P_MemorySize, memorySizeMb);
	setIntegerParam(P_DeviceType, deviceType);
	callParamCallbacks(0);

	I(printf("Device is %X, serial no. %d, fw 0x%4X, mem size %d MB\n",
			deviceType,
			serialNumber,
			firmwareVersion,
			(unsigned int)memorySizeMb));

	return 0;
}

int ADSIS8300::destroyDevice()
{
	int ret;

	D(printf("Enter\n"));

	ret = SIS8300DRV_CALL("sis8300drv_close_device", sis8300drv_close_device(mSisDevice));
	return ret;
}

int ADSIS8300::enableChannel(unsigned int channel)
{
   	mChannelMask |= (1 << channel);
    D(printf("channel mask %X\n", mChannelMask));
	return 0;
}

int ADSIS8300::disableChannel(unsigned int channel)
{
   	mChannelMask &= ~(1 << channel);
    D(printf("channel mask %X\n", mChannelMask));
	return 0;
}

/** Configuration command, called directly or from iocsh */
extern "C" int SIS8300Config(const char *portName, const char *devicePath,
		int maxAddr, int numAiSamples, int dataType, int maxBuffers, int maxMemory,
		int priority, int stackSize)
{
    new ADSIS8300(portName, devicePath,
    		maxAddr,
			0,
    		numAiSamples,
			(NDDataType_t)dataType,
			(maxBuffers < 0) ? 0 : maxBuffers,
			(maxMemory < 0) ? 0 : maxMemory,
			priority, stackSize);
    return(asynSuccess);
}

/** Code for iocsh registration */
static const iocshArg SIS8300ConfigArg0 = {"Port name",     iocshArgString};
static const iocshArg SIS8300ConfigArg1 = {"Device path",   iocshArgString};
static const iocshArg SIS8300ConfigArg2 = {"# channels",    iocshArgInt};
static const iocshArg SIS8300ConfigArg3 = {"# samples",     iocshArgInt};
static const iocshArg SIS8300ConfigArg4 = {"Data type",     iocshArgInt};
static const iocshArg SIS8300ConfigArg5 = {"maxBuffers",    iocshArgInt};
static const iocshArg SIS8300ConfigArg6 = {"maxMemory",     iocshArgInt};
static const iocshArg SIS8300ConfigArg7 = {"priority",      iocshArgInt};
static const iocshArg SIS8300ConfigArg8 = {"stackSize",     iocshArgInt};
static const iocshArg * const SIS8300ConfigArgs[] = {&SIS8300ConfigArg0,
                                                     &SIS8300ConfigArg1,
													 &SIS8300ConfigArg2,
													 &SIS8300ConfigArg3,
													 &SIS8300ConfigArg4,
													 &SIS8300ConfigArg5,
													 &SIS8300ConfigArg6,
													 &SIS8300ConfigArg7,
													 &SIS8300ConfigArg8};
static const iocshFuncDef configSIS8300 = {"SIS8300Config", 9, SIS8300ConfigArgs};
static void configSIS8300CallFunc(const iocshArgBuf *args)
{
    SIS8300Config(args[0].sval, args[1].sval, args[2].ival, args[3].ival,
    		args[4].ival, args[5].ival, args[6].ival, args[7].ival, args[8].ival);
}


static void SIS8300Register(void)
{
    iocshRegister(&configSIS8300, configSIS8300CallFunc);
}

extern "C" {
epicsExportRegistrar(SIS8300Register);
}
