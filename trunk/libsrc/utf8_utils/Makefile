CC =	g++
CFLAGS =  -pipe  -O0 -W -Wall -Wpointer-arith -Wno-unused-parameter -Wunused-function -Wunused-variable -Wunused-value -g 
CPP =	gcc -E
LINK =	$(CC)

WORKROOT=../../
OBJS= utf8.o
LIBDIR=$(WORKROOT)/third/utf8_utils/lib/
INCLUDES = 

all : $(OBJS) libutf8_utils.a

$(OBJS): %.o: %.c
	$(CC) -c $< -o $@ $(INCLUDES) $(CFLAGS)

libutf8_utils.a: $(OBJS)
	ar -ruv $@ $^

.PHONY : clean
clean :
	rm  *.o
	rm  libutf8_utils.a
install :
	cp utf8.h ../../lib/utf8_utils/include/
	cp libutf8_utils.a ../../lib/utf8_utils/lib/
