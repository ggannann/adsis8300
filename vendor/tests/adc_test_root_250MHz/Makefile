# Makefile for the ROOT test programs.
# This Makefile shows nicely how to compile and link applications
# using the ROOT libraries on all supported platforms.
#
# Copyright (c) 2000 Rene Brun and Fons Rademakers
#
# Author: Fons Rademakers, 29/2/2000

include Makefile.arch
-include ../MyConfig.mk

#------------------------------------------------------------------------------


#CXXFLAGS += -I/usr/opt/sisusb/include
#LDFLAGS  += -L/usr/opt/sisusb/lib -ldl -lstdc++ -lsis3150 -Wl,"-rpath=/usr/opt/sisusb/lib"

HSIMPLEO      = hsimple.$(ObjSuf)
HSIMPLES      = hsimple.$(SrcSuf)
HSIMPLE       = hsimple$(ExeSuf)



SIS8300_TESTO      = sis8300-test.$(ObjSuf) gnuplot_i.o
SIS8300_TESTS      = sis8300-test.$(SrcSuf)
SIS8300_TEST       = sis8300-test$(ExeSuf)

SIS3302_WRAP_STOP_PRETRIG_REDUCEDO      = sis3302-wrap-stop-pretrig_reduced_addr_window.$(ObjSuf) gnuplot_i.o
SIS3302_WRAP_STOP_PRETRIG_REDUCEDS      = sis3302-wrap-stop-pretrig_reduced_addr_window.$(SrcSuf)
SIS3302_WRAP_STOP_PRETRIG_REDUCED       = sis3302-wrap-stop-pretrig_reduced_addr_window$(ExeSuf)




OBJS          = $(HSIMPLEO) $(SIS8300_TESTO) $(SIS3302_WRAP_STOP_PRETRIG_REDUCEDO)

PROGRAMS      = $(HSIMPLE)  $(SIS8300_TEST)  $(SIS3302_WRAP_STOP_PRETRIG_REDUCED) 

#------------------------------------------------------------------------------

.SUFFIXES: .$(SrcSuf) .$(ObjSuf) .$(DllSuf)
.PHONY:    Aclock Hello Tetris

all:            $(PROGRAMS)
#ifdef th_raus

gnuplot_i.o:	gnuplot_i.cpp gnuplot_i.hpp


$(HSIMPLE):     $(HSIMPLEO)
		$(LD) $(LDFLAGS) $^ $(LIBS) $(OutPutOpt)$@
		$(MT_EXE)
		@echo "$@ done"


$(SIS8300_TEST):     $(SIS8300_TESTO) 
		$(LD) $(LDFLAGS) $^ $(LIBS) $(OutPutOpt)$@
		$(MT_EXE)
		@echo "$@ done1"

$(SIS3302_WRAP_STOP_PRETRIG_REDUCED):     $(SIS3302_WRAP_STOP_PRETRIG_REDUCEDO) 
		$(LD) $(LDFLAGS) $^ $(LIBS) $(OutPutOpt)$@
		$(MT_EXE)
		@echo "$@ done1"









#ifdef th_raus

#endif

clean:
		@rm -f $(OBJS) $(TRACKMATHSRC) core

distclean:      clean
		@rm -f $(PROGRAMS) $(EVENTSO) $(EVENTLIB) *Dict.* *.def *.exp \
		   *.root *.ps *.so *.lib *.dll *.d *.log .def so_locations \
		   files/*
		@rm -rf cxx_repository
		-@cd RootShower && $(MAKE) distclean
		-@cd rhtml && $(MAKE) distclean
		-@cd RootIDE && $(MAKE) distclean

#-I/usr/opt/sisusb/include

#sis3302-wrap-stop-pretrig.$(ObjSuf): Hello.h

.SUFFIXES: .$(SrcSuf)

#sis3302-wrap-stop-pretrig.$(ObjSuf): -I/usr/opt/sisusb/include


#ifdef raus
###
stressIterators.$(ObjSuf): stressIterators.h
 
Event.$(ObjSuf): Event.h
MainEvent.$(ObjSuf): Event.h

EventDict.$(SrcSuf): Event.h EventLinkDef.h
	@echo "Generating dictionary $@..."
	$(ROOTCINT) -f $@ -c $^

Hello.$(ObjSuf): Hello.h
HelloDict.$(SrcSuf): Hello.h
	@echo "Generating dictionary $@..."
	$(ROOTCINT) -f $@ -c $^

Aclock.$(ObjSuf): Aclock.h
AclockDict.$(SrcSuf): Aclock.h
	@echo "Generating dictionary $@..."
	$(ROOTCINT) -f $@ -c $^

Tetris.$(ObjSuf): Tetris.h
TetrisDict.$(SrcSuf): Tetris.h
	@echo "Generating dictionary $@..."
	$(ROOTCINT) -f $@ -c $^

TBench.$(ObjSuf): TBench.h
TBenchDict.$(SrcSuf): TBench.h benchLinkDef.h
	@echo "Generating dictionary $@..."
	$(ROOTCINT) -f $@ -c $^

guiviewer.$(ObjSuf): guiviewer.h
guiviewerDict.$(SrcSuf): guiviewer.h guiviewerLinkDef.h
	@echo "Generating dictionary $@..."
	$(ROOTCINT) -f $@ -c $^

stressMathCore.$(ObjSuf): 	TrackMathCore.h
TrackMathCoreDict.$(SrcSuf): 	TrackMathCore.h TrackMathCoreLinkDef.h
	@echo "Generating dictionary $@ using rootcint ..."
	$(ROOTCINT) -f $@ -c $^	

TrackMathCoreRflx.$(SrcSuf): 	TrackMathCore.h TrackMathCoreRflx.xml
			@echo "Generating dictionary $@ using gccxml ..."
			genreflex TrackMathCore.h --selection_file=TrackMathCoreRflx.xml -o TrackMathCoreRflx.cxx  -I$(ROOTSYS)/include

.$(SrcSuf).$(ObjSuf):
	$(CXX)  $(CXXFLAGS)  -c $<

ifeq ($(GCC_MAJOR),4)
ifeq ($(GCC_MINOR),1)
TBenchDict.o: CXXFLAGS += -Wno-strict-aliasing
endif
endif
#endif

