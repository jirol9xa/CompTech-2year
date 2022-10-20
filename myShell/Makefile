CC = clang-12
DEBUG_FLAGS = -Wall -Wextra -Wpedantic -fsanitize=address,leak

all: clear
	$(CC) myShell.c parser.c cmd_vector.c cmd.c -o shell -g

clear:
	rm -rf *.o
