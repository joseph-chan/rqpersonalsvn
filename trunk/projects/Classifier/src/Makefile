#include TextKeyTool/make_key.inc

WORKROOT=../../../



LEXICON=$(WORKROOT)/lib/iask_lexicon_20110630/
LOGLIB=$(WORKROOT)/lib/libnx/

INCLUDES=-I$(LEXICON)/include/ \
		 -I$(LOGLIB)/include \
		 -I$(WORKROOT)/third/nginx/include/ \
		 -I$(WORKROOT)/lib/libwb/include/ \
		 -I$(WORKROOT)/lib/jsoncpp/include \
		 -I$(WORKROOT)/lib/jenkins_hash/include/ \
		 -I../../WordNetInclude/
LIBS=-L$(WORKROOT)/third/nginx/lib/ -lngx \
	 -L$(WORKROOT)/lib/libnx/lib/ -lnxlib \
	 -L$(WORKROOT)/lib/libwb/lib/ -lwb \
	 -L$(WORKROOT)/lib/iask_lexicon_20110630/lib -llexicon \
	 -L$(WORKROOT)/lib/iask_lexicon_20110630/example -lcommon -lustr -ltouni -lfromuni \
	 -L$(WORKROOT)/lib/jsoncpp/lib -ljson_linux-gcc-4.1.2_libmt\
	 -L$(WORKROOT)/lib/jenkins_hash/lib -llookup \
	 -lpthread -lcrypt -lpcre -lcrypto -lcrypto -lz


# GetNBModel 产出贝叶斯分类模型，只是一个二分类模型，得到模型和预测都写在一起了。
EXES= GetNBModel

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

GetNBModel:  PrepareTrainingData.o
	g++ $(CFLAGS) -DDOMAIN_SEMANTIC -o $@ GetNBModel.cpp $^ $(INCLUDES) $(LIBS)  -L. -ldealSVDModel -lxs 

.SUFFIXES: .o .cpp .h

WeiboProcessor.o:  WeiboProcessor.cpp
	$(CXX) $(CFLAGS) -c $<  $(INCLUDES) 
PrepareTrainingData.o:  PrepareTrainingData.cpp
	$(CXX) $(CFLAGS) -c $<  $(INCLUDES) 

clean: 
	rm -rf $(EXES)
	rm -rf *.o

install:
	cp $(EXES) $(WORKROOT)/../projects/LoveMarriageIntend/bin
