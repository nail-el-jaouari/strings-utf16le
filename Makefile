CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -fsanitize=address,bounds,undefined,leak
strings-utf16: strings-utf16.c
	$(CC) $(CFLAGS) $< -o $@

debug: strings-utf16.c
	$(CC) $(CFLAGS) -g -O0 $< -o $@
