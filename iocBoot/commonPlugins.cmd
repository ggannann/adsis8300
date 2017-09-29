# Create a netCDF file saving plugin.
#NDFileNetCDFConfigure("FileNetCDF1", $(QSIZE), 0, "$(PORT)", 0)
#dbLoadRecords("NDFileNetCDF.template","P=$(PREFIX),R=netCDF1:,PORT=FileNetCDF1,ADDR=0,TIMEOUT=1,NDARRAY_PORT=$(PORT)")

# Create a NeXus file saving plugin
#NDFileNexusConfigure("FileNexus1", $(QSIZE), 0, "$(PORT)", 0)
#dbLoadRecords("NDFileNexus.template", "P=$(PREFIX),R=Nexus1:,PORT=FileNexus1,ADDR=0,TIMEOUT=1,NDARRAY_PORT=$(PORT)")

# Create an HDF5 file saving plugin
#NDFileHDF5Configure("FileHDF1", $(QSIZE), 0, "$(PORT)", 0)
#dbLoadRecords("NDFileHDF5.template",  "P=$(PREFIX),R=HDF1:,PORT=FileHDF1,ADDR=0,TIMEOUT=1,NDARRAY_PORT=$(PORT)")

# Create 4 ROI plugins
#NDROIConfigure("ROI1", $(QSIZE), 0, "$(PORT)", 0, 0, 0)
#dbLoadRecords("NDROI.template",       "P=$(PREFIX),R=ROI1:,  PORT=ROI1,ADDR=0,TIMEOUT=1,NDARRAY_PORT=$(PORT)")
#NDROIConfigure("ROI2", $(QSIZE), 0, "$(PORT)", 0, 0, 0)
#dbLoadRecords("NDROI.template",       "P=$(PREFIX),R=ROI2:,  PORT=ROI2,ADDR=0,TIMEOUT=1,NDARRAY_PORT=$(PORT)")
#NDROIConfigure("ROI3", $(QSIZE), 0, "$(PORT)", 0, 0, 0)
#dbLoadRecords("NDROI.template",       "P=$(PREFIX),R=ROI3:,  PORT=ROI3,ADDR=0,TIMEOUT=1,NDARRAY_PORT=$(PORT)")
#NDROIConfigure("ROI4", $(QSIZE), 0, "$(PORT)", 0, 0, 0)
#dbLoadRecords("NDROI.template",       "P=$(PREFIX),R=ROI4:,  PORT=ROI4,ADDR=0,TIMEOUT=1,NDARRAY_PORT=$(PORT)")

# Create 8 ROIStat plugins
NDROIStatConfigure("ROISTAT1", $(QSIZE), 0, "$(PORT)", 0, 8, 0, 0)
dbLoadRecords("NDROIStat.template",   "P=$(PREFIX),R=ROIStat1:  ,PORT=ROISTAT1,ADDR=0,TIMEOUT=1,NDARRAY_PORT=$(PORT),NCHANS=$(NCHANS)")
dbLoadRecords("NDROIStatN.template",  "P=$(PREFIX),R=ROIStat1:1:,PORT=ROISTAT1,ADDR=0,TIMEOUT=1,NCHANS=$(NCHANS)")
dbLoadRecords("NDROIStatN.template",  "P=$(PREFIX),R=ROIStat1:2:,PORT=ROISTAT1,ADDR=1,TIMEOUT=1,NCHANS=$(NCHANS)")
dbLoadRecords("NDROIStatN.template",  "P=$(PREFIX),R=ROIStat1:3:,PORT=ROISTAT1,ADDR=2,TIMEOUT=1,NCHANS=$(NCHANS)")
dbLoadRecords("NDROIStatN.template",  "P=$(PREFIX),R=ROIStat1:4:,PORT=ROISTAT1,ADDR=3,TIMEOUT=1,NCHANS=$(NCHANS)")
dbLoadRecords("NDROIStatN.template",  "P=$(PREFIX),R=ROIStat1:5:,PORT=ROISTAT1,ADDR=4,TIMEOUT=1,NCHANS=$(NCHANS)")
dbLoadRecords("NDROIStatN.template",  "P=$(PREFIX),R=ROIStat1:6:,PORT=ROISTAT1,ADDR=5,TIMEOUT=1,NCHANS=$(NCHANS)")
dbLoadRecords("NDROIStatN.template",  "P=$(PREFIX),R=ROIStat1:7:,PORT=ROISTAT1,ADDR=6,TIMEOUT=1,NCHANS=$(NCHANS)")
dbLoadRecords("NDROIStatN.template",  "P=$(PREFIX),R=ROIStat1:8:,PORT=ROISTAT1,ADDR=7,TIMEOUT=1,NCHANS=$(NCHANS)")

# Create a processing plugin
NDProcessConfigure("PROC1", $(QSIZE), 0, "$(PORT)", 0, 0, 0)
dbLoadRecords("NDProcess.template",   "P=$(PREFIX),R=Proc1:,  PORT=PROC1,ADDR=0,TIMEOUT=1,NDARRAY_PORT=$(PORT)")

# Create 5 statistics plugins
#NDStatsConfigure("STATS1", $(QSIZE), 0, "$(PORT)", 0, 0, 0)
#dbLoadRecords("NDStats.template",     "P=$(PREFIX),R=Stats1:,  PORT=STATS1,ADDR=0,TIMEOUT=1,HIST_SIZE=256,XSIZE=$(XSIZE),YSIZE=$(YSIZE),NCHANS=$(NCHANS),NDARRAY_PORT=$(PORT)")
#NDStatsConfigure("STATS2", $(QSIZE), 0, "ROI1", 0, 0, 0)
#dbLoadRecords("NDStats.template",     "P=$(PREFIX),R=Stats2:,  PORT=STATS2,ADDR=0,TIMEOUT=1,HIST_SIZE=256,XSIZE=$(XSIZE),YSIZE=$(YSIZE),NCHANS=$(NCHANS),NDARRAY_PORT=$(PORT)")
#NDStatsConfigure("STATS3", $(QSIZE), 0, "ROI2", 0, 0, 0)
#dbLoadRecords("NDStats.template",     "P=$(PREFIX),R=Stats3:,  PORT=STATS3,ADDR=0,TIMEOUT=1,HIST_SIZE=256,XSIZE=$(XSIZE),YSIZE=$(YSIZE),NCHANS=$(NCHANS),NDARRAY_PORT=$(PORT)")
#NDStatsConfigure("STATS4", $(QSIZE), 0, "ROI3", 0, 0, 0)
#dbLoadRecords("NDStats.template",     "P=$(PREFIX),R=Stats4:,  PORT=STATS4,ADDR=0,TIMEOUT=1,HIST_SIZE=256,XSIZE=$(XSIZE),YSIZE=$(YSIZE),NCHANS=$(NCHANS),NDARRAY_PORT=$(PORT)")
#NDStatsConfigure("STATS5", $(QSIZE), 0, "ROI4", 0, 0, 0)
#dbLoadRecords("NDStats.template",     "P=$(PREFIX),R=Stats5:,  PORT=STATS5,ADDR=0,TIMEOUT=1,HIST_SIZE=256,XSIZE=$(XSIZE),YSIZE=$(YSIZE),NCHANS=$(NCHANS),NDARRAY_PORT=$(PORT)")

# Create a circular buffer plugin
#NDCircularBuffConfigure("CB1", $(QSIZE), 0, "$(PORT)", 0, $(CBUFFS), 0)
#dbLoadRecords("NDCircularBuff.template", "P=$(PREFIX),R=CB1:,  PORT=CB1,ADDR=0,TIMEOUT=1,NDARRAY_PORT=$(PORT)")

set_requestfile_path("./")
set_requestfile_path("$(ADCORE)/ADApp/Db")
set_requestfile_path("$(ADCORE)/iocBoot")
set_savefile_path("./autosave")
set_pass0_restoreFile("auto_settings.sav")
set_pass1_restoreFile("auto_settings.sav")
save_restoreSet_status_prefix("$(PREFIX)")
dbLoadRecords("$(AUTOSAVE)/asApp/Db/save_restoreStatus.db", "P=$(PREFIX)")
