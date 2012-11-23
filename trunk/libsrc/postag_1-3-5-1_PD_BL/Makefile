#-----------------------------------------------------------------#
# Makefile for word POS tagging module - postag.                  #
# Copyright (c) 2007 Baidu                                        #
# David Dai	 2009.11  updated                                   #
#-----------------------------------------------------------------#

WORKROOT = ../..
ULIB = $(WORKROOT)/lib2-64
LIBNAME = postag

BIN_DIR = bin
SRC_DIR = src
LIB_DIR = lib
SO_DIR = solib
DATA_DIR = data
TEST_DIR = test
INCLUDE_DIR = include
OUTPUT_DIR = output

## generate automaton
#$(shell flex++ -8 -Cf -o$(SRC_DIR)/ChunkParser.cpp $(SRC_DIR)/ChunkParser.l)

SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst %.cpp,%.o,$(SOURCES))

SOURCES1 = $(wildcard $(SRC_DIR)/*.c)
OBJS1 = $(patsubst %.c,%.o,$(SOURCES1))

TEST_SOURCES = $(wildcard $(TEST_DIR)/*.cpp)
TEST_EXES = $(patsubst %.cpp,%,$(TEST_SOURCES))
BIN_EXES = $(patsubst $(TEST_DIR)/%,$(BIN_DIR)/%,$(TEST_EXES))

SEGTAG_SOURCES = $(wildcard $(TEST_DIR)/*.cxx)
TEST_SEGTAGS = $(patsubst %.cxx,%,$(SEGTAG_SOURCES))
BIN_SEGTAGS = $(patsubst $(TEST_DIR)/%,$(BIN_DIR)/%,$(TEST_SEGTAGS))

#-----------------------------------------------------------------#
INCLUDEFLAGS = -I$(INCLUDE_DIR) \
		-I$(ULIB)/dict/include \
		-I$(ULIB)/ullib/include \
		-I$(ULIB)/others-ex/include \
		-I$(WORKROOT)/public/odict \
		-I$(ULIB)/wordrank/include
		
LIBFLAGS = -L$(LIB_DIR) \
		-L$(ULIB)/ullib/lib \
		-L$(ULIB)/dict/lib \
		-L$(ULIB)/others-ex/lib \
		-L$(WORKROOT)/public/odict \
		-L$(ULIB)/wordrank/lib \
		-lwordrank -lodict -lullib_ex -luldict -lullib -lcrypto -lpthread
		
CXXFLAGS = -Wall -O3

## test platform, 32 or 64?
HARDWARE_PLATFORM := $(shell uname -i)
ifeq ($(HARDWARE_PLATFORM),x86_64)	
	CXXFLAGS = -Wall -O3 -fPIC
	ULIB = $(WORKROOT)/lib2-64
endif

USERFLAGS  = -D_REENTRANT -D_VERSION=\"1-3-5\"
CXX = g++ $(CXXFLAGS) $(USERFLAGS) 

.PHONY: all clean lib tool test dict

#-----------------------------------------------------------------#
# make all
#-----------------------------------------------------------------#
all: tool
	if [ ! -d $(OUTPUT_DIR)/lib ]; then mkdir -p $(OUTPUT_DIR)/lib; fi
	cp lib/lib$(LIBNAME).a $(OUTPUT_DIR)/lib
	if [ ! -d $(OUTPUT_DIR)/$(SO_DIR) ]; then mkdir -p $(OUTPUT_DIR)/$(SO_DIR); fi
	cp $(SO_DIR)/lib$(LIBNAME).so $(OUTPUT_DIR)/$(SO_DIR)
	if [ ! -d $(OUTPUT_DIR)/include ]; then mkdir -p $(OUTPUT_DIR)/include; fi
	cp include/i*.h $(OUTPUT_DIR)/include
		
#-----------------------------------------------------------------#
# make library
#-----------------------------------------------------------------#
lib: $(OBJS) $(OBJS1)
	if [ ! -d $(LIB_DIR) ]; then mkdir $(LIB_DIR); fi
	ar -ruv $(LIB_DIR)/lib$(LIBNAME).a $(OBJS) $(OBJS1)
	if [ ! -d $(SO_DIR) ]; then mkdir $(SO_DIR); fi
	$(CXX) -fPIC -shared -g  -Werror -Wl -o $(SO_DIR)/lib$(LIBNAME).so $(OBJS) $(OBJS1)
	
$(OBJS): %.o: %.cpp
	$(CXX) -c $< -o $@ $(INCLUDEFLAGS) $(LIBFLAGS)
	
$(OBJS1): %.o: %.c
	$(CXX) -c $< -o $@ $(INCLUDEFLAGS) $(LIBFLAGS)

#-----------------------------------------------------------------#
# make tool
#-----------------------------------------------------------------#
tool: lib $(TEST_EXES)
	
$(TEST_EXES): %: %.cpp
	$(CXX) $< -o $@ $(INCLUDEFLAGS) -l$(LIBNAME) $(LIBFLAGS)
	mv $@ $(BIN_DIR)/
	
#-----------------------------------------------------------------#
# make dict
#-----------------------------------------------------------------#
dict: tool
	if [ ! -d $(DATA_DIR)/tagdict ]; then mkdir -p $(DATA_DIR)/tagdict; fi
	bin/buildTag -c dict/raw/term_tag_freq.txt -o data/tagdict	
	bin/buildChunk -r dict/raw/chunk_rule.txt data/chkdict/

#-----------------------------------------------------------------#
# make test [注意：必须和分词库配合使用!]
#-----------------------------------------------------------------#
test: lib $(TEST_SEGTAGS)

$(TEST_SEGTAGS): %: %.cxx
	$(CXX) $< -o $@ $(INCLUDEFLAGS) -I$(ULIB)/wordner/include -L$(ULIB)/wordner/lib \
	-I$(ULIB)/wordseg -L$(ULIB)/wordseg -I../wordrank/include -L../wordrank/lib \
	-lwordner -lsegment -l$(LIBNAME) -lwordrank $(LIBFLAGS) 
	mv $@ $(BIN_DIR)/
	
#-----------------------------------------------------------------#
# make clean
#-----------------------------------------------------------------#
clean:
	rm -f $(OBJS) $(OBJS1) $(TEST_EXES) $(BIN_EXES) $(LIB_DIR)/*.a $(SO_DIR)/*.so $(TEST_SEGTAGS) $(BIN_SEGTAGS)
	rm -f $(BIN_DIR)/core.* $(BIN_DIR)/gmon.* $(TEST_DIR)/gmon.* ./core.*
	if [ -d $(OUTPUT_DIR) ]; then rm -r $(OUTPUT_DIR); fi
