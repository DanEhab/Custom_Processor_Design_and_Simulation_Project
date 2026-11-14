CC ?= gcc
CFLAGS ?= -O2 -std=c11 -Wall -Wno-unused-function -Wno-unused-variable

# translate.c includes processor.c, so it is the only file to compile.
processor: translate.c processor.c
	$(CC) $(CFLAGS) translate.c -o processor

run: processor
	./processor instructions.txt

test: processor
	python3 tests/run_tests.py

clean:
	rm -f processor processor.exe

.PHONY: run test clean
