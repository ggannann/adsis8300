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
# The maximim image width; used for row profiles in the NDPluginStats plugin
epicsEnvSet("XSIZE",  "10")
# The maximim image height; used for column profiles in the NDPluginStats plugin
epicsEnvSet("YSIZE",  "32")
# The maximum number of time series points in the NDPluginStats plugin
epicsEnvSet("NCHANS", "64")
# The maximum number of time series points in the NDPluginTimeSeries plugin
epicsEnvSet("TSPOINTS", "64")
# The maximum number of frames buffered in the NDPluginCircularBuff plugin
epicsEnvSet("CBUFFS", "500")
# The search path for database files
epicsEnvSet("EPICS_DB_INCLUDE_PATH", "$(ADCORE)/db")

epicsEnvSet("AI1",  "AI1")
epicsEnvSet("AI2",  "AI2")
epicsEnvSet("AI3",  "AI3")
epicsEnvSet("AI4",  "AI4")
epicsEnvSet("AI5",  "AI5")
epicsEnvSet("AI6",  "AI6")
epicsEnvSet("AI7",  "AI7")
epicsEnvSet("AI8",  "AI8")
epicsEnvSet("AI9",  "AI9")
epicsEnvSet("AI10", "AI10")

asynSetMinTimerPeriod(0.001)

# The EPICS environment variable EPICS_CA_MAX_ARRAY_BYTES needs to be set to a value at least as large
# as the largest image that the standard arrays plugin will send.
# That vlaue is $(XSIZE) * $(YSIZE) * sizeof(FTVL data type) for the FTVL used when loading the NDStdArrays.template file.
# The variable can be set in the environment before running the IOC or it can be set here.
# It is often convenient to set it in the environment outside the IOC to the largest array any client 
# or server will need.  For example 10000000 (ten million) bytes may be enough.
# If it is set here then remember to also set it outside the IOC for any CA clients that need to access the waveform record.  
# Do not set EPICS_CA_MAX_ARRAY_BYTES to a value much larger than that required, because EPICS Channel Access actually
# allocates arrays of this size every time it needs a buffer larger than 16K.
# Uncomment the following line to set it in the IOC.
epicsEnvSet("EPICS_CA_MAX_ARRAY_BYTES", "10000000")

# Create an SIS8300 driver
# SIS8300Config(const char *portName, const char *devicePath,
#            int maxAddr, int numParams, int numTimePoints, NDDataType_t dataType,
#            int maxBuffers, size_t maxMemory, int priority, int stackSize)
SIS8300Config("$(PORT)", "/dev/sis8300-12", $(XSIZE), $(YSIZE), 7, 0, 0)
dbLoadRecords("$(ADSIS8300)/db/SIS8300.template",  "P=$(PREFIX),R=,        PORT=$(PORT),ADDR=0,TIMEOUT=1")
dbLoadRecords("$(ADSIS8300)/db/SIS8300N.template", "P=$(PREFIX),R=$(AI1):, PORT=$(PORT),ADDR=0,TIMEOUT=1,NAME=$(AI1)")
dbLoadRecords("$(ADSIS8300)/db/SIS8300N.template", "P=$(PREFIX),R=$(AI2):, PORT=$(PORT),ADDR=1,TIMEOUT=1,NAME=$(AI2)")
dbLoadRecords("$(ADSIS8300)/db/SIS8300N.template", "P=$(PREFIX),R=$(AI3):, PORT=$(PORT),ADDR=2,TIMEOUT=1,NAME=$(AI3)")
dbLoadRecords("$(ADSIS8300)/db/SIS8300N.template", "P=$(PREFIX),R=$(AI4):, PORT=$(PORT),ADDR=3,TIMEOUT=1,NAME=$(AI4)")
dbLoadRecords("$(ADSIS8300)/db/SIS8300N.template", "P=$(PREFIX),R=$(AI5):, PORT=$(PORT),ADDR=4,TIMEOUT=1,NAME=$(AI5)")
dbLoadRecords("$(ADSIS8300)/db/SIS8300N.template", "P=$(PREFIX),R=$(AI6):, PORT=$(PORT),ADDR=5,TIMEOUT=1,NAME=$(AI6)")
dbLoadRecords("$(ADSIS8300)/db/SIS8300N.template", "P=$(PREFIX),R=$(AI7):, PORT=$(PORT),ADDR=6,TIMEOUT=1,NAME=$(AI7)")
dbLoadRecords("$(ADSIS8300)/db/SIS8300N.template", "P=$(PREFIX),R=$(AI8):, PORT=$(PORT),ADDR=7,TIMEOUT=1,NAME=$(AI8)")
dbLoadRecords("$(ADSIS8300)/db/SIS8300N.template", "P=$(PREFIX),R=$(AI9):, PORT=$(PORT),ADDR=8,TIMEOUT=1,NAME=$(AI9)")
dbLoadRecords("$(ADSIS8300)/db/SIS8300N.template", "P=$(PREFIX),R=$(AI10):,PORT=$(PORT),ADDR=9,TIMEOUT=1,NAME=$(AI10)")


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
