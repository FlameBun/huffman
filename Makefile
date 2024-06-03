FILES = main.c compression.c decompression.c
CFLAGS = -Wall -Wextra -Werror -fcommon

huff: $(FILES)
	$(CC) $(CFLAGS) -o $@ $(FILES)

clean:
	rm -f huff
