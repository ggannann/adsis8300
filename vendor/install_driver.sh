#!/bin/bash

#*************************************************************************#
#                                                                         #
#  Filename: install_driver.sh                                            #
#                                                                         #
#  Function:         Reinstall the driver after a kernel update.          #
#                                                                         #
#                    Please run this script with superuser rights.        #
#                                                                         #
#  Autor:                CT                                               #
#  date:                 23.08.2013                                       #
#  last modification:    16.02.2016                                       #
#                                                                         #
# ----------------------------------------------------------------------- #
#                                                                         #
#  SIS  Struck Innovative Systeme GmbH                                    #
#                                                                         #
#  Harksheider Str. 102A                                                  #
#  22399 Hamburg                                                          #
#                                                                         #
#  Tel. +49 (0)40 60 87 305 0                                             #
#  Fax  +49 (0)40 60 87 305 20                                            #
#                                                                         #
#  http://www.struck.de                                                   #
#                                                                         #
#  © 2016                                                                 #
#                                                                         #
#*************************************************************************#

SIS=~/SIS/Testroom
ldriver=/lib/modules/`uname -r`/sis8300drv.ko
lrules=/etc/udev/rules.d/92-struck.rules

if [ ! -f $ldriver ]; then
 make -C $SIS/driver/
 sudo cp $SIS/driver/sis8300drv.ko /lib/modules/`uname -r`
 
 if [ ! -f $lrules ]; then
  sudo make -C $SIS/driver/ install
 else
  sudo depmod -A
 fi
 
 make -C $SIS/driver/ clean
fi

