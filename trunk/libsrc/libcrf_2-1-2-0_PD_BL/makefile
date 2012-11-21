
ifeq ($(MAC),64)
    include ./makefile.64.env
    LIBPATH=$(WORKROOT)lib2-64
    SHA=-Wshadow
else
    include ./makefile.env
    LIBPATH=$(WORKROOT)lib2-64
endif

GCC=g++

OBJ=libcrf.a



INCLUDE_PATH=-I$(ULLIB_PATH)/include/  
#	     -I$(UBLIB)/include/ \
			 

LIB_PATH=-L$(ULLIB_PATH)/lib/ -lullib 
#		 -L$(UBLIB)/lib  -lub \


CPPFLAGS= -g -O3 -Wall $(SHA)\
		   -DVERSION="\"$(OBJ) 2.1.2.0\"" -DCVSTAG="\"$(OBJ)_2-1-2-0_PD_BL\"" \
		   -DPROJECT_NAME=\"$(OBJ)\" -DNDEBUG -fPIC


.PHONY: all clean

all: 
	rm -rf ./output
	mkdir -p ./output/include
	mkdir -p ./output/lib
	mkdir -p ./output/so
	mkdir -p ./output/demo
	make CRF
	cp *.h ./output/include/
	cp *.a ./output/lib
	make so
	cp crftest/*.so output/so
	cp crftest/crftest output/demo

CRF : $(OBJ)
	rm -rf *.o


$(OBJ) : node.o  min_heap.o model.o tag.o seg_tag.o lib_crf.o 
	ar -crv $@ $^


clean:
	rm -f *.o $(OBJ)
	rm -rf *.so
	rm -rf ./output
	cd crftest && make clean

so:
	cd crftest && make clean &&make

%.o : %.cpp
	$(GCC) $(CPPFLAGS) $(MACROFLAG) -c $< -o $@ $(INCLUDE_PATH)

output: $(OBJ)
	rm -rf ./output
	mkdir -p ./output/include
	mkdir -p ./output/lib
	mkdir -p ./output/so
	cp *.h ./output/include/
	cp *.a ./output/lib

