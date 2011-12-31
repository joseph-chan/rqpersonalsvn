#include TextKeyTool/make_key.inc

WORKROOT=../../

OBJECTS=CStringTool.o 
OBJECTS+=CJParser.o 
OBJECTS+=CConfigInfo.o 
OBJECTS+=CResource.o 
OBJECTS+=checkArticle.o 
OBJECTS+=CMBTextTool.o 
#OBJECTS+=get_keyword2.o 
#OBJECTS+=corelativity.o 
#OBJECTS+=CBayesClassifier.o 
#OBJECTS+=bayesHM.o 
#OBJECTS+=rubLabel.o 
#OBJECTS+=wordsegSimple.o 
#OBJECTS+=CClassifier.o 

KEY_O_FILE=CTokenizer.o
KEY_O_FILE+=jenkins.o
KEY_O_FILE+=paichong.o
#KEY_O_FILE+=CMBTextTool.o
KEY_O_FILE+=CTextKeyTool.o


LEXICON=$(WORKROOT)/lib/iask_lexicon_20110630/
LOGLIB=$(WORKROOT)/lib/libnx/

INCLUDES=-I$(LEXICON)/include/ \
		 -I$(LOGLIB)/include \
		 -I$(WORKROOT)/third/nginx/include/ \
		 -I$(LEXICON)/include/
LIBS=-L$(WORKROOT)/third/nginx/lib/ -lngx \
	 -L$(WORKROOT)/lib/libnx/lib/ -lnxlib \
	 -L$(WORKROOT)/lib/iask_lexicon_20110630/lib -llexicon \
	 -L$(WORKROOT)/lib/iask_lexicon_20110630/example -lcommon -lustr -ltouni -lfromuni \
	 -lpthread -lcrypt -lpcre -lcrypto -lcrypto -lz


OUTPUT= libwb.a

CXX = g++
DEBUG = 0
BINPATH = ../bin/

ifeq ($(DEBUG),1) 
	CFLAGS = -pipe  -O0 -W -Wall -Wpointer-arith -Wno-unused-parameter -Wunused-function -Wunused-variable -Wunused-value -g
else
	CFLAGS = -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Wunused-function -Wunused-variable -Wunused-value
endif

.PHONY: all 

all: $(OBJECTS) $(OUTPUT) minilzo.o

$(OBJECTS): %.o: %.cpp
	g++ -c $< -o $(CFLAGS) -DDOMAIN_SEMANTIC -o $@  $(INCLUDES) 

minilzo.o: minilzo.c
	g++ -c $< -o $(CFLAGS) -DDOMAIN_SEMANTIC -o $@  $(INCLUDES) 
	

$(OUTPUT): $(OBJECTS) strmatch.o $(KEY_O_FILE)
	ar -ruv $@ $^ 


.SUFFIXES: .o .cpp .h

.cpp.o: 
	$(CXX) $(CFLAGS) -c $<  $(INCLUDES)

clean: 
	rm -rf $(OUTPUT)
	rm -rf $(OBJECTS)

install:
	cp libwb.a ../../lib/libwb/lib/
	cp *.h ../../lib/libwb/include
