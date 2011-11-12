CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -g

all: typescript2txt

typescript2txt: typescript2txt.o

clean:
	-rm -f *.o typescript2txt 

.PHONY: all clean