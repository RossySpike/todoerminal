main:
	gcc src/main.c -o main.out -lncurses -lsqlite3 -O2 -flto -march=native
main-debug:
	gcc src/main.c -o main-debug.out -lncurses -lsqlite3 -g -Wall -Werror -fsanitize=address -ggdb -O0
debug:
	ASAN_OPTIONS=detect_leaks=1 ./main-debug.out
