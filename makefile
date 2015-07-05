all: zigzag.o me.o

zigzag.o: zigzag.c
	gcc zigzag.c -pthread -o zigzag.o
me.o: me.c
	gcc me.c -pthread -o me.o
