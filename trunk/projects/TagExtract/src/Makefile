#include TextKeyTool/make_key.inc

WORKROOT=../../../

OBJECTS=WeiboProcessor.o 
OBJECTS+=CStringTool.o 
OBJECTS+=CJParser.o 
OBJECTS+=CConfigInfo.o 
OBJECTS+=CResource.o 
OBJECTS+=checkArticle.o 
#OBJECTS+=minilzo.o 
#OBJECTS+=get_keyword2.o 
#OBJECTS+=corelativity.o 
#OBJECTS+=CMBTextTool.o 
#OBJECTS+=CBayesClassifier.o 
#OBJECTS+=bayesHM.o 
#OBJECTS+=rubLabel.o 
#OBJECTS+=wordsegSimple.o 
#OBJECTS+=CClassifier.o 

LEXICON=$(WORKROOT)/lib/iask_lexicon_20110630/
LOGLIB=$(WORKROOT)/lib/nx_log/

INCLUDES=-I$(LEXICON)/include/ \
		 -I$(LOGLIB)/include \
		 -I$(WORKROOT)/third/nginx/include/
LIBS=-L$(WORKROOT)/third/nginx/lib/ -lngx \
	 -L$(WORKROOT)/lib/nx_log/lib/ -lnx_log \
	 -lcrypt -lpcre -lcrypto -lcrypto -lz


EXES= TagExtract

CXX = g++
DEBUG = 1
BINPATH = ../bin/

ifneq ($(DEBUG),1) 
	CFLAGS = -pipe  -O0 -W -Wall -Wpointer-arith -Wno-unused-parameter -Wunused-function -Wunused-variable -Wunused-value -g
else
	CFLAGS = -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Wunused-function -Wunused-variable -Wunused-value
endif

.PHONY: all 

all : $(EXES) 

TagExtract: $(OBJECTS) strmatch.o TagExtract.cpp
	g++ -g -DDOMAIN_SEMANTIC -o $@ $(OBJECTS) strmatch.o TagExtract.cpp  $(LIBS) $(KEY_O_FILE) $(SEG_O_FILE) -I$(SEG_INC) -I$(KEY_INC) $(INCLUDES) -L. -ldealSVDModel -lxs -L$(SEG_LIB) $(SEG_LIB_FILE) 




.SUFFIXES: .o .cpp .h

.cpp.o: 
	$(CXX) $(CFLAGS) -c $<  $(INCLUDES)

clean: 
	rm -rf $(EXES)
	rm -rf $(OBJECTS)

install:
	cp $(EXES) $(BINPATH)
