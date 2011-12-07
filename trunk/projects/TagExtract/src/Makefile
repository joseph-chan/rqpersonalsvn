include TextKeyTool/make_key.inc

WORKROOT=../../../

OBJECTS=WeiboProcessor.o 
OBJECTS+=CStringTool.o 
OBJECTS+=CJParser.o 
OBJECTS+=CKeyWordsManager.o 
OBJECTS+=CConfigInfo.o 
OBJECTS+=CResource.o 
OBJECTS+=checkArticle.o 
OBJECTS+=minilzo.o 
OBJECTS+=get_keyword2.o 
OBJECTS+=corelativity.o 
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
	 -lcrypt -lpcre -lcrypto -lcrypto -lz


EXES= HotBlog

CXX = g++
DEBUG = 1
BINPATH = ../bin/

ifneq ($(DEBUG),1) 
	CFLAGS = -g -Wall -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE
else
	CFLAGS = -DDEBUG -g -Wall -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE
endif

.PHONY: all 

all : $(EXES) 

HotBlog: $(OBJECTS) strmatch.o HotBlog.cpp
	g++ -g -DDOMAIN_SEMANTIC -o $@ $(OBJECTS) strmatch.o HotBlog.cpp  $(LIBS) $(KEY_O_FILE) $(SEG_O_FILE) -I$(SEG_INC) -I$(KEY_INC) $(INCLUDES) -L. -ldealSVDModel -lxs -L$(SEG_LIB) $(SEG_LIB_FILE) 




.SUFFIXES: .o .cpp .h

.cpp.o: 
	$(CXX) $(CFLAGS) -c $<  -I$(KEY_INC) -I$(SEG_INC) $(INCLUDES)

clean: 
	rm -rf $(EXES)
	rm -rf $(OBJECTS)

install:
	cp $(EXES) $(BINPATH)
