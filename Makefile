CC  = g++
CXX = g++

INCLUDES =

CFLAGS   = -g -Wall $(INCLUDES)
CXXFLAGS = -g -Wall $(INCLUDES) -std=c++0x

LDFLAGS = -g
LDLIBS  = -lpthread

.PHONY: default
default: bfclient
	rm -rf *~ a.out *.o *dSYM

bfclient: tuip.o bfclient.o bf_node.o routing_table.o

bfclient.o: bfclient.h bfclient.cpp bf_node.cpp bf_node.h routing_table.cpp routing_table.h tuip.cpp tuip.h

.PHONY: clean
clean:
	rm -f *~ a.out core *.o sender receiver

.PHONY: all
all: clean default
