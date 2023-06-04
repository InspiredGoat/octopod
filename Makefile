SOURCES = $(wildcard *.c)
OBJ = $(addprefix bin/, $(addsuffix .o, $(basename $(SOURCES))))
TESTS_SRC = $(wildcard tests/test_*.c)
TESTS_BIN = $(addprefix bin/, $(addsuffix .out, $(basename $(TESTS_SRC))))

# Test flags
TESTS_FLAGS = -lm -g3 -lcmocka

# Debug
CFLAGS = -lm -g3
# Release
# CFLAGS = -O2

all: binfolder bin/library.o tests

again: clean all

bin/library.o: $(OBJ)
	ld -r $(OBJ) -o $@

bin/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@


run_tests: $(TESTS_BIN)
	echo running $<
	./$<

tests: $(TESTS_BIN)

bin/tests/%.out: tests/%.c
	$(CC) -L bin -l:library.o $(TESTS_FLAGS) $< -o $@

binfolder:
	mkdir -p bin
	mkdir -p bin/tests

clean:
	rm -rf bin/*
	mkdir -p bin
	mkdir -p bin/tests

