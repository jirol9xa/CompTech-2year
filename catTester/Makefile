CC = clang++
SRC = CatTester.cpp
CFLAGS = -O3 -Wall -Wextra -Wpedantic -fsanitize=address,leak,undefined -g  

all: myCat.o tester.o
	$(CC) $(CFLAGS) tester.o -o test 
clear:
	rm -rf *.o

myCat.o: myCat2.cpp
	$(CC) $(CFLAGS) myCat2.cpp -o myCat
tester.o: CatTester.cpp
	$(CC) -c $(CFLAGS) CatTester.cpp -o tester.o
