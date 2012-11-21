#-----------------------------------------------------------------#
# Makefile for word segment module - wordseg.                     #
# Copyright (c) 2007 Baidu                                        #
# David Dai	2007.8.10                                         #
#-----------------------------------------------------------------#

WORKROOT = ../..
ULIB = $(WORKROOT)/lib2-64
PUBLIC = $(WORKROOT)/public/odict/output/
LIBNAME = segment

BIN_DIR = bin
SRC_DIR = src
LIB_DIR = lib2-64
TEST_DIR = test
INCLUDE_DIR = include
OUTPUT_DIR = output
TOOL_DIR = tools
SO_DIR = so

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst %.cpp,%.o,$(SOURCES))

SOURCES1 = $(wildcard $(SRC_DIR)/*.c)
OBJS1 = $(patsubst %.c,%.o,$(SOURCES1))

TEST_SOURCES = $(wildcard $(TEST_DIR)/*.cpp)
TEST_EXES = $(patsubst %.cpp,%,$(TEST_SOURCES))
BIN_EXES = $(patsubst $(TEST_DIR)/%,$(BIN_DIR)/%,$(TEST_EXES))

#-----------------------------------------------------------------#
INCLUDEFLAGS =	-I$(INCLUDE_DIR)/ \
		-I$(ULIB)/others-ex/include \
		-I$(ULIB)/dict/include \
		-I$(ULIB)/ullib/include \
		-I$(ULIB)/ccode/include \
		-I$(ULIB)/libcrf/include \
        -I$(PUBLIC)/include/
		
LIBFLAGS  =	-L$(LIB_DIR)/ \
		-L$(ULIB)/others-ex/lib \
		-L$(ULIB)/ullib/lib \
		-L$(PUBLIC)/ -lodict \
		-L$(ULIB)/dict/lib \
		-L$(ULIB)/ccode/lib -lulccode  \
		-L$(ULIB)/libcrf/lib -lcrf \
		-luldict -lullib -lullib_ex \
		-lcrypto -lm -lz -lpthread
		
CXXFLAGS = -Wall -O3 -fPIC 

## test platform, 32 or 64?
HARDWARE_PLATFORM := $(shell uname -i)
ifeq ($(HARDWARE_PLATFORM),x86_64)	
	CXXFLAGS = -Wall -O3 -fPIC  -Wno-deprecated
	ULIB = $(WORKROOT)/lib2-64
endif

USERFLAGS  = -DLINUX -DDEBUG_OFF -D_REENTRANT -DSEGVER=\"2-0-5\" -Wall
CXX = g++ $(CXXFLAGS) $(USERFLAGS) 

.PHONY: all clean lib test

#-----------------------------------------------------------------#
# make all
#-----------------------------------------------------------------#
all: test  
	if [ ! -d $(OUTPUT_DIR) ]; then mkdir $(OUTPUT_DIR); fi
	cp lib/lib$(LIBNAME).a include/*.h $(OUTPUT_DIR)
	if [ ! -d $(TOOL_DIR) ]; then mkdir $(TOOL_DIR); fi                       
	chmod +x $(TEST_DIR)/checkworddictman.pl                                              
	cp $(TEST_DIR)/checkworddictman.pl $(TOOL_DIR)/ 
	if [ ! -d $(OUTPUT_DIR)/so ]; then mkdir $(OUTPUT_DIR)/so; fi
	cp $(SO_DIR)/libwordseg.so $(OUTPUT_DIR)/so	

#-----------------------------------------------------------------#
#make so     
# -----------------------------------------------------------------#
libwordseg.so:
	$(CXX)  -shared -g -o $(SO_DIR)/libwordseg.so $(SO_DIR)/wordseg_so.cpp $(SRC_DIR)/*.o  $(INCLUDEFLAGS) -l$(LIBNAME) $(LIBFLAGS) -lc

#-----------------------------------------------------------------#
# make library
#-----------------------------------------------------------------#
lib: $(OBJS) $(OBJS1)
	if [ ! -d $(LIB_DIR) ]; then mkdir $(LIB_DIR); fi
	ar -ruv $(LIB_DIR)/lib$(LIBNAME).a $(OBJS) $(OBJS1)	

#-----------------------------------------------------------------#
# make test	
#-----------------------------------------------------------------#
test: lib $(TEST_EXES) libwordseg.so
	
$(TEST_EXES): %: %.cpp
	$(CXX) $< -o $@ $(INCLUDEFLAGS) -l$(LIBNAME) $(LIBFLAGS)
	mv $@ $(BIN_DIR)/
			
$(OBJS): %.o: %.cpp
	$(CXX) -c $< -o $@ $(INCLUDEFLAGS) 
	
$(OBJS1): %.o: %.c
	$(CXX) -c $< -o $@ $(INCLUDEFLAGS) 
#-----------------------------------------------------------------#
# make clean
#-----------------------------------------------------------------#
clean:
	rm -f $(OBJS) $(OBJS1) $(TEST_EXES) $(BIN_EXES) $(LIB_DIR)/*.a
	rm -f $(BIN_DIR)/core.* $(BIN_DIR)/gmon.* $(TEST_DIR)/gmon.*
	if [ -d $(OUTPUT_DIR) ]; then rm  -r $(OUTPUT_DIR); fi
	if [ -d $(TOOL_DIR) ]; then rm -r $(TOOL_DIR); fi 
	rm -rf $(SO_DIR)/*.o $(SO_DIR)/*.so
