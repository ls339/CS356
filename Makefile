CC=gcc
CFLAGS=-H

tcpserver:
	$(CC) $(CFLAGS) -o tcpserver tcpserver.c
