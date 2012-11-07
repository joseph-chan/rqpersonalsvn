#include TextKeyTool/make_key.inc

WORKROOT=../../

LOGLIB=$(WORKROOT)/lib/libnx/

INCLUDES=-I$(LOGLIB)/include \
		 -I$(WORKROOT)/third/nginx/include/ 
LIBS=-L$(WORKROOT)/third/nginx/lib/ -lngx \
	 -L$(WORKROOT)/lib/libnx/lib/ -lnxlib \
	 -lm


OBJECTS=lookup.o
OUTPUT= liblookup.a

CXX = g++
DEBUG = 0
BINPATH = ../bin/

ifeq ($(DEBUG),1) 
	CFLAGS = -pipe  -O0 -W -Wall -Wpointer-arith -Wno-unused-parameter -Wunused-function -Wunused-variable -Wunused-value -g
else
	CFLAGS = -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Wunused-function -Wunused-variable -Wunused-value
endif

.PHONY: all 

all: $(OBJECTS) $(OUTPUT)

$(OBJECTS): %.o: %.cpp
	g++ -c $< -o $(CFLAGS) -DDOMAIN_SEMANTIC -o $@  $(INCLUDES) 
	

$(OUTPUT): $(OBJECTS)
	ar -ruv $@ $^ 


.SUFFIXES: .o .cpp .h

.c.o: 
	$(CXX) $(CFLAGS) -c $<  $(INCLUDES)

clean: 
	rm -rf $(OUTPUT)
	rm -rf $(OBJECTS)

install:
	cp liblookup.a ../../lib/jenkins_hash/lib/
	cp *.h ../../lib/jenkins_hash/include
