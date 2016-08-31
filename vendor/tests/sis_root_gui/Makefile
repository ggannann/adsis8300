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
LDFLAGS  +=  -lfftw3


SIS8300_LIBRARYO    = sis8300_library.$(ObjSuf)
SIS8300_LIBRARYS    = sis8300_library.$(SrcSuf)

SIS8300_ROOT_GUIO      = sis8300_root_gui.$(ObjSuf)
SIS8300_ROOT_GUIS      = sis8300_root_gui.$(SrcSuf)
SIS8300_ROOT_GUI       = sis8300_root_gui$(ExeSuf)




OBJS         += $(GUITESTO)  $(SIS8300_ROOT_GUIO)  $(SIS8300_LIBRARYO)  
PROGRAMS     += $(GUITEST)  $(SIS8300_ROOT_GUI)  


#------------------------------------------------------------------------------

.SUFFIXES: .$(SrcSuf) .$(ObjSuf) .$(DllSuf)
.PHONY:    Aclock Hello Tetris

all:            $(PROGRAMS)
#ifdef th_raus

gnuplot_i.o:	gnuplot_i.cpp gnuplot_i.hpp



$(SIS8300_ROOT_GUI):     $(SIS8300_ROOT_GUIO) $(SIS8300_LIBRARYO)
		$(LD)  $^ $(GLIBS) $(LDFLAGS) $(OutPutOpt)$@
		$(MT_EXE)
		@echo "$@ done"






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


.SUFFIXES: .$(SrcSuf)

###
 
.$(SrcSuf).$(ObjSuf):
	$(CXX)  $(CXXFLAGS)  -c $<

ifeq ($(GCC_MAJOR),4)
ifeq ($(GCC_MINOR),1)
TBenchDict.o: CXXFLAGS += -Wno-strict-aliasing
endif
endif
#endif

