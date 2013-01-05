#include TextKeyTool/make_key.inc

WORKROOT=../../../../



LEXICON=$(WORKROOT)/lib/iask_lexicon_20110630/
LOGLIB=$(WORKROOT)/lib/libnx/

INCLUDES=-I$(LEXICON)/include/ \
		 -I$(LOGLIB)/include \
		 -I$(WORKROOT)/third/nginx/include/ \
		 -I$(WORKROOT)/lib/libwb/include/ \
		 -I$(WORKROOT)/lib/jsoncpp/include \
		 -I../../WordNetInclude/
LIBS=-L$(WORKROOT)/third/nginx/lib/ -lngx \
	 -L$(WORKROOT)/lib/libnx/lib/ -lnxlib \
	 -L$(WORKROOT)/lib/libwb/lib/ -lwb \
	 -L$(WORKROOT)/lib/iask_lexicon_20110630/lib -llexicon \
	 -L$(WORKROOT)/lib/iask_lexicon_20110630/example -lcommon -lustr -ltouni -lfromuni \
	 -L$(WORKROOT)/lib/jsoncpp/lib -ljson_linux-gcc-4.1.2_libmt\
	 -lpthread -lcrypt -lpcre -lcrypto -lcrypto -lz


EXES= PreWordSeg 

CXX = g++
DEBUG = 1
BINPATH = ../bin/

ifeq ($(DEBUG),1) 
	CFLAGS = -pipe  -O0 -W -Wall -Wpointer-arith -Wno-unused-parameter -Wunused-function -Wunused-variable -Wunused-value -g
else
	CFLAGS = -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Wunused-function -Wunused-variable -Wunused-value
endif

.PHONY: all 

all : $(EXES) 

PreWordSeg:  WeiboProcessor.o
	g++ $(CFLAGS) -DDOMAIN_SEMANTIC -o $@ PreWordSeg.cpp $^ $(INCLUDES) $(LIBS)  -L. -ldealSVDModel -lxs -m32

.SUFFIXES: .o .cpp .h

WeiboProcessor.o:  WeiboProcessor.cpp
	$(CXX) $(CFLAGS) -c $<  $(INCLUDES) -m32

clean: 
	rm -rf $(EXES)
	rm -rf *.o

install:
	cp $(EXES) $(WORKROOT)/../projects/MakeWordNet/bin
