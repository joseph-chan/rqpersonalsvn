CC =	g++
CFLAGS =  -pipe  -O0 -W -Wall -Wpointer-arith -Wno-unused-parameter -Wunused-function -Wunused-variable -Wunused-value -g 
CPP =	gcc -E
LINK =	$(CC)

WORKROOT=../../
OBJS= utf8.o
LIBDIR=$(WORKROOT)/third/utf8_utils/lib/
INCLUDES = 

all : $(OBJS) librqlib.a

$(OBJS): %.o: %.c
	$(CC) -c $< -o $@ $(INCLUDES) $(CFLAGS)

librqlib.a: $(OBJS)
	ar -ruv $@ $^

.PHONY : clean
clean :
	rm  *.o
	rm  librqlib.a
install :
	cp utf8.h ../../lib/rqlib/include/
	cp librqlib.a ../../lib/rqlib/lib/
	cp librqlib.a ../../lib/
