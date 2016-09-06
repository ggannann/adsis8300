TOP=../..
include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE

#======== DETECTOR LIBRARY ==============

# The following gets rid of the -fno-implicit-templates flag on vxWorks, 
# so we get automatic template instantiation.
# This is what we want for miscellaneous/asynPortDriver.cpp
ifeq (vxWorks,$(findstring vxWorks, $(T_A)))
CODE_CXXFLAGS=
endif

LIBRARY_IOC = SIS8300

# Vendor specific headers and sources
USR_INCLUDES += -I$(TOP)/vendor/struck/driver
USR_INCLUDES += -I$(TOP)/vendor/struck/lib/libSIS830x
SRC_DIRS += $(TOP)/vendor/struck/lib/libSIS830x
LIB_SRCS += ad9510.c internal.c main.c rtmI2C.c si5338aSynth.c spiFlash.c

# EPICS specfic headers and sources
INC += ADSIS8300.h
LIB_SRCS += ADSIS8300.cpp

DBD += SIS8300Support.dbd

include $(ADCORE)/ADApp/commonLibraryMakefile

#=============================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

