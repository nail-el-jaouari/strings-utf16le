CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -fsanitize=address,bounds,undefined,leak
LDFLAGS = -fsanitize=address,bounds,undefined,leak

strings-utf16.o: strings-utf16.c unicode-length-utf8.h vec-str.h
	$(CC) $(CFLAGS) -c -DBUF_SZ=$(buf_size) $< -o $@

unicode-length-utf8.o: unicode-length-utf8.c unicode-length-utf8.h
	$(CC) $(CFLAGS) -c $< -o $@

vec-str.o: vec-str.c vec-str.h
	$(CC) $(CFLAGS) -c $< -o $@

strings-utf16: strings-utf16.o unicode-length-utf8.o vec-str.o
	$(CC) $(LDFLAGS) $^ -o $@

debug: strings-utf16.c unicode-length-utf8.c vec-str.c
	$(CC) $(CFLAGS) -g -O0 -DBUF_SZ=$(buf_size) $^ -o $@

clean:
	rm -f *.o debug strings-utf16
