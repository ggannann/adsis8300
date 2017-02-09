#Makefile at top of application tree
TOP = .
include $(TOP)/configure/CONFIG
DIRS += configure
DIRS += SIS8300App
DIRS += vendor
SIS8300App_DEPEND_DIRS += vendor

ifeq ($(BUILD_IOCS), YES)
DIRS += SIS8300DemoApp
SIS8300DemoApp_DEPEND_DIRS += SIS8300App
iocBoot_DEPEND_DIRS += SIS8300DemoApp
DIRS += iocBoot
endif

include $(TOP)/configure/RULES_TOP
