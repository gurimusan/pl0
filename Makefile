CC := gcc
CFLAGS = -ansi -Wall -std=c99

all: src/main.c src/codegen.c src/compile.c src/getSource.c src/table.c
	if [[ ! -d "bin" ]]; then mkdir "bin"; fi
	$(CC) $(CFLAGS) -o bin/pl0 src/main.c src/codegen.c src/compile.c src/getSource.c src/table.c
