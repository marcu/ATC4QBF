export CC = g++
GLIB_FLAG = `pkg-config --cflags --libs glib-2.0`
CFLAG =  $(GLIB_FLAG) -c -fPIC -I minisat/mtl/
DYNAMIC_FLAG_MACOS = -dynamiclib
DYNAMIC_FLAC_LINUX = -shared

CD	?= cd

.PHONY: clean all

all : 
	$(CD) minisat/core/ && $(MAKE)
	$(MAKE) antichain.dylib

no_option : 
	$(MAKE) mrproper
	$(CC) $(CFLAG) -o antichain.o antichain.c
	$(MAKE)

m :
	$(MAKE) mrproper
	$(CC) $(CFLAG) -D MINISAT -D STOPTEST -o antichain.o antichain.c
	$(MAKE)

s1 :
	$(MAKE) mrproper
	$(CC) $(CFLAG) -D SWAP1 -D STOPTEST -o antichain.o antichain.c
	$(MAKE)

s2 :
	$(MAKE) mrproper
	$(CC) $(CFLAG) -D SWAP2 -D STOPTEST -o antichain.o antichain.c
	$(MAKE)

o : 
	$(MAKE) mrproper
	$(CC) $(CFLAG) -D OPTIMA -D STOPTEST -o antichain.o antichain.c
	$(MAKE)

mo : 
	$(MAKE) mrproper
	$(CC) $(CFLAG) -D OPTIMA -D STOPTEST -D MINISAT -o antichain.o antichain.c
	$(MAKE)

ms1 :
	$(MAKE) mrproper
	$(CC) $(CFLAG) -D MINISAT -D SWAP1 -D STOPTEST -o antichain.o antichain.c
	$(MAKE)

os1 : 
	$(MAKE) mrproper
	$(CC) $(CFLAG) -D OPTIMA -D SWAP1 -D STOPTEST -o antichain.o antichain.c
	$(MAKE)

mos1 : 
	$(MAKE) mrproper
	$(CC) $(CFLAG) -D OPTIMA -D MINISAT -D STOPTEST -D SWAP1 -o antichain.o antichain.c
	$(MAKE)


ms2 :
	$(MAKE) mrproper
	$(CC) $(CFLAG) -D MINISAT -D SWAP2 -D STOPTEST -o antichain.o antichain.c
	$(MAKE)

os2 : 
	$(MAKE) mrproper
	$(CC) $(CFLAG) -D OPTIMA -D SWAP2 -D STOPTEST -o antichain.o antichain.c
	$(MAKE)

mos2 : 
	$(MAKE) mrproper
	$(CC) $(CFLAG) -D OPTIMA -D MINISAT -D STOPTEST -D SWAP2 -o antichain.o antichain.c
	$(MAKE)

antichain.o : antichain.c
	$(CC) $(CFLAG) $<

antichain.dylib : antichain.o minisat/core/Solver.o
	$(CC) $(DYNAMIC_FLAG_MACOS) -o $@ $(GLIB_FLAG) $^

antichain.so : antichain.o minisat/core/Solver.o
	$(CC) $(DYNAMIC_FLAC_LINUX) -o $@ $(GLIB_FLAG) $^

clean :
	$(CD) minisat/core && $(MAKE) clean
	$(RM) $(wildcard *~ *.o *.exe *.c~)

mrproper : clean
	$(RM) antichain.dylib
	$(RM) antichain.so