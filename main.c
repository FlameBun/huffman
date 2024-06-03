#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "huff.h"

int valid_options(int argc, char **argv);
void print_menu(void);
int is_valid_block_size(const char* str);

int main(int argc, char** argv) {
    int ret;

    // Check if argument is valid
    if (valid_options(argc, argv) == -1) {
        fprintf(stderr, "Invalid options.\n");
        print_menu();
        return EXIT_FAILURE;
    }

    // Check if bit 0 is set
    if (global_options & 1) {
        print_menu();
        return EXIT_SUCCESS;
    }

    // Check if bit 1 or 2 is set
    if (global_options & 0x2) {
        ret = compress();
        if (ret == -1)
            fprintf(stderr, "Compression error\n");
    } else if (global_options & 0x4) {
        ret = decompress();
        if (ret == -1)
            fprintf(stderr, "Decompression error\n");
    }

    if (ret == 0)
        return EXIT_SUCCESS;
    else
        return EXIT_FAILURE;
}

/**
 * valid_options() check to see if the specified command line arguments are
 * valid or not. If they are valid, the bits in the global_options variable
 * will be set to indicate what action to take as well as other information.
 *
 * Return 0 if the specified command line arguments are valid. Otherwise,
 * return -1.
 */
int valid_options(int argc, char **argv) {
    // Failure if no flags are provided
    if (argc == 1)
        return -1;

    // Success if -h is the first option on the command line
    if (strcmp(argv[1], "-h") == 0) {
        global_options |= 1;
        return 0;
    }

    // Success if -d is the first and only option on the command line
    if (argc == 2 && strcmp(argv[1], "-d") == 0) {
        global_options |= 0xffff0004;
        return 0;
    }

    if (strcmp(argv[1], "-c") == 0) {
        // Success if -c is the first and only option on the command line
        if (argc == 2) {
            global_options |= 0xffff0002;
            return 0;
        }

        // Success if command line format is "./huff -c -b BLOCKSIZE", where
        // BLOCKSIZE is in the valid range
        // [MIN_BLOCK_SIZE (1024), MAX_BLOCK_SIZE (65536)]
        if (argc == 4 && strcmp(argv[2], "-b") == 0 &&
            is_valid_block_size(argv[3])) {
            // Store the specified block size minus one in the 16 most
            // significant bits of global_options
            int block_size = atoi(argv[3]);
            global_options |= ((block_size - 1) << 16) | 0x2;
            return 0;
        }
    }

    return -1;
}

/**
 * Print the menu.
 */
void print_menu(void) {
    fprintf(stderr,
        "Menu:\n"
        "./huff [-h] [-c|-d] [-b BLOCKSIZE]\n"
        "-h   Help: Display this help menu.\n"
        "-c   Compress: Read the original data and output compressed data.\n"
        "-b   Block Size: (Use only if -c is specified). Specify the block size in bytes ([1024, 65536]).\n"
        "-d   Decompress: Read the compressed data and output original data.\n");
}

/**
 * Check if the string is a valid block size. If the string is a whole number
 * and the whole number is in the valid range
 * [MIN_BLOCK_SIZE (1024), MAX_BLOCK_SIZE (65536)], return 1. Otherwise,
 * return 0.
*/
int is_valid_block_size(const char* str) {
    int num_digits = 0;
    const char* ptr = str;

    // Scan past any leading zeros
    while (*ptr != '\0' && *ptr == '0')
        ptr++;

    while (*ptr != '\0') {
        // Return false if the character is not a digit
        if (!(*ptr >= '0' && *ptr <= '9'))
            return 0;

        ptr++;
        num_digits++;

        // Return false if there are too many digits (excluding leading zeroes)
        if (num_digits >= 6)
            return 0;
    }

    // Return true if the block size specified by str is valid
    int block_size = atoi(str);
    if (block_size >= MIN_BLOCK_SIZE && block_size <= MAX_BLOCK_SIZE)
        return 1;

    return 0;
}
