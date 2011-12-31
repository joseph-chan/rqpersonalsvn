CC =	g++
CFLAGS =  -pipe  -O0 -W -Wall -Wpointer-arith -Wno-unused-parameter -Wunused-function -Wunused-variable -Wunused-value -g 
CPP =	gcc -E
LINK =	$(CC)

WORKROOT=../../
NGX=$(WORKROOT)/thirdsrc/nginx-1.0.10/
OBJS= nx_log.o
OBJS+= dictmatch.o
LIBDIR=$(WORKROOT)/third/nginx/lib/
INCLUDES = -I $(NGX)src/core \
	-I $(NGX)src/event \
	-I $(NGX)src/event/modules \
	-I $(NGX)src/os/unix \
	-I $(NGX)objs

all : $(OBJS) libnxlib.a

$(OBJS): %.o: %.cpp
	$(CC) -c $< -o $@ $(INCLUDES) $(CFLAGS)

libnxlib.a: $(OBJS)
	ar -ruv $@ $^ $(LIBDIR)/libngx.a

.PHONY : clean
clean :
	rm  *.o
	rm  libnxlib.a
install :
	cp nx_log.h ../../lib/libnx/include/
	cp dictmatch.h ../../lib/libnx/include/
	cp libnxlib.a ../../lib/libnx/lib/
