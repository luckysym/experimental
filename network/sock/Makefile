
TARGET=communication
SOURCES=inetchannel.cpp localchannel.cpp
HEADERS=inetchannel.h localchannel.h sockdef.h
DEBUG=1
BINTYPE=DLL


CPP=g++
CPPFLAGS=
LD=g++
LDFLAGS=
FILE_EXTNAME=
RM=rm -fR
RMDIR=rmdir
MKDIR=mkdir -p
COPY=cp
OBJDIR=build
OBJS=$(addprefix $(OBJDIR)/, $(patsubst %.cpp, %.o, $(SOURCES)))
DEGUB_TAG=
TARGET_PREFIX=


ifneq ($(DEBUG),0)
	CPPFLAGS+=-g
	DEBUG_TAG=D
endif

ifeq ($(BINTYPE), DLL)
	CPPFLAGS+=-fPIC
	LDFLAGS+=-shared
	FILE_EXTNAME=so
	TARGET_PREFIX=lib
endif


TARGET:=$(OBJDIR)/$(TARGET_PREFIX)$(TARGET)$(DEBUG_TAG).$(FILE_EXTNAME)

.PHONY: all clean install

all: $(TARGET)
	$(LD) $(LDFLAGS) -o $(TARGET) $(OBJS)

install:
	$(MKDIR) $(PROGRAM)/include/network
	$(COPY) $(HEADERS) $(PROGRAM)/include/network
	$(COPY) $(OBJDIR)/*.so $(PROGRAM)/lib


$(TARGET): $(OBJDIR) $(OBJS)
	
clean:
	$(RM) $(OBJDIR)

$(OBJDIR):
	$(MKDIR) $(OBJDIR)

$(OBJDIR)/%.o: %.cpp
	$(CPP) -c $(CPPFLAGS) $< -o $@

