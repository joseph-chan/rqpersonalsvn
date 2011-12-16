#include TextKeyTool/make_key.inc

WORKROOT=../../../

OBJECTS=TagExtractor.o
OBJECTS+=WeiboProcessor.o 
OBJECTS+=CStringTool.o 
OBJECTS+=CJParser.o 
OBJECTS+=CConfigInfo.o 
OBJECTS+=CResource.o 
OBJECTS+=checkArticle.o 
OBJECTS+=minilzo.o 
#OBJECTS+=get_keyword2.o 
#OBJECTS+=corelativity.o 
#OBJECTS+=CMBTextTool.o 
#OBJECTS+=CBayesClassifier.o 
#OBJECTS+=bayesHM.o 
#OBJECTS+=rubLabel.o 
#OBJECTS+=wordsegSimple.o 
#OBJECTS+=CClassifier.o 

KEY_O_FILE=CTokenizer.o
KEY_O_FILE+=jenkins.o
KEY_O_FILE+=paichong.o
KEY_O_FILE+=CMBTextTool.o
KEY_O_FILE+=CTextKeyTool.o


LEXICON=$(WORKROOT)/lib/iask_lexicon_20110630/
LOGLIB=$(WORKROOT)/lib/libnx/

INCLUDES=-I$(LEXICON)/include/ \
		 -I$(LOGLIB)/include \
		 -I$(WORKROOT)/third/nginx/include/
LIBS=-L$(WORKROOT)/third/nginx/lib/ -lngx \
	 -L$(WORKROOT)/lib/libnx/lib/ -lnxlib \
	 -L$(WORKROOT)/lib/iask_lexicon_20110630/lib -llexicon \
	 -L$(WORKROOT)/lib/iask_lexicon_20110630/example -lcommon -lustr -ltouni -lfromuni \
	 -lpthread -lcrypt -lpcre -lcrypto -lcrypto -lz


EXES= TagExtract

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

TagExtract: $(OBJECTS) strmatch.o TagExtract.cpp
	g++ $(CFLAGS) -DDOMAIN_SEMANTIC -o $@ $(OBJECTS) strmatch.o TagExtract.cpp  $(LIBS) $(KEY_O_FILE) $(INCLUDES) -L. -ldealSVDModel -lxs




.SUFFIXES: .o .cpp .h

.cpp.o: 
	$(CXX) $(CFLAGS) -c $<  $(INCLUDES)

clean: 
	rm -rf $(EXES)
	rm -rf $(OBJECTS)

install:
	cp TagExtract /data4/ruqiang/projects/TagExtract/bin/ 
