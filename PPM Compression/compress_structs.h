/*
 *      compress_structs.h
 *      by Cansu Birsen (cbirse01), Ethan Goldman (gethan01)
 *      october 24, 2023
 *      PPM Compression
 *
 *      Stores struct types that are passed between compress/decompress steps
 */

#ifndef COMPRESS_STRUCTS_INCLUDED
#define COMPRESS_STRUCTS_INCLUDED

/* Represents component video pixels (floats) */
typedef struct component_video {
        float y, pb, pr;
} *component_video;

/* Represents unscaled rgb pixels (floats) */
typedef struct Pnm_rgb_float {
        float red, green, blue;
} *Pnm_rgb_float;

/* Represents quantized pixel (signed and unsigned integers) */
typedef struct quantized_values {
        unsigned a, index_pb, index_pr;
        int b, c, d;
} *quantized_values;

#endif