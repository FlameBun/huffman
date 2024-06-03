#include <stdlib.h>
#include <stdio.h>
#include "huff.h"

void label_leaves(NODE* root);

/**
 * Read compressed data from standard input, decompress that data, and write
 * the decompressed data to standard output. Assume the input data is
 * constructed from compress().
 *
 * Return 0 if decompression succeeds without error. Otherwise, return -1.
 */
int decompress(void) {
    int ret;

    while (!feof(stdin)) {
        ret = decompress_block();

        // Return -1 if error with decompression
        if (ret == -1)
            return -1;

        // Reset num_nodes
        num_nodes = 0;

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
 * Read one block of compressed data from standard input, decompress that
 * data, and write the decompressed data to standard output.
 *
 * Return 0 if the block decompresses without error. Otherwise, return -1.
 */
int decompress_block(void) {
    int ret = reconstruct_huffman_tree(); // Reconstruct Huffman tree from description

    // Return 0 if EOF is encountered / if nothing left to decompress
    if (ret == 1)
        return 0;

    // Return -1 if error with decompression
    if (ret == -1)
        return -1;

    int buffer = fgetc(stdin); // Buffer to hold bytes
    if (buffer == EOF)
        return -1;
    int bit_num = 7; // Counter to track bit position
    int end_block = 0; // Indicator for whether end block is reached

    while (!end_block) {
        // Traverse Huffman tree until leaf is reached
        NODE* ptr = nodes;
        while (!(ptr->left == NULL && ptr->right == NULL)) {
            if (buffer & (1 << bit_num))
                ptr = ptr->right;
            else
                ptr = ptr->left;

            // If ptr is at internal node, update buffer/bit_num accordingly
            if (!(ptr->left == NULL && ptr->right == NULL)) {
                if (bit_num == 0) {
                    buffer = fgetc(stdin);

                    if (buffer == EOF)
                        return -1;

                    bit_num = 7;
                } else {
                    bit_num--;
                }
            } else {
                // If end block is reached, DO NOT call fgetc()
                if (ptr->symbol != 256) {
                    if (bit_num == 0) {
                        buffer = fgetc(stdin);

                        if (buffer == EOF)
                            return -1;

                        bit_num = 7;
                    } else {
                        bit_num--;
                    }
                }
            }
        }

        // Output symbol at leaf unless leaf is end block
        int symbol = ptr->symbol;
        if (symbol == 256) {
            end_block = 1;
        } else {
            fputc(symbol & 0xFF, stdout);
        }
    }

    // Return -1 if error indicator associated with stdout is set
    if (ferror(stdout))
        return -1;

    return 0;
}

/**
 * Read a description of a Huffman tree from standard input and reconstruct the
 * tree from the description.
 *
 * Return 0 if the tree is reconstructed without error or 1 if EOF is
 * encountered at the start of a block. Otherwise, return -1.
 */
int reconstruct_huffman_tree(void) {
    // Determine number of nodes from first two bytes read
    int byte = fgetc(stdin);
    if (byte == EOF) {
        // Return -1 if error indicator associated with stdin is set
        if (ferror(stdin))
            return -1;

        // Return 1 if there is nothing left to decompress
        return 1;
    }
    num_nodes = fgetc(stdin);
    if (num_nodes == EOF)
        return -1;
    num_nodes |= byte << 8;

    int buffer = fgetc(stdin); // Buffer to hold bytes
    if (buffer == EOF)
        return -1;
    int bit_num = 7; // Counter to track bit position
    int top = -1; // Top of the stack

    // Reconstruct Huffman tree from description
    for (int i = 0, j = num_nodes - 1; i < num_nodes; i++) {
        if (buffer & (1 << bit_num)) {
            // Pop two nodes and move to high-end of array
            NODE right = nodes[top];
            NODE left = nodes[top - 1];
            nodes[j] = right;
            nodes[j - 1] = left;
            top--;

            // Push the new parent node onto the stack
            (nodes + top)->left = nodes + j - 1;
            (nodes + top)->right = nodes + j;

            j -= 2;
        } else {
            // Push a new leaf node onto the stack
            top++;
            (nodes + top)->left = NULL;
            (nodes + top)->right = NULL;
        }

        if (bit_num == 0 && i != num_nodes - 1) {
            buffer = fgetc(stdin);

            if (buffer == EOF)
                return -1;

            bit_num = 7;
        } else {
            bit_num--;
        }
    }

    // Assign symbol values to leaves from left to right of tree
    label_leaves(nodes);
    if (feof(stdin) || ferror(stdin))
        return -1;

    return 0;
}

/**
 * Save the symbol values at the leaf nodes from the left to the right of the Huffman tree.
 */
void label_leaves(NODE* root) {
    int buffer;

    if (root->left != NULL)
        label_leaves(root->left);

    if (root->right != NULL)
        label_leaves(root->right);

    // Save symbol values to corresponding leaves
    if (root->left == NULL && root->right == NULL) {
        buffer = fgetc(stdin);

        if ((buffer & 0xFF) != 0xFF) {
            root->symbol = buffer & 0xFF;
        } else {
            buffer = fgetc(stdin);
            root->symbol = (buffer == 0) ? 256 : 255;
        }
    }
}
