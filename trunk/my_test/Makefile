
CC =    gcc
CFLAGS =  -pipe  -O0 -W -Wall -Wpointer-arith -Wno-unused-parameter -Wunused-function -Wunused-variable -Wunused-value -g
CPP =   g++
LINK =  $(CC)

WORKROOT=../

OBJS=test.o

INCLUDEDIR=-I$(WORKROOT)/lib/libnx/include\
		   -I$(WORKROOT)/lib/jsoncpp/include \
		   -I$(WORKROOT)/third/nginx/include/ \
		   -I$(WORKROOT)/third/pcre/include/ \
		   -I$(WORKROOT)/lib/jenkins_hash/include/


#LIBDIR=-lpthread -lcrypt -lpcre -lcrypto -lcrypto -lz -lstdc++ \

#LIBDIR=-L$(WORKROOT)/lib/nx_log/lib -lnxlog \
#		-L$(WORKROOT)/third/nginx/lib -lngx \
#	-lpthread -lcrypt -lpcre -lcrypto -lcrypto -lz
LIBDIR=-L$(WORKROOT)/lib/libnx/lib -lnxlib \
	   -L$(WORKROOT)/third/nginx/lib -lngx \
	   -L$(WORKROOT)/third/pcre/lib -lpcre \
	   -L$(WORKROOT)/lib/jsoncpp/lib -ljson_linux-gcc-4.6_libmt\
	   -lpthread -lcrypt -lpcre -lcrypto -lz

#-L$(WORKROOT)/lib/jenkins_hash/lib -llookup \
	   #-L$(WORKROOT)/lib/jsoncpp/lib -ljson_linux-gcc-4.1.2_libmt\

all: test

test: $(OBJS)
	g++ -o test test.o  $(LIBDIR) 
$(OBJS): %.o: %.cpp 
		$(CPP) -c $< -o $@ $(INCLUDEDIR) $(CFLAGS)


clean:
	rm *.o test
