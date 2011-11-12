CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -g

all: typescript2txt

typescript2txt: typescript2txt.o

test_01_passed:
	./typescript2txt < test_01_input.txt > test_01_actual_output.txt
	diff -q test_01_input.txt test_01_actual_output.txt
	touch test_01_passed

test: test_01_passed

clean:
	-rm -f *.o typescript2txt 

.PHONY: all clean test