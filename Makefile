CFLAGS=-std=gnu99 -Wall -pedantic

hex: game.c
	gcc $(CFLAGS) game.c -o hex