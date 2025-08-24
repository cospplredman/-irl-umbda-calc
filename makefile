
.PHONY: clean all format

SRC=src_c/main.c
OUT=out
OBJ=$(OUT)/main.o

CC=gcc
O4_FLAGS='-march=native' '-mtune=native' '-O3' '-fsched-pressure' '-fmodulo-sched' '-fmodulo-sched-allow-regmoves' '-fgcse-sm' '-fgcse-las' '-fivopts' '-fgraphite-identity' '-ftree-vectorize' '-floop-nest-optimize' '-ftree-loop-im' '-ftree-loop-ivcanon' '-funroll-loops' '-fdevirtualize-speculatively'
CFLAGS=-std=c99 -O3 -Wall -Werror ${O4_FLAGS}

all: $(OUT)/lc

clean:
	rm -f $(OUT)/lc $(OUT)/lc.a $(OBJ)

format:
	clang-format -i $(SRC)

$(OUT)/%.o: src_c/%.c
	mkdir -p $(OUT)
	${CC} ${CFLAGS} -c $< -o $@

$(OUT)/lc: $(OUT)/lc.a $(OBJ)
	${CC} ${CFLAGS} $(OBJ) -o $@

$(OUT)/lc.a: $(OBJ)
	ar rcs $@ $^


