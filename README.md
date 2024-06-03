# Huffman Coding
My program features a command line interface (CLI) that allows a user to perform data compression and decompression. These operations are accomplished through Huffman coding. Assigning shorter bit sequences to input bytes that appear frequently and longer bit sequences to input bytes that appear rarely effectively compresses the input data.

## How to Get Started
Inside the `huffman` directory, run `make` to compile and link the .c files. This will create the executable `./huff` in the `huffman` directory. Afterwards, run `./huff -h`. The following instructions will appear on the terminal:
<pre>
Menu:
./huff [-h] [-c|-d] [-b BLOCKSIZE]
-h   Help: Display this help menu.
-c   Compress: Read the original data and output compressed data.
-b   Block Size: (Use only if -c is specified). Specify the block size in bytes ([1024, 65536]).
-d   Decompress: Read the compressed data and output original data.
</pre>

## How to Use
It is necessary to perform input `<` and output `>` redirection in order to compress and decompress the desired files. For instance,
<pre>
./huff -c < InputFile.txt > OutputFile
</pre>
In the command shown above, `InputFile.txt` is the file to be compressed and `OutputFile` is the name of the compressed file. `OutputFile` will be created if it does not exist or truncated if it does exist.

## Notes
* `-b` is meant to be paired with `-c`. Without specifying a block size via `-b`, the default block size is 65536.
* The smaller the block size, the weaker the compression. On the other hand, the higher the block size, the stronger the compression.
