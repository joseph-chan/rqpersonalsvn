# 2. 20, 2006
####################################

include ../make.inc

EXES = CommonServer
all: $(EXES)

CommonServer:commonserver.c commonworker.o
	g++ -W -o $@ commonserver.c strmatch.o commonworker.o -I. -I$(INCLUDES) -L$(LIBDIR) -lparsejson  -ltransdoc -lmonitor  -ldispatch -lcommon  -lstdc++ $(THREAD_LIB)

commonworker.o:
	g++ -c -g commonworker.cpp -I. -I$(INCLUDES)

clean:
	rm  -rf $(EXES) commonworker.o
	
