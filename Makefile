CC ?= gcc
CFLAGS  ?= -O0 -g -Wall
LDFLAGS += -li2c

tipd: tipd.o

clean:
	rm -f tipd *.o
