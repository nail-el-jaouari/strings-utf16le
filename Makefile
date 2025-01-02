CC = gcc
strings-utf16: strings-utf16.c
	$(CC) -Wall -Wextra -Wpedantic $< -o $@

debug: strings-utf16.c
	$(CC) -Wall -Wextra -Wpedantic -ggdb -O0 $< -o $@
