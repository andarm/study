.SUFFIXES: .cpp .o
.DEFAULT: all

TARGET=Logic_RS.so
ifndef ${CC}
CC=arm-none-linux-gnueabi-g++
#CC=g++
endif

ifndef ${DEBUG}
DEBUG=-D_DEBUG
endif

DLD=-L/usr/local/lib -L../comm/thirdparty/socketcan/lib
LIBS=-lsocketcan -lpthread -lrt -shared
CFLAGS= -g -Wall -O0 -fPIC

INCLUDE=-I/usr/local/include/ -I../comm/frame -I../comm/sys -I../protocol -I../comm/db -I../comm/thirdparty -I../comm/thirdparty/rapidjson -I../comm/thirdparty/rapidjson/internal -I../comm/thirdparty/socketcan/include

SRC=../comm/sys/thread.cpp \
	../comm/sys/delegate.cpp \
	../comm/sys/iniaccess.cpp \
	../comm/sys/log.cpp \
	../comm/sys/crc.cpp \
	../comm/sys/plex.cpp \
	../comm/sys/logicmsgmng.cpp \
	../comm/frame/timerhelper.cpp \
	./RSSrv.cpp \

OBJS=${SRC:.cpp=.o}


${TARGET}:${OBJS}
	${CC} ${DEBUG} ${OBJS} $(DLD) -o ${TARGET} ${LIBS}
	@echo ${TARGET} "build success"
#	cp ../Logic_HT/libLogic_HT.so ../comm/lib/
.cpp.o:
	${CC} ${DEBUG} ${INCLUDE} -c $< ${CFLAGS} -o $@

.PHONY:clean
clean:
	@rm ${TARGET} ${OBJS}
	@echo ${TARGET} " clean success"


