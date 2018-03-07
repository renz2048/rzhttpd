all: rzhttpd

rzhttpd: rzhttpd.c
	gcc -g -W -Wall -lpthread -o rzhttpd rzhttpd.c

clean:
	rm rzhttpd
