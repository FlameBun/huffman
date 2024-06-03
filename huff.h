#ifndef HUFF_H
#define HUFF_H

/**
 * Each possible byte value (0-255) from the input data represents a symbol.
 * MAX_SYMBOLS (257) is the maximum number of "symbols" represented in the
 * Huffman tree used for both compression and decompression. However, there is
 * a special symbol used to indicate the end of a block, and that special
 * symbol will be associated with the symbol value 256.
 */
#define MAX_SYMBOLS (256 + 1)

#define MIN_BLOCK_SIZE 1024 // Smallest possible block size
#define MAX_BLOCK_SIZE 65536 // Largest possible block size

/**
 * Bit 0 is 1: -h flag is specified.
 * Bit 1 is 1: -c flag is specified.
 * Bit 2 is 1: -d flag is specified.
 *
 * The block size minus one is logged in the 16 most significant bits of
 * global_options. The default block size is 65536. Therefore, by default, the
 * 16 most significant bits of global_options is 0xffff. If -b is specified,
 * then the specified block size minus one is logged in the 16 most significant
 * bits of global_options instead.
 */
int global_options;

// Huffman tree nodes
typedef struct node {
    struct node* left;
    struct node* right;
    struct node* parent;
    int weight;
    short symbol;
} NODE;

int num_nodes; // # of nodes currently in the Huffman tree

/**
 * current_block is a buffer to hold the symbols of the block currently being
 * compressed.
 */
unsigned char current_block[MAX_BLOCK_SIZE];

/**
 * nodes is the array used to store Huffman tree nodes.
 *
 * Since a binary tree with n leaves has exactly 2 * n - 1 nodes, considering
 * the fact that the number of unique symbols is the number of leaves in the
 * Huffman tree, more than 2 * MAX_SYMBOLS - 1 nodes for the tree is not
 * needed.
 */
NODE nodes[2 * MAX_SYMBOLS - 1];

/**
 * Akin to a hash map, node_for_symbol is the array used to map symbols to
 * Huffman tree nodes, or, more specifically, the leaves of the Huffman tree.
 * node_for_symbol is used in compressing blocks of input data.
 */
NODE* node_for_symbol[MAX_SYMBOLS];

// Compression functions
int compress(void);
int compress_block(void);
void output_description(void);

// Decompression functions
int decompress(void);
int decompress_block(void);
int reconstruct_huffman_tree(void);

#endif
