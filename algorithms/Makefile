CC =    gcc
CFLAGS =  -pipe  -O0 -W -Wall -Wpointer-arith -Wno-unused-parameter -Wunused-function -Wunused-variable -Wunused-value -g
CPP =   g++
LINK =  $(CC)

OBJS=heap_sort quick_sort binary_search_tree binary_search positive_negative list_circle merge_sort find_two_substring find_two_substring_daiwei

all: $(OBJS)

$(OBJS): % :%.cc 
	$(CPP) $< -o $@ $(CFLAGS)

clean:
	rm $(OBJS)
