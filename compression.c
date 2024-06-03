#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include "huff.h"

void output_description(void);
void postorder_seq(NODE* root, char* buffer, int* bit_num);
void output_symbols(NODE* root);
void set_parents(NODE* root);

/**
 * Read data from standard input, compress the data, and write the compressed
 * data to standard output.
 *
 * Return 0 if compressing succeeds without error. Otherwise, return -1.
 */
int compress(void) {
    int ret;

    while (!feof(stdin)) {
        ret = compress_block();

        // Return -1 if error with compression
        if (ret == -1)
            return -1;

        // Reset num_nodes
        num_nodes = 0;

        // Reset current_block array
        for (int i = 0; i < MAX_BLOCK_SIZE; i++)
            current_block[i] = 0;

        // Reset nodes array
        NODE empty_node = {.left = NULL, .right = NULL, .parent = NULL, .weight = 0, .symbol = 0};
        for (int i = 0; i < 2 * MAX_SYMBOLS - 1; i++)
            nodes[i] = empty_node;

        // Reset node_for_symbol array
        for (int i = 0; i < MAX_SYMBOLS; i++)
            node_for_symbol[i] = NULL;
    }

    fflush(stdout);

    return 0;
}

/**
 * Read one block of data from standard input, compress the data, and output
 * the compressed data to standard output.
 *
 * Typically, the specified block size is read. The only time compress_block()
 * reads less than the specified block size is when EOF is reached before being
 * able to read a full-sized block. The specified block size is logged in the
 * 16 most significant bits of the global_options variable.
 *
 * Return 0 if the block is compressed without error. Otherwise, return -1;
 */
int compress_block(void) {
    int block_size = ((global_options >> 16) & 0xffff) + 1;
    int num_symbols = 0; // Current number of symbols in current_block
    int num_leaves = 0; // Current number of leaves in nodes array

    // Read a block of input data; record the symbols in the current block and
    // the number of times a symbol occurs in the input
    for (int i = 0; i < block_size; i++) {
        int symbol_val = fgetc(stdin);

        if (symbol_val == EOF) {
            // Return -1 if error indicator associated with stdin is set
            if (ferror(stdin))
                return -1;

            // Return -1 if block of data is empty
            if (i == 0)
                return 0;

            // End of file is reached and block has data in it
            break;
        }

        // Read byte from standard input and insert into current_block
        current_block[num_symbols] = symbol_val;
        num_symbols++;

        // Add new byte or update frequency of byte in histogram
        if (node_for_symbol[symbol_val] == NULL) {
            node_for_symbol[symbol_val] = nodes + num_leaves;
            nodes[num_leaves].parent = NULL;
            nodes[num_leaves].left = NULL;
            nodes[num_leaves].right = NULL;
            nodes[num_leaves].weight = 1;
            nodes[num_leaves].symbol = symbol_val;
            num_leaves++;
        } else {
            node_for_symbol[symbol_val]->weight++;
        }
    }

    // Add end block "symbol"
    node_for_symbol[256] = nodes + num_leaves;
    nodes[num_leaves].parent = NULL;
    nodes[num_leaves].left = NULL;
    nodes[num_leaves].right = NULL;
    nodes[num_leaves].weight = 0;
    nodes[num_leaves].symbol = 256;
    num_leaves++;

    num_nodes = 2 * num_leaves - 1; // Total number of nodes in Huffman tree

    // Algorithm to build Huffman tree
    for (int low = num_leaves, high = num_nodes; low > 1; low--, high -= 2) {
        NODE *min_node_1 = NULL, *min_node_2 = NULL;
        int min_1 = INT_MAX, min_2 = INT_MAX;
        int min_index_1 = -1, min_index_2 = -1;

        // Select two minimum-weight nodes
        for (int i = 0; i < low; i++) {
            if ((nodes + i)->weight < min_1) {
                min_node_2 = min_node_1;
                min_2 = min_1;
                min_index_2 = min_index_1;
                min_node_1 = nodes + i;
                min_1 = (nodes + i)->weight;
                min_index_1 = i;
            } else if ((nodes + i)->weight < min_2) {
                min_node_2 = nodes + i;
                min_2 = (nodes + i)->weight;
                min_index_2 = i;
            }
        }

        // Move two selected minimum-weight nodes to high-end of array
        nodes[high - 1] = *min_node_2;
        nodes[high - 2] = *min_node_1;

        int min_index; // Leftmost position of the two minimum-weight nodes
        min_index = (min_index_1 < min_index_2) ? min_index_1 : min_index_2;

        // Construct parent node at nodes[minIndex]
        nodes[min_index].parent = NULL;
        nodes[min_index].left = nodes + high - 2;
        nodes[min_index].right = nodes + high - 1;
        nodes[min_index].weight = (nodes + high - 2)->weight + (nodes + high - 1)->weight;
        nodes[min_index].symbol = -1;

        // Shift low-end of array to the left to maintain contiguity
        for (int i = (min_index_1 > min_index_2) ? min_index_1 : min_index_2; i < low - 1; i++)
            nodes[i] = nodes[i + 1];
    }

    // Output the description of the Huffman tree
    output_description();

    // Set parent pointer for each non-root node in Huffman tree
    set_parents(nodes);

    // Install pointers to leaf nodes
    for (int i = 0; i < num_nodes; i++) {
        if ((nodes + i)->left == NULL && (nodes + i)->right == NULL)
            node_for_symbol[(nodes + i)->symbol] = nodes + i;
    }

    // Output encoded bit sequence
    char buffer = 0; // Buffer to use in outputting bit sequence
    int bit_num = 7; // Counter to track bit position in buffer
    for (int i = 0; i < num_symbols; i++) {
        // Acquire leaf corresponding to symbol at current_block[i]
        NODE* leaf = node_for_symbol[current_block[i]];
        NODE* child_ptr = leaf;
        NODE* parent_ptr = child_ptr->parent;

        // Store directions using "weight" field
        while (parent_ptr != NULL) {
            parent_ptr->weight = (parent_ptr->left == child_ptr) ? 0 : 1;
            parent_ptr = parent_ptr->parent;
            child_ptr = child_ptr->parent;
        }

        // Output code for the corresponding symbol
        NODE* ptr = nodes;
        while (ptr != leaf) {
            if (ptr->weight == 1) {
                buffer |= 1 << bit_num;
                ptr = ptr->right;
            } else {
                ptr = ptr->left;
            }

            if (bit_num == 0) {
                fputc(buffer & 0xFF, stdout);
                bit_num = 7;
                buffer = 0;
            } else {
                bit_num--;
            }
        }
    }

    // Output code for "end block" symbol
    NODE* leaf = node_for_symbol[256];
    NODE* child_ptr = leaf;
    NODE* parent_ptr = child_ptr->parent;
    while (parent_ptr != NULL) {
        parent_ptr->weight = (parent_ptr->left == child_ptr) ? 0 : 1;

        parent_ptr = parent_ptr->parent;
        child_ptr = child_ptr->parent;
    }
    NODE* ptr = nodes;
    while (ptr != leaf) {
        if (ptr->weight == 1) {
            buffer |= 1 << bit_num;
            ptr = ptr->right;
        } else {
            ptr = ptr->left;
        }

        if (bit_num == 0) {
            fputc(buffer & 0xFF, stdout);
            bit_num = 7;
            buffer = 0;
        } else {
            bit_num--;
        }
    }

    // Additional padding if encoded sequence length is not multiple of 8 bits
    if (bit_num != 7)
        fputc(buffer & 0xFF, stdout);

    // Return -1 if error indicator associated with stdout is set
    if (ferror(stdout))
        return -1;

    return 0;
}

/**
 * Output a description of the Huffman tree used to compress the current block
 * to standard output.
 *
 * The first two bytes of the description is the number of nodes. The following
 * set of byte(s) represents a postorder traversal of the Huffman tree, where 0
 * indicates a leaf node and 1 indicates a non-leaf node (additional padding of
 * zeroes may be necessary to include). The last set of bytes specify the
 * symbol values of the leaf nodes from the left of the tree to the right of
 * the tree.
 */
void output_description(void) {
    // Output number of nodes as a two-byte sequence
    fputc((num_nodes & 0xFF00) >> 8, stdout);
    fputc(num_nodes & 0xFF, stdout);

    // Output bit sequence through postorder traversal
    char buffer = 0; // Buffer to use in outputting bit sequence
    int bit_num = 7; // Counter to track bit position in buffer
    postorder_seq(nodes, &buffer, &bit_num);

    // Additional padding if sequence length is not multiple of 8 bits
    if (bit_num != 7)
        fputc(buffer & 0xFF, stdout);

    // Output the symbol values at the leaves from left to right
    output_symbols(nodes);
}

/**
 * Output a sequence of num_nodes bits through a postorder traversal of the
 * Huffman tree in which each 0 bit denotes a leaf and each 1 bit denotes an
 * internal node.
 */
void postorder_seq(NODE* root, char* buffer, int* bit_num) {
    if (root->left != NULL)
        postorder_seq(root->left, buffer, bit_num);

    if (root->right != NULL)
        postorder_seq(root->right, buffer, bit_num);

    // Set bit to 1 if internal node
    if (!(root->left == NULL && root->right == NULL))
        *buffer |= 1 << *bit_num;

    if (*bit_num == 0) {
        fputc(*buffer & 0xFF, stdout);
        *bit_num = 7;
        *buffer = 0;
    } else {
        (*bit_num)--;
    }
}

/**
 * Output symbol values at the leaves from left to right of the Huffman tree.
 */
void output_symbols(NODE* root) {
    if (root->left != NULL)
        output_symbols(root->left);

    if (root->right != NULL)
        output_symbols(root->right);

    // Output symbol value at leaf node
    if (root->left == NULL && root->right == NULL) {
        if (root->symbol == 256) {
            fputc(0xFF, stdout);
            fputc(0, stdout);
        } else if (root->symbol == 255) {
            fputc(0xFF, stdout);
            fputc(1, stdout);
        } else {
            fputc(root->symbol & 0xFF, stdout);
        }
    }
}

/**
 * Set the parent pointers for each non-root node in the Huffman tree.
 */
void set_parents(NODE* root) {
    if (root->left != NULL) {
        root->left->parent = root;
        set_parents(root->left);
    }

    if (root->right != NULL) {
        root->right->parent = root;
        set_parents(root->right);
    }
}
