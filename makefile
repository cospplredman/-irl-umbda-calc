
.PHONY: clean all format

SRC=$(wildcard src_c/*.c)
HEADER=$(wildcard src_c/*.h)
OUT=out
OBJ=$(addprefix $(OUT)/, $(notdir $(SRC:.c=.o)))

CC ?= gcc
O4_FLAGS='-march=native' '-mtune=native' '-O3' '-fsched-pressure' '-fmodulo-sched' '-fmodulo-sched-allow-regmoves' '-fgcse-sm' '-fgcse-las' '-fivopts' '-fgraphite-identity' '-ftree-vectorize' '-floop-nest-optimize' '-ftree-loop-im' '-ftree-loop-ivcanon' '-funroll-loops' '-fdevirtualize-speculatively'
CFLAGS=-std=c99 -O3 -Wall -Werror $(O4_FLAGS) -pedantic
#CFLAGS=-std=c99 -O0 -Wall -Werror -pedantic -g -fno-omit-frame-pointer

all: $(OUT)/lc

clean:
	rm -f $(OUT)/lc $(OUT)/lc.a $(OBJ) gmon.out perf.data perf.data.old

format:
	clang-format -i $(SRC) $(HEADER)

$(OUT)/%.o: src_c/%.c
	mkdir -p $(OUT)
	${CC} ${CFLAGS} -c $< -o $@

$(OUT)/lc: $(OUT)/lc.a $(OBJ)
	${CC} ${CFLAGS} $(OBJ) -o $@

$(OUT)/lc.a: $(OBJ)
	ar rcs $@ $^


