# 2. 20, 2006
####################################

#######################################
# 各模块通用变量定义
#######################################
#CC = gcc -fpack-struct -g
CC = gcc -g

WORKROOT = ../../../
DEBUG = 0
LIBDIR = ../lib
LexiconLib=../../../lib/iask_lexicon_20110630/lib/
INCLUDES = -I../include \
		   -I$(WORKROOT)lib/libnx/include \
		   -I$(WORKROOT)/third/nginx/include

LIBS = -L$(WORKROOT)lib/libnx/lib -lnxlib \
	   -L$(WORKROOT)/third/nginx/lib -lngx \
	   -lcrypt -lpcre -lcrypto -lcrypto -lz

THREAD_LIB = -pthread #FreeBSD编译使用
#THREAD_LIB = -lpthread #Solaris编译使用

EXES = CommonServer test
all: $(EXES)

CommonServer:commonserver.c commonworker.o
	g++ -W -g -O0 -o $@ commonserver.c strmatch.o commonworker.o -I. $(INCLUDES) $(LIBS) -L$(LIBDIR) -lparsejson  -ltransdoc -lmonitor  -ldispatch -lcommon  -lstdc++ $(THREAD_LIB)

test:test.c
	$(CC) -W -O0 -o $@ test.c -I. $(INCLUDES)  -L$(LIBDIR) -L$(LexiconLib)  -ltransdoc -lmonitor  -ldispatch -lcommon -l pthread -DDEBUG

commonworker.o:
	g++ -c -g -O0 commonworker.cpp -I. $(INCLUDES)

clean:
	rm  -rf $(EXES) commonworker.o
	
