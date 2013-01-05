# 2. 20, 2006
####################################

#######################################
# 各模块通用变量定义
#######################################
#CC = gcc -fpack-struct -g
CC = gcc -g

WORKROOT = ../../../../
LIBDIR = ../lib
LexiconLib=../../../lib/iask_lexicon_20110630/lib/
INCLUDES = -I$(WORKROOT)/lib/commonserver/include \
		   -I$(WORKROOT)lib/libnx/include \
		   -I$(WORKROOT)lib/iask_lexicon_20110630/include \
		   -I$(WORKROOT)/lib/libwb/include/ \
           -I$(WORKROOT)/lib/jenkins_hash/include \
		   -I../../WordNetInclude/ \
		   -I$(WORKROOT)/third/nginx/include

LIBS = -L$(WORKROOT)lib/libnx/lib -lnxlib \
	   -L$(WORKROOT)/third/nginx/lib -lngx \
       -L$(WORKROOT)/lib/jenkins_hash/lib -llookup \
	   -L$(WORKROOT)/lib/commonserver/lib -lparsejson  -ltransdoc -lmonitor  -ldispatch -lcommon \
	   -L$(WORKROOT)/lib/iask_lexicon_20110630/lib/ \
	   -L$(WORKROOT)/lib/libwb/lib/ -lwb \
	   -lcrypt -lpcre -lcrypto -lcrypto -lz -pthread

THREAD_LIB = -pthread #FreeBSD编译使用
#THREAD_LIB = -lpthread #Solaris编译使用

DEBUG = 1
ifeq ($(DEBUG),1) 
	CFLAGS = -pipe  -O0 -W -Wall -Wpointer-arith -Wno-unused-parameter -Wunused-function -Wunused-variable -Wunused-value -g
else
	CFLAGS = -pipe  -O -W -Wall -Wpointer-arith -Wno-unused-parameter -Wunused-function -Wunused-variable -Wunused-value
endif

EXES = CommonServer test
all: $(EXES)

CommonServer:commonserver.c commonworker.o WordNetIndexor.o
	g++ $(CFLAGS) -o $@ commonserver.c strmatch.o commonworker.o WordNetIndexor.o -I. $(INCLUDES) $(LIBS)
test:test.c
	gcc $(CFLAGS) -o $@ test.c -I. $(INCLUDES) $(LIBS) -DDEBUG

commonworker.o:
	g++ $(CFLAGS) -c commonworker.cpp -I. $(INCLUDES)
WordNetIndexor.o:
	g++ $(CFLAGS) -c  WordNetIndexor.cpp -I. $(INCLUDES)

clean:
	rm  -rf $(EXES) commonworker.o WordNetIndexor.o
	
install:
	cp  $(EXES) /data4/ruqiang/projects/WordNetServer/bin
