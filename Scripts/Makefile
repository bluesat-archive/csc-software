# BLUESAT Critical Systems Computer Builder Script
# Copyright BLUESAT Project 2009
# Version 1.0 
# Author: Colin Tan
#
# This script will build the various components of the OS and perform linking.
# An image file will than be created from the component compiles. 
# Burning can also be done via this file
#

#----------
# Includes
#----------
include ../Burner/lpcburn/Makefile


#------------- 
# Directories
#-------------
APP_SOURCE_DIR		=../Applications
BURNER_DIR			=../Burner/lpcburn
BUILDS_DIR			=../Builds
CORE_SOURCE_DIR		=../Core
DRIVER_SOURCE_DIR	=../Drivers
IMAGE_DIR			=../Dist
INCLUDES_DIR		=../Includes
SCRIPTS_DIR			=../Scripts
UNIT_TESTS_DIR		=../Unit_Tests


#---------------------
# Compiler / Settings
#---------------------
CC			=arm-elf-gcc
OBJCOPY		=arm-elf-objcopy
WARNINGS=-Wall -Wextra -Wshadow -Wpointer-arith -Wbad-function-cast -Wcast-align -Wsign-compare \
-Waggregate-return -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wunused
CRT0		=$(CORE_SOURCE_DIR)/boot.s
CFLAGS		=$(WARNINGS) -D GCC_ARM7 -I. -I $(INCLUDES_DIR) -mcpu=arm7tdmi -fomit-frame-pointer \
-fno-strict-aliasing -Os -D RUN_FROM_ROM -T $(CORE_SOURCE_DIR)/lpc2109-rom.ld
#TODO: use -nostdlib but have to implement some stdlib functions
LINKER_FLAGS=-Xlinker -o$(IMAGE_DIR)/csc.elf -Xlinker -M -Xlinker -Map=$(IMAGE_DIR)/csc.map


#--------------
# Source files
#--------------
CORE_SRC = \
$(CORE_SOURCE_DIR)/tasks.c \
$(CORE_SOURCE_DIR)/queue.c \
$(CORE_SOURCE_DIR)/list.c \
$(CORE_SOURCE_DIR)/heap_2.c \
$(CORE_SOURCE_DIR)/port.c \
$(CORE_SOURCE_DIR)/portISR.c \
$(CORE_SOURCE_DIR)/main.c

APP_SRC = \
$(APP_SOURCE_DIR)/VIC_Helper.c\
$(APP_SOURCE_DIR)/Dtmf_keys.c \
$(APP_SOURCE_DIR)/XXTEA_Dtmf.c \
$(APP_SOURCE_DIR)/Dtmf.c \
$(APP_SOURCE_DIR)/Terminal.c\
$(APP_SOURCE_DIR)/Command.c\
$(APP_SOURCE_DIR)/Radio.c\
$(APP_SOURCE_DIR)/Inter_Tray_Comms.c

DRIVER_SRC = \
$(DRIVER_SOURCE_DIR)/Init_Ports.c \
$(DRIVER_SOURCE_DIR)/Reg_Ctrl.c \
$(DRIVER_SOURCE_DIR)/Comms_DTMF.c \
$(DRIVER_SOURCE_DIR)/Comms_UART.c\
$(DRIVER_SOURCE_DIR)/Comms_Radio.c\
$(DRIVER_SOURCE_DIR)/Comms_I2C.c\
$(DRIVER_SOURCE_DIR)/Sys_RTC.c

ALL_SRC = \
$(CORE_SRC) \
$(DRIVER_SRC) \
$(APP_SRC)


#--------------------------
# Compilation and Linking
#--------------------------

# Define all object files
ALL_OBJ = $(ALL_SRC:.c=.o)

TARGET_OBJS=$(addprefix $(BUILDS_DIR)/,$(notdir $(ALL_OBJ)))

default: hex

elf: $(ALL_OBJ) $(CRT0)
	$(CC) $(CFLAGS) $(TARGET_OBJS) -nostartfiles $(CRT0) $(LINKER_FLAGS)

hex: elf
	$(OBJCOPY) $(IMAGE_DIR)/csc.elf -O ihex $(IMAGE_DIR)/csc.hex
	
$(ALL_OBJ): %.o : %.c
	$(CC) -c $(CFLAGS) $< -o $(BUILDS_DIR)/$(@F)

burn:
	$(IMAGE_DIR)/lpc21isp $(IMAGE_DIR)/csc.hex /dev/ttyUSB0 115200 14746
	
burner:
	$(MAKE) --directory=$(BURNER_DIR) -f Makefile lpc21isp
	
burnerclean:
	$(MAKE) --directory=$(BURNER_DIR) -f Makefile lpcclean
	
keys: 
	perl $(SCRIPTS_DIR)/genDtmfKeys.pl > $(APP_SOURCE_DIR)/Dtmf_keys.c

makeCommands: $(APPS_SOURCE_DIR)/makeCommands.c $(APP_SOURCE_DIR)/XXTEA_Dtmf.c $(APP_SOURCE_DIR)/Dtmf_keys.c
	gcc -o $(BUILDS_DIR)/makeCommands \
$(APPS_SOURCE_DIR)/makeCommands.c \
$(APP_SOURCE_DIR)/XXTEA_Dtmf.c \
$(APP_SOURCE_DIR)/Dtmf_keys.c \
-I$(INCLUDES_DIR)/include

#----------
# Clean Up
#----------

clean: 
	rm -f $(BUILDS_DIR)/*
	rm -f $(IMAGE_DIR)/csc.*
