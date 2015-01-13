SHELL=/bin/sh

BASEDIR=.

LIB_DIR=$(BASEDIR)/lib/
BIN_DIR=$(BASEDIR)/bin/
OBJECT_DIR=$(BASEDIR)/obj/
SRC_DIR=$(BASEDIR)/src
INC_DIR=$(BASEDIR)/inc


$(shell mkdir -p ${LIB_DIR})
$(shell mkdir -p ${BIN_DIR})
$(shell mkdir -p ${OBJECT_DIR})

RM=rm -rf

#------------------

CC=gcc
CXX=g++
SHARED=-shared -o
FPIC=-c

SRC_OBJECT= $(SRC_DIR)/time_posix.cpp	\
		    $(SRC_DIR)/sock_posix.cpp

INC_OBJECT= $(INC_DIR)/timei.h	\
			$(INC_DIR)/socki.h

LD_LIBS = -ldl -lm -lrt
LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu

OBJECT=	time_posix.o sock_posix.o

LIB_OBJECT=libutil.so

#-------------------

.PHONY:all
all:$(LIB_OBJECT)

$(LIB_OBJECT):$(OBJECT)
	$(CXX) $(OBJECT) -L$(LD_LIBRARY_PATH) $(LD_LIBS) $(SHARED) -fPic -o $(LIB_OBJECT)
	mv $(LIB_OBJECT) $(LIB_DIR)

$(OBJECT):$(SRC_OBJECT) 
	$(CXX) $(FPIC) -I$(INC_DIR) $(SRC_OBJECT)

clean:
	$(RM) $(OBJECT_DIR)
