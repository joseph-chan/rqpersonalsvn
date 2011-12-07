CC =	g++
CFLAGS =  -pipe  -O0 -W -Wall -Wpointer-arith -Wno-unused-parameter -Wunused-function -Wunused-variable -Wunused-value -g 
CPP =	gcc -E
LINK =	$(CC)

WORKROOT=../../
NGX=$(WORKROOT)/thirdsrc/nginx-1.0.10/
OBJS= nx_log.o
LIBDIR=$(WORKROOT)/third/nginx/lib/
INCLUDES = -I $(NGX)src/core \
	-I $(NGX)src/event \
	-I $(NGX)src/event/modules \
	-I $(NGX)src/os/unix \
	-I $(NGX)objs

all : $(OBJS)

$(OBJS): %.o: %.cpp
	$(CC) -c $< -o $@ $(INCLUDES) $(CFLAGS)
	ar -ruv libnxlog.a nx_log.o $(LIBDIR)/libngx.a

.PHONY : clean
clean :
	rm  nx_log.o
	rm  libnxlog.a
install :
	cp nx_log.h ../../lib/nx_log/include/
	cp libnxlog.a ../../lib/nx_log/lib/
