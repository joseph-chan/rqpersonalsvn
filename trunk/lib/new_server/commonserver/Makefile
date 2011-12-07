# 2. 20, 2006
####################################

include ../make.inc

EXES = CommonServer commonclient

all: $(LIBS) $(EXES)
.SUFFIXES:
.SUFFIXES: .o .c .h

CommonServer:commonserver.c commonworker.c
	$(CC) -W -o $@ commonserver.c commonworker.c -I. -I$(INCLUDES)  -L$(LIBDIR)  -lparsejson  -ltransdoc -lmonitor  -ldispatch -lcommon  -lstdc++ $(THREAD_LIB)

commonclient:commonclient.c
	$(CC) -W -o $@ commonclient.c -I. -I$(INCLUDES)  -L$(LIBDIR)  -ltransdoc -lmonitor  -ldispatch  -lcommon  $(THREAD_LIB)

.c:
	$(CC)  -o $@ $< $(CFLAGS)  

.c.o:
	if [ "`uname`" = "Linux" ];then\
		$(CC) -c -DDEBUG=$(DEBUG)  $< -I$(INCLUDES) ;\
	else\
		$(CC) -c -DDEBUG=$(DEBUG)  $< -I$(INCLUDES) ;\
	fi

clean:
	rm $(LIBS) $(EXES) *.o
	
