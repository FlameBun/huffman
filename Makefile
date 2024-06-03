FILES = src/main.c src/compression.c src/decompression.c
CFLAGS = -Wall -Wextra -Werror -fcommon

huff: $(FILES)
	$(CC) $(CFLAGS) -o $@ $(FILES)

clean:
	rm -f huff