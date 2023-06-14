all:
		g++ -Isrc/Include -Lsrc/lib -o pureplayer pureplayer.c -lmingw32 -lSDL2main -lSDL2