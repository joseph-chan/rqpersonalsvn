CC =    gcc
CFLAGS =  -pipe  -O0 -W -Wall -Wpointer-arith -Wno-unused-parameter -Wunused-function -Wunused-variable -Wunused-value -g
CPP =   g++
LINK =  $(CC)

OBJS=head_sort quick_sort

all: $(OBJS)

$(OBJS): % :%.cc 
	$(CPP) $< -o $@ $(CFLAGS)
