< envPaths
errlogInit(20000)

dbLoadDatabase("$(TOP)/dbd/SIS8300DemoApp.dbd")
SIS8300DemoApp_registerRecordDeviceDriver(pdbbase) 

# Prefix for all records
epicsEnvSet("PREFIX", "SIS8300:")
# The port name for the detector
epicsEnvSet("PORT",   "SIS8300")
# The queue size for all plugins
epicsEnvSet("QSIZE",  "20")
# The maximum number of time series points in the NDPluginStats plugin
epicsEnvSet("NCHANS", "64")
# The maximum number of frames buffered in the NDPluginCircularBuff plugin
epicsEnvSet("CBUFFS", "500")
# The search path for database files
epicsEnvSet("EPICS_DB_INCLUDE_PATH", "$(ADCORE)/db")

epicsEnvSet("AICH0",      "AI0")
epicsEnvSet("AICH1",      "AI1")
epicsEnvSet("AICH2",      "AI2")
epicsEnvSet("AICH3",      "AI3")
epicsEnvSet("AICH4",      "AI4")
epicsEnvSet("AICH5",      "AI5")
epicsEnvSet("AICH6",      "AI6")
epicsEnvSet("AICH7",      "AI7")
epicsEnvSet("AICH8",      "AI8")
epicsEnvSet("AICH9",      "AI9")

# This is sum of AI and BPM asyn addresses
# ADDR 0 .. 9 are for AI
epicsEnvSet("NUM_CH",        "10")
# Number of samples to acquire
epicsEnvSet("NUM_SAMPLES",   "1024")
# The maximum number of time series points in the NDPluginTimeSeries plugin
epicsEnvSet("TSPOINTS",      "600000")

asynSetMinTimerPeriod(0.001)

# Uncomment the following line to set it in the IOC.
epicsEnvSet("EPICS_CA_MAX_ARRAY_BYTES", "10000000")

# Create an SIS8300 driver
# SIS8300Config(const char *portName, const char *devicePath,
#            int maxAddr, int numParams, int numTimePoints, NDDataType_t dataType,
#            int maxBuffers, size_t maxMemory, int priority, int stackSize)
SIS8300Config("$(PORT)", "/dev/sis8300-12", $(NUM_CH), $(NUM_SAMPLES), 7, 0, 0)
dbLoadRecords("$(ADSIS8300)/db/SIS8300.template",  "P=$(PREFIX),R=,          PORT=$(PORT),ADDR=0,TIMEOUT=1")
dbLoadRecords("$(ADSIS8300)/db/SIS8300N.template", "P=$(PREFIX),R=$(AICH0):, PORT=$(PORT),ADDR=0,TIMEOUT=1,NAME=$(AICH0)")
dbLoadRecords("$(ADSIS8300)/db/SIS8300N.template", "P=$(PREFIX),R=$(AICH1):, PORT=$(PORT),ADDR=1,TIMEOUT=1,NAME=$(AICH1)")
dbLoadRecords("$(ADSIS8300)/db/SIS8300N.template", "P=$(PREFIX),R=$(AICH2):, PORT=$(PORT),ADDR=2,TIMEOUT=1,NAME=$(AICH2)")
dbLoadRecords("$(ADSIS8300)/db/SIS8300N.template", "P=$(PREFIX),R=$(AICH3):, PORT=$(PORT),ADDR=3,TIMEOUT=1,NAME=$(AICH3)")
dbLoadRecords("$(ADSIS8300)/db/SIS8300N.template", "P=$(PREFIX),R=$(AICH4):, PORT=$(PORT),ADDR=4,TIMEOUT=1,NAME=$(AICH4)")
dbLoadRecords("$(ADSIS8300)/db/SIS8300N.template", "P=$(PREFIX),R=$(AICH5):, PORT=$(PORT),ADDR=5,TIMEOUT=1,NAME=$(AICH5)")
dbLoadRecords("$(ADSIS8300)/db/SIS8300N.template", "P=$(PREFIX),R=$(AICH6):, PORT=$(PORT),ADDR=6,TIMEOUT=1,NAME=$(AICH6)")
dbLoadRecords("$(ADSIS8300)/db/SIS8300N.template", "P=$(PREFIX),R=$(AICH7):, PORT=$(PORT),ADDR=7,TIMEOUT=1,NAME=$(AICH7)")
dbLoadRecords("$(ADSIS8300)/db/SIS8300N.template", "P=$(PREFIX),R=$(AICH8):, PORT=$(PORT),ADDR=8,TIMEOUT=1,NAME=$(AICH8)")
dbLoadRecords("$(ADSIS8300)/db/SIS8300N.template", "P=$(PREFIX),R=$(AICH9):, PORT=$(PORT),ADDR=9,TIMEOUT=1,NAME=$(AICH9)")

# Create a standard arrays plugin, set it to get data from ADSIS8300 driver.
NDStdArraysConfigure("Image1", 3, 0, "$(PORT)", 0)
# This creates a waveform large enough for 100x10 arrays.
dbLoadRecords("$(ADCORE)/db/NDStdArrays.template", "P=$(PREFIX),R=image1:,PORT=Image1,ADDR=0,TIMEOUT=1,NDARRAY_PORT=$(PORT),TYPE=Float64,FTVL=DOUBLE,NELEMENTS=1000")

# Time series plugin for converted AI data
NDTimeSeriesConfigure("TS0", $(QSIZE), 0, "$(PORT)", 0, 10)
dbLoadRecords("$(ADCORE)/db/NDTimeSeries.template",  "P=$(PREFIX),R=TS0:,   PORT=TS0,ADDR=0,TIMEOUT=1,NDARRAY_PORT=$(PORT),NDARRAY_ADDR=0,NCHANS=$(TSPOINTS),TIME_LINK=$(PREFIX)TimeStep CP MS,ENABLED=1")
dbLoadRecords("$(ADCORE)/db/NDTimeSeriesN.template", "P=$(PREFIX),R=TS0:0:, PORT=TS0,ADDR=0,TIMEOUT=1,NCHANS=$(TSPOINTS),NAME=$(AICH0)")
dbLoadRecords("$(ADCORE)/db/NDTimeSeriesN.template", "P=$(PREFIX),R=TS0:1:, PORT=TS0,ADDR=1,TIMEOUT=1,NCHANS=$(TSPOINTS),NAME=$(AICH1)")
dbLoadRecords("$(ADCORE)/db/NDTimeSeriesN.template", "P=$(PREFIX),R=TS0:2:, PORT=TS0,ADDR=2,TIMEOUT=1,NCHANS=$(TSPOINTS),NAME=$(AICH2)")
dbLoadRecords("$(ADCORE)/db/NDTimeSeriesN.template", "P=$(PREFIX),R=TS0:3:, PORT=TS0,ADDR=3,TIMEOUT=1,NCHANS=$(TSPOINTS),NAME=$(AICH3)")
dbLoadRecords("$(ADCORE)/db/NDTimeSeriesN.template", "P=$(PREFIX),R=TS0:4:, PORT=TS0,ADDR=4,TIMEOUT=1,NCHANS=$(TSPOINTS),NAME=$(AICH4)")
dbLoadRecords("$(ADCORE)/db/NDTimeSeriesN.template", "P=$(PREFIX),R=TS0:5:, PORT=TS0,ADDR=5,TIMEOUT=1,NCHANS=$(TSPOINTS),NAME=$(AICH5)")
dbLoadRecords("$(ADCORE)/db/NDTimeSeriesN.template", "P=$(PREFIX),R=TS0:6:, PORT=TS0,ADDR=6,TIMEOUT=1,NCHANS=$(TSPOINTS),NAME=$(AICH6)")
dbLoadRecords("$(ADCORE)/db/NDTimeSeriesN.template", "P=$(PREFIX),R=TS0:7:, PORT=TS0,ADDR=7,TIMEOUT=1,NCHANS=$(TSPOINTS),NAME=$(AICH7)")
dbLoadRecords("$(ADCORE)/db/NDTimeSeriesN.template", "P=$(PREFIX),R=TS0:8:, PORT=TS0,ADDR=8,TIMEOUT=1,NCHANS=$(TSPOINTS),NAME=$(AICH8)")
dbLoadRecords("$(ADCORE)/db/NDTimeSeriesN.template", "P=$(PREFIX),R=TS0:9:, PORT=TS0,ADDR=9,TIMEOUT=1,NCHANS=$(TSPOINTS),NAME=$(AICH9)")

# FFT plugins
NDFFTConfigure("FFT0", $(QSIZE), 0, "TS0", 0)
dbLoadRecords("$(ADCORE)/db/NDFFT.template","P=$(PREFIX),R=FFT0:,PORT=FFT0,ADDR=0,TIMEOUT=1,NDARRAY_PORT=TS0,NDARRAY_ADDR=0,NCHANS=$(TSPOINTS),TIME_LINK=$(PREFIX)TS:TSAveragingTime_RBV CP MS,ENABLED=1,NAME=$(AICH0)")
NDFFTConfigure("FFT1", $(QSIZE), 0, "TS0", 1)
dbLoadRecords("$(ADCORE)/db/NDFFT.template","P=$(PREFIX),R=FFT1:,PORT=FFT1,ADDR=0,TIMEOUT=1,NDARRAY_PORT=TS0,NDARRAY_ADDR=1,NCHANS=$(TSPOINTS),TIME_LINK=$(PREFIX)TS:TSAveragingTime_RBV CP MS,ENABLED=1,NAME=$(AICH1)")
NDFFTConfigure("FFT2", $(QSIZE), 0, "TS0", 2
dbLoadRecords("$(ADCORE)/db/NDFFT.template","P=$(PREFIX),R=FFT2:,PORT=FFT2,ADDR=0,TIMEOUT=1,NDARRAY_PORT=TS0,NDARRAY_ADDR=2,NCHANS=$(TSPOINTS),TIME_LINK=$(PREFIX)TS:TSAveragingTime_RBV CP MS,ENABLED=1,NAME=$(AICH2)")
NDFFTConfigure("FFT3", $(QSIZE), 0, "TS0", 3)
dbLoadRecords("$(ADCORE)/db/NDFFT.template","P=$(PREFIX),R=FFT3:,PORT=FFT3,ADDR=0,TIMEOUT=1,NDARRAY_PORT=TS0,NDARRAY_ADDR=3,NCHANS=$(TSPOINTS),TIME_LINK=$(PREFIX)TS:TSAveragingTime_RBV CP MS,ENABLED=1,NAME=$(AICH3)")
NDFFTConfigure("FFT4", $(QSIZE), 0, "TS0", 4)
dbLoadRecords("$(ADCORE)/db/NDFFT.template","P=$(PREFIX),R=FFT4:,PORT=FFT4,ADDR=0,TIMEOUT=1,NDARRAY_PORT=TS0,NDARRAY_ADDR=4,NCHANS=$(TSPOINTS),TIME_LINK=$(PREFIX)TS:TSAveragingTime_RBV CP MS,ENABLED=1,NAME=$(AICH4)")
NDFFTConfigure("FFT5", $(QSIZE), 0, "TS0", 5)
dbLoadRecords("$(ADCORE)/db/NDFFT.template","P=$(PREFIX),R=FFT5:,PORT=FFT5,ADDR=0,TIMEOUT=1,NDARRAY_PORT=TS0,NDARRAY_ADDR=5,NCHANS=$(TSPOINTS),TIME_LINK=$(PREFIX)TS:TSAveragingTime_RBV CP MS,ENABLED=1,NAME=$(AICH5)")
NDFFTConfigure("FFT6", $(QSIZE), 0, "TS0", 6)
dbLoadRecords("$(ADCORE)/db/NDFFT.template","P=$(PREFIX),R=FFT6:,PORT=FFT6,ADDR=0,TIMEOUT=1,NDARRAY_PORT=TS0,NDARRAY_ADDR=6,NCHANS=$(TSPOINTS),TIME_LINK=$(PREFIX)TS:TSAveragingTime_RBV CP MS,ENABLED=1,NAME=$(AICH6)")
NDFFTConfigure("FFT7", $(QSIZE), 0, "TS0", 7)
dbLoadRecords("$(ADCORE)/db/NDFFT.template","P=$(PREFIX),R=FFT7:,PORT=FFT7,ADDR=0,TIMEOUT=1,NDARRAY_PORT=TS0,NDARRAY_ADDR=7,NCHANS=$(TSPOINTS),TIME_LINK=$(PREFIX)TS:TSAveragingTime_RBV CP MS,ENABLED=1,NAME=$(AICH7)")
NDFFTConfigure("FFT8", $(QSIZE), 0, "TS0", 8)
dbLoadRecords("$(ADCORE)/db/NDFFT.template","P=$(PREFIX),R=FFT8:,PORT=FFT8,ADDR=0,TIMEOUT=1,NDARRAY_PORT=TS0,NDARRAY_ADDR=8,NCHANS=$(TSPOINTS),TIME_LINK=$(PREFIX)TS:TSAveragingTime_RBV CP MS,ENABLED=1,NAME=$(AICH8)")
NDFFTConfigure("FFT9", $(QSIZE), 0, "TS0", 9)
dbLoadRecords("$(ADCORE)/db/NDFFT.template","P=$(PREFIX),R=FFT9:,PORT=FFT9,ADDR=0,TIMEOUT=1,NDARRAY_PORT=TS0,NDARRAY_ADDR=9,NCHANS=$(TSPOINTS),TIME_LINK=$(PREFIX)TS:TSAveragingTime_RBV CP MS,ENABLED=1,NAME=$(AICH9)")


# Timing MTCA EVR 300
# As per EVR MTCA 300 engineering manual ch 5.3.5
epicsEnvSet("SYS"               "EVR")
epicsEnvSet("DEVICE"            "MTCA")
epicsEnvSet("EVR_PCIDOMAIN"     "0x0")
epicsEnvSet("EVR_PCIBUS"        "0x6")
epicsEnvSet("EVR_PCIDEVICE"     "0x0")
epicsEnvSet("EVR_PCIFUNCTION"   "0x0")

mrmEvrSetupPCI($(DEVICE), $(EVR_PCIDOMAIN), $(EVR_PCIBUS), $(EVR_PCIDEVICE), $(EVR_PCIFUNCTION))
dbLoadRecords("$(MRFIOC2)/db/evr-mtca-300.db", "DEVICE=$(DEVICE), SYS=$(SYS), Link-Clk-SP=88.0525")

dbLoadRecords("$(MRFIOC2)/db/evr-softEvent.template", "DEVICE=$(DEVICE), SYS=$(SYS), EVT=14, CODE=14")
dbLoadRecords("$(MRFIOC2)/db/evr-pulserMap.template", "DEVICE=$(DEVICE), SYS=$(SYS), PID=0, F=Trig, ID=0, EVT=14")

## Load all other plugins using commonPlugins.cmd
< $(ADCORE)/iocBoot/commonPlugins.cmd

set_requestfile_path("$(ADSIS8300)/SIS8300App/Db")

#asynSetTraceIOMask("$(PORT)",0,2)
#asynSetTraceMask("$(PORT)",0,255)

iocInit()



# Disable Rear Universal Output 33
dbpf $(SYS)-$(DEVICE):RearUniv33-Ena-SP "Disabled"
# Map Rear Universal Output 33 to pulser o
dbpf $(SYS)-$(DEVICE):RearUniv33-Src-SP 0
# Map pulser 0 to event 14
dbpf $(SYS)-$(DEVICE):Pul0-Evt-Trig0-SP 14
# Set pulser 0 width to 10 ms
dbpf $(SYS)-$(DEVICE):Pul0-Width-SP 10000
# Watch closely to the graph in the CSS OPI screen when enabling Rear Universal Output 33, since
# in the next event 14 received the SIS8300 will start the data acquisition
dbpf $(SYS)-$(DEVICE):RearUniv33-Ena-SP "Enabled"
