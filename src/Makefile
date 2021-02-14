TARGET=gpcc

CFLAGS=	-g -Wall

SRCS=			\
	main.c		\
	debug.c		\
	vector.c	\
	lex.c		\
	parser.c	\
	codegen.c	\
    hash.c

all:
	gcc ${SRCS} ${CFLAGS} -o ${TARGET}

clean :
	rm -f ${TARGET} *.o build/tmp.S


