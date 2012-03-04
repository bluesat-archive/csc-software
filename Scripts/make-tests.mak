# BLUESAT Critical Systems Computer Unit Test Builder Script
# Copyright BLUESAT Project 2012
# Version 1.0 
# Author: Colin Tan
#
# This script will build the unit tests for the BLUESat OS
#


#------------- 
# Directories
#-------------

CSC_DIR           =..
IMAGE_DIR			=../Dist
SCRIPTS_DIR			=.
UNIT_TESTS_DIR		=../Unit_Tests
TEST_SUITES			=$(UNIT_TESTS_DIR)/Test_Suites


#---------------------
# Compiler / Settings
#---------------------
C			=gcc
WARNINGS	=-Wall 
CFLAGS	=-I. -I$(UNIT_TESTS_DIR) $(patsubst %,-I%,$(subst :, ,$(INC_DIRS)))
OUTPUT	=-o $(IMAGE_DIR)/$(<F).exe


#--------------
# Source files
#--------------
# files which should not be compiled need to be of another type instead of *.c/*.h
# it is suggested changing to *.c.ignore/*.h.ignore

CODE_TEMPLATE =test_*.c
EXEC_FILES    = $(shell  ls $(IMAGE_DIR)/test_*.exe)
TEST_SUBDIRS  =/test/
FIND_TESTS    =/usr/bin/find .. -name $(CODE_TEMPLATE) | grep $(TEST_SUBDIRS)
CODE_SUBDIRS  =/src/
FIND_CODE     =/usr/bin/find .. -name $(CODE_TEMPLATE) | grep $(CODE_SUBDIRS)
INC_SUBDIRS   =includes
FIND_INCLUDES =/usr/bin/find .. -type d -name $(INC_SUBDIRS)

INC_DIRS = $(shell $(FIND_INCLUDES))
TESTS  = $(shell  $(FIND_TESTS))
CUTEST = $(shell ls $(UNIT_TESTS_DIR)/*.c) 
SUITES = $(shell ls $(TEST_SUITES)/*.c) 

TEST_CORE = $(CUTEST)\
$(SUITES)

#--------------------------
# Compilation and Linking
#--------------------------
ALL_OBJ = $(TESTS:.c=.o)
all: buildTests runAllTests

buildTests: $(ALL_OBJ)	
	
runAllTests: 
	sh run_tests.sh $(IMAGE_DIR)

$(ALL_OBJ) : %.o : %.c
	$(C) $(TEST_CORE) $< $(CFLAGS) $(OUTPUT)

   
#----------
# Clean Up
#----------

clean: 
	rm -f $(IMAGE_DIR)/*
