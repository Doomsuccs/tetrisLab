all: client server
	gcc tetrisserver.c -o server -lncurses
	gcc tetrisclient.c -o client -lncurses
