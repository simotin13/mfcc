TARGET=mfcc

CFLAGS=-Wall -g

SRCS=			\
	ast.c		\
	codegen.c	\
	debug.c		\
	func.c		\
	hash.c		\
	lex.c		\
	main.c		\
	parser.c	\
	type.c		\
	vector.c

all:
	gcc ${CFLAGS} ${SRCS} -o ${TARGET}

clean :
	rm -f ${TARGET} *.o build/tmp.S
