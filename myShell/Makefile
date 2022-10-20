CC = clang-12
DEBUG_FLAGS = -Wall -Wextra -Wpedantic -fsanitize=address,leak
I_FLAG = -I ./src/Include

all: clear
	$(CC) myShell.c src/parser.c src/cmd_vector.c src/cmd.c -o shell -g $(I_FLAG)

clear:
	rm -rf *.o
