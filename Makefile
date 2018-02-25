all: rzhttpd

rzhttpd: rzhttpd.c
	gcc -W -Wall -lpthread -o rzhttpd rzhttpd.c

clean:
	rm rzhttpd
