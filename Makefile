CC	= cc
CFLAGS	=
LDFLAGS	= -lncurses

.PHONY: all clear

all: cct

cct: cct.o
	${CC} cct.o -o $@ ${LDFLAGS}

.SUFFIXES: .c.o
.c.o:
	${CC} ${CFLAGS} -c $< -o $@
