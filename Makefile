all: rzhttpd

rzhttpd: rzhttpd.c
	gcc -g -W -Wall -lpthread -o rzhttpd.bin rzhttpd.c

clean:
	rm rzhttpd.bin
