/*
 *      final_bitpack.h
 *      by Cansu Birsen (cbirse01), Ethan Goldman (gethan01)
 *      october 24, 2023
 *      PPM Compression
 *
 *      Interface for the final_bitpack.c file. Functions in this file are used
 *      to convert between 32 bit bitpacks and quantized values
 */

#ifndef FINAL_BITPACK_INCLUDED
#define FINAL_BITPACK_INCLUDED

/* Standard C Libraries */
#include <stdio.h>

/* CS40 Libraries */
#include <a2methods.h>

extern void compress_bitpack(A2Methods_UArray2 quantized_pixels);
extern A2Methods_UArray2 decompress_bitpack(FILE *input);

#endif