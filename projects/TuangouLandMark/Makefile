
WORKROOT=../../



LOGLIB=$(WORKROOT)/lib/libnx/

INCLUDES=-I$(LOGLIB)/include \
	 	 -I$(WORKROOT)/third/pcre/include/ \
		 -I$(WORKROOT)/third/nginx/include/ \
		 -I$(WORKROOT)/lib/libwb/include/ 

#		 -I$(WORKROOT)/lib/jenkins_hash/include \
#-I$(WORKROOT)/lib/jsoncpp/include \

LIBS=-L$(WORKROOT)/third/nginx/lib/ -lngx \
	 -L$(WORKROOT)/lib/libnx/lib/ -lnxlib \
	 -L$(WORKROOT)/lib/libwb/lib/ -lwb \
	 -L$(WORKROOT)/third/pcre/lib/ -lpcre \
	 -lpthread -lcrypt -lcrypto -lz

	#-L$(WORKROOT)/lib/jenkins_hash/lib -llookup \
	 #-L$(WORKROOT)/lib/iask_lexicon_20110630/lib -llexicon \
	 #-L$(WORKROOT)/lib/iask_lexicon_20110630/example -lcommon -lustr -ltouni -lfromuni \
	 #-L$(WORKROOT)/lib/jsoncpp/lib -ljson_linux-gcc-4.1.2_libmt\
	 #-L$(WORKROOT)/lib/libwb/lib/ -lwb \

EXES= GetLandMark

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

GetLandMark:  main.cpp  TuanDanProcessor.o ac.o
	g++ $(CFLAGS) -DDOMAIN_SEMANTIC -o $@ $^ $(INCLUDES) $(LIBS)  -L.

.SUFFIXES: .o .cpp .h

.cpp.o:
	$(CXX) $(CFLAGS) -c $<  $(INCLUDES)

clean: 
	rm -rf $(EXES)
	rm -rf *.o

install:
	scp $(EXES) sys@192.168.11.38:ruqiang/projects/tuangou/bin/
