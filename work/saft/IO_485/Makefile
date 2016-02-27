.SUFFIXES: .cpp .o
.DEFAULT: all

TARGET=IO_485.so
ifndef ${CC}
CC=arm-none-linux-gnueabi-g++
endif

DEBUG=-DNDEBUG
DLD=-L/usr/local/lib
LIBS=-lpthread -lrt -shared
CFLAGS= -g -Wall -O0 -fPIC

INCLUDE=-I./ -I/usr/local/include -I../comm/frame -I../comm/sys
SRC=../comm/sys/thread.cpp \
	../comm/sys/log.cpp \
	../comm/sys/Serial.cpp \
	./IO_485.cpp

OBJS=${SRC:.cpp=.o}

${TARGET}:${OBJS}
	${CC} ${DEBUG} ${OBJS} $(DLD) -o ${TARGET} ${LIBS}
	@echo ${TARGET} " build success"
.cpp.o:
	${CC} ${DEBUG} ${INCLUDE} -c $< ${CFLAGS} -o $@

.PHONY:clean
clean:
	@rm ${TARGET} ${OBJS}
	@echo ${TARGET} " clean success"

