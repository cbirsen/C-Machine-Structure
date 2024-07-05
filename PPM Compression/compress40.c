/*
 *      compress40.c
 *      by Cansu Birsen (cbirse01), Ethan Goldman (gethan01)
 *      october 24, 2023
 *      PPM Compression
 *
 *      Implementation for the compress40.h file. Functions in this file are 
 *      used to compress a ppm image to a file approximately 3 time smaller
 *      than the original and to decompress compressed files
 */

/* CS40 Libraries */
#include <compress40.h>
#include <pnm.h>
#include <a2blocked.h>

/* Hanson Libraries */
#include <assert.h>

/* Custom .h files */
#include "read_and_write.h"
#include "rgb_conversion.h"
#include "rgb_yPbPr_conversion.h"
#include "merge_blocks.h"
#include "final_bitpack.h"


/***************************** compress40 *************************************
 *
 * Compresses a ppm image and writes output to stdout
 *
 * Inputs:      
 *              File *input: The ppm image to compress
 * Return:
 *              none - all output is printed to stdout
 * Expects:
 *              input is not NULL
 * Notes:
 *              will CRE if expectations fail
 *
 *****************************************************************************/
void compress40(FILE *input) 
{
        assert(input != NULL);

        Pnm_ppm image = read_ppm(input);
        compress_rgb_conversion(image);
        A2Methods_UArray2 pixels = compress_yPbPr_conversion(image);
        A2Methods_UArray2 quantized = compress_merge(pixels);
        compress_bitpack(quantized);
}

/***************************** decompress40 ***********************************
 *
 * Decompresses a compressed file back to ppm file format
 *
 * Inputs:      
 *              File *input: The file to decompress
 * Return:
 *              none - all output is printed to stdout
 * Expects:
 *              input is not NULL
 * Notes:
 *              will CRE if expectations fail
 *
 *****************************************************************************/
void decompress40(FILE *input) 
{
        assert(input != NULL);

        A2Methods_UArray2 quantized = decompress_bitpack(input);
        A2Methods_UArray2 pixels = decompress_merge_blocks(quantized);
        Pnm_ppm image = decompress_yPbPr_conversion(pixels);
        decompress_rgb_conversion(image); 
        write_ppm(image);
}
