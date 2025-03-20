CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -fsanitize=address,bounds,undefined,leak
strings-utf16: strings-utf16.c
	$(CC) $(CFLAGS) -DBUF_SZ=$(buf_size) $< -o $@

debug: strings-utf16.c
	$(CC) $(CFLAGS) -g -O0 -DBUF_SIZ=$(buf_size) $< -o $@
