
CPP=g++
CPP_FLAGS=-c
SOURCES=inetsock.cpp
OBJS=inetsock.o
TARGET=inetsock.o
RM=rm -f


.PHONY: all clean

all: $(TARGET)


clean:
	$(RM) $(OBJS)


inetsock.o: inetsock.cpp
	$(CPP) $(CPP_FLAGS) inetsock.cpp
