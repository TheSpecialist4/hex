CFLAGS=-std=gnu99 -Wall -pedantic

bob: game.c
	gcc $(CFLAGS) game.c -o bob