
CC =    gcc
CFLAGS =  -pipe  -O0 -W -Wall -Wpointer-arith -Wno-unused-parameter -Wunused-function -Wunused-variable -Wunused-value -g
CPP =   g++
LINK =  $(CC)

WORKROOT=../

OBJS=test.o

INCLUDEDIR=-I$(WORKROOT)/lib/nx_log/include\
		   -I$(WORKROOT)/third/nginx/include/

#LIBDIR=-lpthread -lcrypt -lpcre -lcrypto -lcrypto -lz -lstdc++ \

#LIBDIR=-L$(WORKROOT)/lib/nx_log/lib -lnxlog \
#		-L$(WORKROOT)/third/nginx/lib -lngx \
#	-lpthread -lcrypt -lpcre -lcrypto -lcrypto -lz
LIBDIR=-L$(WORKROOT)/lib/nx_log/lib -lnxlog \
	   -L$(WORKROOT)/third/nginx/lib -lngx \
	-lpthread -lcrypt -lpcre -lcrypto -lcrypto -lz

all: test

$(OBJS): %.o: %.cpp 
		$(CPP) -c $< -o $@ $(INCLUDEDIR) $(CFLAGS)

test: $(OBJS)
	g++ -o test test.o  $(LIBDIR) 

clean:
	rm *.o test
