.SUFFIXES: .cpp .o
.DEFAULT: all



CC=g++

TARGET=testSQL
DLD=-L./linux_sqlite/lib 
LIBS=-lsqlite3
CFLAGS= -g  -O0 
SRC=./main.cpp

OBJS=${SRC:.cpp=.o}
${TARGET}:${OBJS}
	${CC} ${DEBUG} ${OBJS} $(DLD) -o ${TARGET} ${LIBS}
	@echo ${TARGET} "build success"
.cpp.o:
	${CC} ${DEBUG} ${INCLUDE} -c $< ${CFLAGS} -o $@

.PHONY:clean
	clean:
	@rm ${TARGET} ${OBJS}
	@echo ${TARGET} " clean success"
