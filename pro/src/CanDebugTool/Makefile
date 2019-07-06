.SUFFIXES: .cpp .o
.DEFAULT: all

TARGET=CANDebugTool
ifndef ${CC}
CC=arm-none-linux-gnueabi-g++
#CC=g++
#CC=g++
endif

ifndef ${DEBUG}
DEBUG=-D_DEBUG
endif

DLD=-L/usr/local/lib -L../comm/thirdparty/socketcan/lib -L../comm/can/can_arm
#LIBS=-lsocketcan -lpthread -lrt -shared
LIBS=-lpthread -lrt  
CFLAGS= -g -Wall -O0 

INCLUDE=-I/usr/local/include/ -I../comm/frame -I../comm/sys -I../protocol -I../comm/db -I../comm/thirdparty -I../comm/thirdparty/ -I../comm/can/

SRC= \
     ../comm/sys/log.cpp \
     ../comm/sys/thread.cpp \
     ../comm/sys/logicmsgmng.cpp \
     ./SaftopCAN.cpp	\
     ./SocketCAN.cpp \
     ./main.cpp

OBJS=${SRC:.cpp=.o}


${TARGET}:${OBJS}
	${CC} ${DEBUG} ${OBJS} $(DLD) -o ${TARGET} ${LIBS}
	@echo ${TARGET} "build success"
#	cp ../Logic_HT/libLogic_HT.so ../comm/lib/
	cp CANDebugTool  /home/saftop/DL4/usr/saftop/
.cpp.o:
	${CC} ${DEBUG} ${INCLUDE} -c $< ${CFLAGS} -o $@

.PHONY:clean
clean:
	@rm ${TARGET} ${OBJS}
	@echo ${TARGET} " clean success"


