
ifeq ($(MAC),64)
    include ./makefile.64.env
    LIBPATH=$(WORKROOT)lib2-64
    SHA=-Wshadow
else
    include ./makefile.env
    LIBPATH=$(WORKROOT)lib2
endif

GCC=g++

OBJ=crftest

CRFLIB=../output/

INCLUDE_PATH=-I$(ULLIB_PATH)/include/ \
	     -I$(CRFLIB)/include \


LIB_PATH=-L$(CRFLIB)/lib -lcrf \
	 -L$(ULLIB_PATH)/lib/ -lullib \
	 -lpthread

CPPFLAGS= -g -pg -Wall $(SHA) -W -Winline -Werror  -Wno-unused-parameter   -Wno-unused-function \
		   -DVERSION="\"$(OBJ) 2.1.2.0\"" -DCVSTAG="\"$(OBJ)_2-1-2-0_PD_BL\"" \
		   -DPROJNAME=\"$(OBJ)\" -DNDEBUG


.PHONY: all clean 
all:  libcrf.so crftest

crftest : crftest.o 
		$(GCC) -o $@ $^ $(LIB_PATH)

crftest_thread : crftest_thread.o myopt.o $
		$(GCC) -DCRF -o $@ $^ $(LIB_PATH)

clean:
	rm -f *.o crftest crftest_thread
	rm -f *.so
	rm -rf ./output

libcrf.so : libcrf_so.cpp $(CRFLIB)/lib/libcrf.a 
	$(GCC) $(CPPFLAGS) -DCRF -fpic -shared libcrf_so.cpp -o libcrf.so $(INCLUDE_PATH) $(LIB_PATH)

crftest.o : crftest.cpp
	$(GCC) $(CPPFLAGS) -c $< -o $@ $(INCLUDE_PATH)

output:
	rm -rf ./output
	mkdir ./output
	cp crftest ./output

release:
	scp crftest img@ai-apptest-img11.ai01:~/crftest/
