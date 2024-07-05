/*
 *      final_bitpack.c
 *      by Cansu Birsen (cbirse01), Ethan Goldman (gethan01)
 *      october 24, 2023
 *      PPM Compression
 *
 *      Implemenation for the final_bitpack.c file. Functions in this file are
 *      used to convert between 32 bit bitpacks and quantized values
 */

/* CS40 Libraries */
#include <bitpack.h>
#include <a2plain.h>

/* Hanson Libraries */
#include <assert.h>

/* Custom .h files */
#include "final_bitpack.h"
#include "compress_structs.h"

/* Width (in bits) of a subword in bitpack */
const unsigned A_WIDTH = 9;

/* Lowest significant bit of a subword in bitpack */
const unsigned A_LSB = 23;

/* Width (in bits) of b subword in bitpack */
const unsigned B_WIDTH = 5;

/* Lowest significant bit of b subword in bitpack */
const unsigned B_LSB = 18;

/* Width (in bits) of c subword in bitpack */
const unsigned C_WIDTH = 5;

/* Lowest significant bit of c subword in bitpack */
const unsigned C_LSB = 13;

/* Width (in bits) of d subword in bitpack */
const unsigned D_WIDTH = 5;

/* Lowest significant bit of d subword in bitpack */
const unsigned D_LSB = 8;

/* Width (in bits) of pb subword in bitpack */
const unsigned PB_WIDTH = 4;

/* Lowest significant bit of pb subword in bitpack */
const unsigned PB_LSB = 4;

/* Width (in bits) of pr subword in bitpack */
const unsigned PR_WIDTH = 4;

/* Lowest significant bit of pr subword in bitpack */
const unsigned PR_LSB = 0;

void compress_bitpack_apply(int col, int row, A2Methods_UArray2 array2, 
                            void *elem, void *cl);

void decompress_word_apply(int col, int row, A2Methods_UArray2 array2, 
                            void *elem, void *input);

void decompress_bitpack_apply(int col, int row, A2Methods_UArray2 array2, 
                            void *elem, void *code_words);


/***************************** compress_bitpack *******************************
 *
 * Compresses quantized values in each pixel to 32 bit code words and prints 
 * them to standard output
 *
 * Inputs:
 *              A2Methods_UArray2 quantized_pixels: UArray2 of pixels that are
 *                                                  in quantized values form
 * Return:
 *              none - only prints results to standard output
 * Expects:
 *              A2Methods_UArray2 quantized_pixels not to be null
 * Notes:
 *              will CRE if expectations fail
 *              the user accepts that  A2Methods_UArray2 quantized_pixels is 
 *                      freed
 *
 *****************************************************************************/
void compress_bitpack(A2Methods_UArray2 quantized_pixels)
{
        assert(quantized_pixels != NULL);
        
        /* Set the methods suite for the UArray2 */
        A2Methods_T methods = uarray2_methods_plain;
        assert(methods != NULL);

        /* Calculate the width and height values before merged 2x2 block */
        int width = methods->width(quantized_pixels) * 2;
        int height = methods->height(quantized_pixels) * 2;

        /* Print output header */
        printf("COMP40 Compressed image format 2\n%u %u\n", width, height);

        /* Compress quantized values to code words and free the UArray2 */
        methods->map_row_major(quantized_pixels, compress_bitpack_apply, NULL);
        methods->free(&quantized_pixels);
}

/*********************** compress_bitpack_apply *******************************
 *
 * compress_bitpack helper function - used when mapping through the 2D
 * UArray of quantized values to build the 32 bit code word
 *
 * Inputs:
 *              int col: The current column in the UArray2
 *              int row: The current row in the UArray2
 *              A2Methods_UArray2 array2: The UArray2 to traverse (contains
 *                                        quantized values in quantized_values
 *                                        structs)
 *              void: *elem: The element at the index [col, row] in the UArray2
 *              void: *cl: Closure variable
 * Return:
 *              none - the 32 bit code word is printed to stdout using putchar
 * Expects:
 *              array2 and elem are not null
 * Notes:
 *              will CRE if expectations fail
 *
 *****************************************************************************/
void compress_bitpack_apply(int col, int row, A2Methods_UArray2 array2, 
                            void *elem, void *cl)
{
        assert(array2 != NULL);
        assert(elem != NULL);

        (void) col;
        (void) row;
        (void) cl;
        
        uint64_t word = (uint64_t)0;
        quantized_values q = elem;

        /* Bitpack a */
        word = Bitpack_newu(word, A_WIDTH, A_LSB, q->a);

        /* Bitpack b */
        word = Bitpack_news(word, B_WIDTH, B_LSB, q->b);

        /* Bitpack c */
        word = Bitpack_news(word, C_WIDTH, C_LSB, q->c);

        /* Bitpack d */
        word = Bitpack_news(word, D_WIDTH, D_LSB, q->d);

        /* Bitpack pb */
        word = Bitpack_newu(word, PB_WIDTH, PB_LSB, q->index_pb);

        /* Bitpack pr */
        word = Bitpack_newu(word, PR_WIDTH, PR_LSB, q->index_pr);

        /* Print the bitpacked codeword to stdout using putchar 
           Codewords are written in big-endian order */
        for(int i = 3; i >= 0; i--) {
                uint64_t sub_word = Bitpack_getu(word, 8, i * 8);
                putchar(sub_word);
        }

}

/**************************** decompress_bitpack ******************************
 *
 * Decompresses 32 bit code words read in from the input stream to quantized 
 * values for each pixel in a UArray2
 *
 * Inputs:
 *              FILE *input: pointer to input stream to read the 32 bit code 
 *                           words from
 * Return:
 *              A2Methods_UArray2 quantized_pixs that holds quantized values
 *              a, b, c, d, index_pb, and index_pr for each 2x2 pixel block in 
 *              original image
 * Expects:
 *              FILE *input not to be null
 * Notes:
 *              will CRE if expectations fail
 *              it is the resposibility of the user to free the returned
 *                      quantized_pixs array and close the input stream
 *
 *****************************************************************************/
A2Methods_UArray2 decompress_bitpack(FILE *input)
{
        assert(input != NULL);
        
        /* Read in the header of the compressed file */
        unsigned height, width;
        int read = fscanf(input, "COMP40 Compressed image format 2\n%u %u", 
                          &width, &height);
        assert(read == 2);
        int c = getc(input);
        assert(c == '\n');

        /* Set the methods suite for the UArray2 */
        A2Methods_T methods = uarray2_methods_plain;
        assert(methods);

        /* Initialize and traverse through a UArray2 to store codewords */
        A2Methods_UArray2 codewords = methods->new(width / 2, height / 2, 
                                                   sizeof(uint64_t));
        methods->map_row_major(codewords, decompress_word_apply, input);

        /* Initialize a UArray2 to store quantized values */
        A2Methods_UArray2 quantized_pixs = methods->new(width / 2, height / 2, 
                                              sizeof(struct quantized_values));
        
        /* Traverse quantized_pixs to calculate quant vals from code words */
        methods->map_row_major(quantized_pixs, decompress_bitpack_apply, 
                               codewords);

        /* Free code words UArray2 */
        methods->free(&codewords);

        return quantized_pixs;

}

/*********************** decompress_word_apply ********************************
 *
 * decompress_bitpack helper function - used to read a single 32 bit 
 * codeword from a compressed file - used when reading through compressed file
 * to build the 2D UArray of codewords
 *
 * Inputs:
 *              int col: The current column in the UArray2
 *              int row: The current row in the UArray2
 *              A2Methods_UArray2 array2: The UArray2 to traverse (stores
 *                                        codewords)
 *              void: *elem: The element at the index [col, row] in the UArray2
 *              void: *input: The input file to read from
 * Return:
 *              none - the codewords are stored in array2
 * Expects:
 *              array2, elem, and input are not null
 * Notes:
 *              will CRE if expectations fail
 *
 *****************************************************************************/
void decompress_word_apply(int col, int row, A2Methods_UArray2 array2, 
                            void *elem, void *input) 
{
        assert(array2 != NULL);
        assert(elem != NULL);
        assert(input != NULL);
        
        (void) col;
        (void) row;
        
        FILE *fp = input;

        /* Read in the codeword in big-endian order using getc */
        uint64_t word = (uint64_t)0;
        for(int i = 3; i >= 0; i--) {
                int c = getc(fp);
                assert(c != EOF);
                word = Bitpack_newu(word, 8, i * 8, (uint64_t)c);
        }

        /* Store the codeword in the UArray2 */
        uint64_t *curr = elem;
        *curr = word;
}

/*********************** decompress_bitpack_apply *****************************
 *
 * decompress_bitpack helper function - used when mapping through the 2D
 * UArray of quantized values to calculate the values using the code_words 
 * UArray with the 32 bit code words
 *
 * Inputs:
 *              int col: The current column in the UArray2
 *              int row: The current row in the UArray2
 *              A2Methods_UArray2 array2: The UArray2 to traverse (contains
 *                                        quantized values in quantized_values
 *                                        structs)
 *              void: *elem: The element at the index [col, row] in the UArray2
 *              void: *code_words: The UArray2 holding 32 bit code words 
 * Return:
 *              none 
 * Expects:
 *              array2, elem, and code_words are not null
 * Notes:
 *              will CRE if expectations fail
 *
 *****************************************************************************/
void decompress_bitpack_apply(int col, int row, A2Methods_UArray2 array2, 
                            void *elem, void *code_words) 
{
        assert(array2 != NULL);
        assert(elem != NULL);
        assert(code_words != NULL);
        
        /* Set methods suite and cast void pointer code_words */
        A2Methods_T methods = uarray2_methods_plain;
        assert(methods);
        A2Methods_UArray2 codewords = code_words;

        /* Access to the codeword corresponding to the current pixel */
        uint64_t *word = methods->at(codewords, col, row);
        assert(word != NULL); 
        quantized_values q = elem;

        /* Unpack a */
        q->a = Bitpack_getu(*word, A_WIDTH, A_LSB);

        /* Unpack b */
        q->b = Bitpack_gets(*word, B_WIDTH, B_LSB);

        /* Unpack c */
        q->c = Bitpack_gets(*word, C_WIDTH, C_LSB);

        /* Unpack d */
        q->d = Bitpack_gets(*word, D_WIDTH, D_LSB);

        /* Unpack pb */
        q->index_pb = Bitpack_getu(*word, PB_WIDTH, PB_LSB);

        /* Unpack pr */
        q->index_pr = Bitpack_getu(*word, PR_WIDTH, PR_LSB);
}