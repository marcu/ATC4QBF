MTL = ../mtl

all : Solver.o

Solver.o : Solver.c
	$(CC) -I $(MTL) -c -fPIC $<

clean :
	$(RM) $(wildcard *~ *.exe *.c~ *.o)