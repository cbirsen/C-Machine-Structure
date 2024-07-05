/*
 *      merge_blocks.c
 *      by Cansu Birsen (cbirse01), Ethan Goldman (gethan01)
 *      october 23, 2023
 *      PPM Compression
 *
 *      Implementation for the merge_blocks.h file. Functions in this file are
 *      used to convert between component video values and quantized values
 */

/* Standard C Libraries*/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/* CS40 Libraries */
#include <arith40.h>
#include <a2blocked.h>
#include <a2plain.h>

/* Hanson Libraries */
#include <assert.h>

/* Custom .h Files */
#include "compress_structs.h"
#include "merge_blocks.h"


void compress_merge_apply(int col, int row, A2Methods_UArray2 array2b,  
                          void *elem, void *params);
void decompress_merge_apply(int col, int row, A2Methods_UArray2 array2,  
                          void *elem, void *params);

unsigned calc_a(float y1, float y2, float y3, float y4);
int calc_b(float y1, float y2, float y3, float y4);
int calc_c(float y1, float y2, float y3, float y4);
int calc_d(float y1, float y2, float y3, float y4);
unsigned calc_index_pb(float pb1, float pb2, float pb3, float pb4);
unsigned calc_index_pr(float pr1, float pr2, float pr3, float pr4);
float truncate_0_3(float num);
float truncate(float num, float min, float max);

float calc_y1(float a, float b, float c, float d);
float calc_y2(float a, float b, float c, float d);
float calc_y3(float a, float b, float c, float d);
float calc_y4(float a, float b, float c, float d);


/**********struct compress_parameters********
 * About: This struct hold the parameters A2Methods_UArray2 pixels and its
 *        A2Methods_T methodsB (used to store blocked methods) and methodsP
 *        (used to store plain methods). They are grouped together to allow
 *        them to be passed together into mapping functions during 
 *        compression steps
************************/
struct compress_parameters {
        A2Methods_UArray2 pixels;
        A2Methods_T methodsB;
        A2Methods_T methodsP;
};

/* Range for truncating b, c, d values */
static const float B_C_D_MAX = 0.3;

/* Range for truncating y values */
static const float Y_MAX = 1.0;
static const float Y_MIN = 0.0;

/* Range for truncating Pb, Pr */
static const float PB_PR_RANGE = 0.5;

/* Scale factor for a */
static const float A_SCALE_FACTOR = 511.0;

/* Scale factor for b, c, d */
static const float B_C_D_SCALE_FACTOR = 50.0;


/******************************* compress_merge *******************************
 *
 * Compresses 2x2 blocks of component video pixels into 1x1 quantized blocks
 *
 * Inputs:
 *              A2Methods_UArray2 component_pixels: UArray2 of pixels that are
 *                                                  in component video form
 * Return:
 *              A2Methods_UArray2 storing quantized values
 * Expects:
 *              A2Methods_UArray2 component_pixels not to be null
 * Notes:
 *              will CRE if expectations fail
 *              the user accepts that  A2Methods_UArray2 component_pixels is 
 *                      freed
 *              it is the resposibility of the user to free the 
 *                      returned quantized_pixels array
 *
 *****************************************************************************/
A2Methods_UArray2 compress_merge(A2Methods_UArray2 component_pixels) 
{
        assert(component_pixels != NULL);    
    
        /* Set up blocked and plain methods */
        A2Methods_T methodsB = uarray2_methods_blocked;
        assert(methodsB);
        A2Methods_T methodsP = uarray2_methods_plain;
        assert(methodsP);
        
        /* Initialize the new quantized_pixels UArray2 */
        int width = methodsB->width(component_pixels) / 2;
        int height = methodsB->height(component_pixels) / 2;
        A2Methods_UArray2 quantized_pixels = methodsP->new(width, height,
                                              sizeof(struct quantized_values));
        
        /* Traverse component video pixels and store them as quantized values
           in new array */
        struct compress_parameters params = {quantized_pixels, methodsB, 
                                             methodsP};
        methodsB->map_block_major(component_pixels, compress_merge_apply, 
                                  &params);

        /* Free component pixels */
        methodsB->free(&component_pixels);
                                    
        return quantized_pixels;

}


/*********************** compress_merge_apply *********************************
 *
 * compress_merge helper function - used when mapping through the 2D
 * UArray of pixels to merge Y, Pb, Pr floats in 2-by-2 blocks to a, b, c, d,
 * index_pb, and index_pr variables.
 *
 * Inputs:
 *              int col: The current column in the UArray2
 *              int row: The current row in the UArray2
 *              A2Methods_UArray2 array2b: The UArray2 to traverse (contains
 *                                         float Y, Pb, Pr values in
 *                                         component_video structs)
 *              void: *elem: The element at the index [col, row] in the UArray2
 *              void: *params: Struct containing the UArray2 that we are 
 *                             mapping into (contains a, b, c, d, index_pb, and
 *                             index_pr variables in quantized_values structs) 
 *                             and its associated method suites
 * Return:
 *              none - the UArray2s that we update are passed by reference
 * Expects:
 *              array2b, elem, and params are not null
 * Notes:
 *              will CRE if expectations fail
 *
 *****************************************************************************/
void compress_merge_apply(int col, int row, A2Methods_UArray2 array2b,  
                          void *elem, void *params)
{ 
        assert(array2b != NULL);
        assert(elem != NULL);
        assert(params != NULL);

        /* Check if the current elem is the top left pixel in a block */
        if (col % 2 == 0 && row % 2 == 0) {
                /* Load in compress_prm information */
                struct compress_parameters *compress_prm = params;
                A2Methods_UArray2 quant_pix = compress_prm->pixels;
                A2Methods_T methodsB = compress_prm->methodsB;
                A2Methods_T methodsP = compress_prm->methodsP;

                /* Access to other pixels in the same block as the current */
                component_video c1 = elem;
                struct component_video *c2 = methodsB->at(array2b, col + 1, 
                                                          row);
                struct component_video *c3 = methodsB->at(array2b, col, 
                                                          row + 1);
                struct component_video *c4 = methodsB->at(array2b, col + 1, 
                                                          row + 1);

                /* Access to the quantized pixel corresponding to the block */
                struct quantized_values *new_pixel = methodsP->at(quant_pix, 
                                                             col / 2, row / 2);
                
                /* Make sure y values are in the correct range */
                c1->y = truncate(c1->y, Y_MIN, Y_MAX);
                c2->y = truncate(c2->y, Y_MIN, Y_MAX);
                c3->y = truncate(c3->y, Y_MIN, Y_MAX);
                c4->y = truncate(c4->y, Y_MIN, Y_MAX);

                /* Make sure pb values are in the correct range */
                c1->pb = truncate(c1->pb, -PB_PR_RANGE, PB_PR_RANGE);
                c2->pb = truncate(c2->pb, -PB_PR_RANGE, PB_PR_RANGE);
                c3->pb = truncate(c3->pb, -PB_PR_RANGE, PB_PR_RANGE);
                c4->pb = truncate(c4->pb, -PB_PR_RANGE, PB_PR_RANGE);

                /* Make sure pr values are in the correct range */
                c1->pr = truncate(c1->pr, -PB_PR_RANGE, PB_PR_RANGE);
                c2->pr = truncate(c2->pr, -PB_PR_RANGE, PB_PR_RANGE);
                c3->pr = truncate(c3->pr, -PB_PR_RANGE, PB_PR_RANGE);
                c4->pr = truncate(c4->pr, -PB_PR_RANGE, PB_PR_RANGE);

                /* Calculate quantized values from y, pb, pr floats */
                new_pixel->a = calc_a(c1->y, c2->y, c3->y, c4->y);
                new_pixel->b = calc_b(c1->y, c2->y, c3->y, c4->y);
                new_pixel->c = calc_c(c1->y, c2->y, c3->y, c4->y);
                new_pixel->d = calc_d(c1->y, c2->y, c3->y, c4->y);
                new_pixel->index_pb = calc_index_pb(c1->pb, c2->pb, c3->pb, 
                                                    c4->pb);
                new_pixel->index_pr = calc_index_pr(c1->pr, c2->pr, c3->pr, 
                                                    c4->pr);
        }
}

/*********************************** calc_a **********************************
 *
 * Calcuates unsigned integer a value using unscaled float component video
 * value y
 *
 * Inputs:
 *              float y1: y value for the top left pixel in the block
 *              float y2: y value for the top right pixel in the block
 *              float y3: y value for the bottom left pixel in the block
 *              float y4: y value for the bottom right pixel in the block
 * Return:
 *              Scaled unsigned integer a variable in quantized_values struct
 * Expects:
 *              y values to be in the range 0 to 1 
 * Notes:
 *              will CRE if expectations fail
 *
 *****************************************************************************/
unsigned calc_a(float y1, float y2, float y3, float y4)
{
        assert(y1 >= Y_MIN && y1 <= Y_MAX);
        assert(y2 >= Y_MIN && y2 <= Y_MAX);
        assert(y3 >= Y_MIN && y3 <= Y_MAX);
        assert(y4 >= Y_MIN && y4 <= Y_MAX);
        float a_temp = (y4 + y3 + y2 + y1) / 4.0;
        return (unsigned)(a_temp * A_SCALE_FACTOR);
}

/*********************************** calc_b **********************************
 *
 * Calcuates integer b value using unscaled float component video value y
 *
 * Inputs:
 *              float y1: y value for the top left pixel in the block
 *              float y2: y value for the top right pixel in the block
 *              float y3: y value for the bottom left pixel in the block
 *              float y4: y value for the bottom right pixel in the block
 * Return:
 *              Scaled integer b variable in quantized_values struct
 * Expects:
 *              y values to be in the range 0 to 1 
 * Notes:
 *              will CRE if expectations fail
 *
 *****************************************************************************/
int calc_b(float y1, float y2, float y3, float y4)
{
        assert(y1 >= Y_MIN && y1 <= Y_MAX);
        assert(y2 >= Y_MIN && y2 <= Y_MAX);
        assert(y3 >= Y_MIN && y3 <= Y_MAX);
        assert(y4 >= Y_MIN && y4 <= Y_MAX);
        float b_temp = (y4 + y3 - y2 - y1) / 4.0;
        b_temp = truncate(b_temp, -B_C_D_MAX, B_C_D_MAX);
        return (int)(b_temp * B_C_D_SCALE_FACTOR);
}

/*********************************** calc_c **********************************
 *
 * Calcuates integer c value using unscaled float component video value y
 *
 * Inputs:
 *              float y1: y value for the top left pixel in the block
 *              float y2: y value for the top right pixel in the block
 *              float y3: y value for the bottom left pixel in the block
 *              float y4: y value for the bottom right pixel in the block
 * Return:
 *              Scaled integer c variable in quantized_values struct
 * Expects:
 *              y values to be in the range 0 to 1 
 * Notes:
 *              will CRE if expectations fail
 *
 *****************************************************************************/
int calc_c(float y1, float y2, float y3, float y4)
{
        assert(y1 >= Y_MIN && y1 <= Y_MAX);
        assert(y2 >= Y_MIN && y2 <= Y_MAX);
        assert(y3 >= Y_MIN && y3 <= Y_MAX);
        assert(y4 >= Y_MIN && y4 <= Y_MAX);
        float c_temp = (y4 - y3 + y2 - y1) / 4.0;
        c_temp = truncate(c_temp, -B_C_D_MAX, B_C_D_MAX);
        return (int)(c_temp * B_C_D_SCALE_FACTOR);
}

/*********************************** calc_d **********************************
 *
 * Calcuates integer d value using unscaled float component video value y
 *
 * Inputs:
 *              float y1: y value for the top left pixel in the block
 *              float y2: y value for the top right pixel in the block
 *              float y3: y value for the bottom left pixel in the block
 *              float y4: y value for the bottom right pixel in the block
 * Return:
 *              Scaled integer d variable in quantized_values struct
 * Expects:
 *              y values to be in the range 0 to 1 
 * Notes:
 *              will CRE if expectations fail
 *
 *****************************************************************************/
int calc_d(float y1, float y2, float y3, float y4)
{
        assert(y1 >= Y_MIN && y1 <= Y_MAX);
        assert(y2 >= Y_MIN && y2 <= Y_MAX);
        assert(y3 >= Y_MIN && y3 <= Y_MAX);
        assert(y4 >= Y_MIN && y4 <= Y_MAX);
        float d_temp = (y4 - y3 - y2 + y1) / 4.0;
        d_temp = truncate(d_temp, -B_C_D_MAX, B_C_D_MAX);
        return (int)(d_temp * B_C_D_SCALE_FACTOR);
}

/******************************* calc_index_pb ********************************
 *
 * Calcuates unsigned integer index_pb value using unscaled float component 
 * video value pb
 *
 * Inputs:
 *              float pb1: pb value for the top left pixel in the block
 *              float pb2: pb value for the top right pixel in the block
 *              float pb3: pb value for the bottom left pixel in the block
 *              float pb4: pb value for the bottom right pixel in the block
 * Return:
 *              Scaled unsigned integer index_pb variable in quantized_values 
 *              struct
 * Expects:
 *              pb values to be in the range -0.5 to 0.5
 * Notes:
 *              will CRE if expectations fail
 *
 *****************************************************************************/
unsigned calc_index_pb(float pb1, float pb2, float pb3, float pb4)
{
        assert(pb1 >= -PB_PR_RANGE && pb1 <= PB_PR_RANGE);
        assert(pb2 >= -PB_PR_RANGE && pb2 <= PB_PR_RANGE);
        assert(pb3 >= -PB_PR_RANGE && pb3 <= PB_PR_RANGE);
        assert(pb4 >= -PB_PR_RANGE && pb4 <= PB_PR_RANGE);
        float pb_avg = (pb1 + pb2 + pb3 + pb4) / 4.0;
        return Arith40_index_of_chroma(pb_avg);
}

/******************************* calc_index_pr ********************************
 *
 * Calcuates unsigned integer index_pr value using unscaled float component 
 * video value pr
 *
 * Inputs:
 *              float pr1: pr value for the top left pixel in the block
 *              float pr2: pr value for the top right pixel in the block
 *              float pr3: pr value for the bottom left pixel in the block
 *              float pr4: pr value for the bottom right pixel in the block
 * Return:
 *              Scaled unsigned integer index_pr variable in quantized_values 
 *              struct
 * Expects:
 *              pr values to be in the range -0.5 to 0.5
 * Notes:
 *              will CRE if expectations fail
 *
 *****************************************************************************/
unsigned calc_index_pr(float pr1, float pr2, float pr3, float pr4)
{
        assert(pr1 >= -PB_PR_RANGE && pr1 <= PB_PR_RANGE);
        assert(pr2 >= -PB_PR_RANGE && pr2 <= PB_PR_RANGE);
        assert(pr3 >= -PB_PR_RANGE && pr3 <= PB_PR_RANGE);
        assert(pr4 >= -PB_PR_RANGE && pr4 <= PB_PR_RANGE);
        float pr_avg = (pr1 + pr2 + pr3 + pr4) / 4.0;
        return Arith40_index_of_chroma(pr_avg);
}

/************************** decompress_merge_blocks ***************************
 *
 * Deompresses 1x1 quantized blocks into 2x2 blocks of component video pixels
 *
 * Inputs:
 *              A2Methods_UArray2 quant_pixs: UArray2 of pixels that are
 *                                                  in quantized form
 * Return:
 *              A2Methods_UArray2 storing component video values
 * Expects:
 *              A2Methods_UArray2 quant_pixs not to be null
 * Notes:
 *              will CRE if expectations fail
 *              the user accepts that A2Methods_UArray2 quant_pixs is 
 *                      freed
 *              it is the resposibility of the user to free the 
 *                      returned comp_pixels array
 *
 *****************************************************************************/
A2Methods_UArray2 decompress_merge_blocks(A2Methods_UArray2 quant_pixs) 
{
        assert(quant_pixs != NULL); 

        /* Set up blocked and plain methods */
        A2Methods_T methodsB = uarray2_methods_blocked;
        assert(methodsB);
        A2Methods_T methodsP = uarray2_methods_plain;
        assert(methodsP);
        
        /* Initialize the new component video pixels UArray2 */
        int width = methodsP->width(quant_pixs) * 2;
        int height = methodsP->height(quant_pixs) * 2;
        A2Methods_UArray2 comp_pixels = methodsB->new_with_blocksize(width,
                                    height, sizeof(struct component_video), 2);
        
        /* Traverse quantized values and store them as component video pixels
           in new array */
        struct compress_parameters params = {comp_pixels, methodsB, NULL};             
        methodsP->map_row_major(quant_pixs, decompress_merge_apply, 
                                  &params);

        /* Free quantized pixels */
        methodsP->free(&quant_pixs);
                
        return comp_pixels;
}

/********************** decompress_merge_apply ********************************
 *
 * decompress_merge_blocks helper function - used when mapping through the 2D
 * UArray of pixels to un-merge a, b, c, d, index_pb, and index_pr variables
 * in 1x1 blocks to Y, Pb, Pr floats in 2x2 blocks
 *
 * Inputs:
 *              int col: The current column in the UArray2
 *              int row: The current row in the UArray2
 *              A2Methods_UArray2 array2: The UArray2 to traverse (contains 
                                           a, b, c, d, index_pb, and index_pr
                                           variables in quantized_values structs)
 *              void: *elem: The element at the index [col, row] in the UArray2
 *              void: *params: Struct containing the UArray2 that we are 
 *                             mapping into (contains float Y, Pb, Pr values in
 *                             component_video structs) and its associated 
 *                             method suites
 * Return:
 *              none - the UArray2s that we update are passed by reference
 * Expects:
 *              array2, elem, and params are not null
 * Notes:
 *              will CRE if expectations fail
 *
 *****************************************************************************/
void decompress_merge_apply(int col, int row, A2Methods_UArray2 array2,  
                          void *elem, void *params)
{
        assert(array2 != NULL);
        assert(elem != NULL);
        assert(params != NULL);
        
        /* Load in compress_prm information */
        struct compress_parameters *compress_prm = params;
        A2Methods_UArray2 comp_pixels = compress_prm->pixels;
        A2Methods_T methodsB = compress_prm->methodsB;

        /* Dereference current elem*/
        quantized_values q1 = elem;

        /* Load in the four indicies in the component video array that the 
           quantized values should be mapped to */
        int comp_row = row * 2; 
        int comp_col = col * 2;
        struct component_video *c1 = methodsB->at(comp_pixels, comp_col, 
                                                  comp_row);
        struct component_video *c2 = methodsB->at(comp_pixels, comp_col + 1, 
                                                  comp_row);
        struct component_video *c3 = methodsB->at(comp_pixels, comp_col, 
                                                  comp_row + 1);
        struct component_video *c4 = methodsB->at(comp_pixels, comp_col + 1, 
                                                  comp_row + 1);

        /* Unscale the a, b, c, d float values*/
        float a_float = ((float) q1->a) / A_SCALE_FACTOR;
        float b_float = ((float) q1->b) / B_C_D_SCALE_FACTOR;
        float c_float = ((float) q1->c) / B_C_D_SCALE_FACTOR;
        float d_float = ((float) q1->d) / B_C_D_SCALE_FACTOR;
        
        /* Calculate the y values */
        c1->y = calc_y1(a_float, b_float, c_float, d_float);
        c2->y = calc_y2(a_float, b_float, c_float, d_float);
        c3->y = calc_y3(a_float, b_float, c_float, d_float);
        c4->y = calc_y4(a_float, b_float, c_float, d_float);

        /* Calculate the pb values */
        c1->pb = truncate(Arith40_chroma_of_index(q1->index_pb), 
                        -PB_PR_RANGE, PB_PR_RANGE);
        c2->pb = truncate(Arith40_chroma_of_index(q1->index_pb), 
                        -PB_PR_RANGE, PB_PR_RANGE);
        c3->pb = truncate(Arith40_chroma_of_index(q1->index_pb), 
                        -PB_PR_RANGE, PB_PR_RANGE);
        c4->pb = truncate(Arith40_chroma_of_index(q1->index_pb), 
                        -PB_PR_RANGE, PB_PR_RANGE);

        /* Calculate the pr values */
        c1->pr = truncate(Arith40_chroma_of_index(q1->index_pr), 
                        -PB_PR_RANGE, PB_PR_RANGE);
        c2->pr = truncate(Arith40_chroma_of_index(q1->index_pr), 
                        -PB_PR_RANGE, PB_PR_RANGE);
        c3->pr = truncate(Arith40_chroma_of_index(q1->index_pr), 
                        -PB_PR_RANGE, PB_PR_RANGE);
        c4->pr = truncate(Arith40_chroma_of_index(q1->index_pr), 
                        -PB_PR_RANGE, PB_PR_RANGE);
}

/******************************* calc_y1 ********************************
 *
 * Calculates float Y value for the top left pixel of a 2-by-2 block using 
 * unscaled float versions of the variables from quantized_values struct
 *
 * Inputs:
 *              float a: unscaled float a value from quantized_values
 *              float b: unscaled float b value from quantized_values
 *              float c: unscaled float c value from quantized_values
 *              float d: unscaled float d value from quantized_values
 * Return:
 *              component video value Y in the range 0 to 1 for top left pixel
 *              in a 2-by-2 block
 * Expects:
 *              none
 * Notes:
 *              none
 *
 *****************************************************************************/
float calc_y1(float a, float b, float c, float d) {
        return truncate(a - b - c + d, Y_MIN, Y_MAX);
}

/******************************* calc_y2 ********************************
 *
 * Calculates float Y value for the top right pixel of a 2-by-2 block using 
 * unscaled float versions of the variables from quantized_values struct
 *
 * Inputs:
 *              float a: unscaled float a value from quantized_values
 *              float b: unscaled float b value from quantized_values
 *              float c: unscaled float c value from quantized_values
 *              float d: unscaled float d value from quantized_values
 * Return:
 *              component video value Y in the range 0 to 1 for top right pixel
 *              in a 2-by-2 block
 * Expects:
 *              none
 * Notes:
 *              none
 *
 *****************************************************************************/
float calc_y2(float a, float b, float c, float d) {
        return truncate(a - b + c - d, Y_MIN, Y_MAX);
}

/******************************* calc_y3 ********************************
 *
 * Calculates float Y value for the bottom left pixel of a 2-by-2 block using 
 * unscaled float versions of the variables from quantized_values struct
 *
 * Inputs:
 *              float a: unscaled float a value from quantized_values
 *              float b: unscaled float b value from quantized_values
 *              float c: unscaled float c value from quantized_values
 *              float d: unscaled float d value from quantized_values
 * Return:
 *              component video value Y in the range 0 to 1 for bottom left 
 *              pixel in a 2-by-2 block
 * Expects:
 *              none
 * Notes:
 *              none
 *
 *****************************************************************************/
float calc_y3(float a, float b, float c, float d) {
        return truncate(a + b - c - d, Y_MIN, Y_MAX);
}

/******************************* calc_y4 ********************************
 *
 * Calculates float Y value for the bottom right pixel of a 2-by-2 block using 
 * unscaled float versions of the variables from quantized_values struct
 *
 * Inputs:
 *              float a: unscaled float a value from quantized_values
 *              float b: unscaled float b value from quantized_values
 *              float c: unscaled float c value from quantized_values
 *              float d: unscaled float d value from quantized_values
 * Return:
 *              component video value Y in the range 0 to 1 for bottom right 
 *              pixel in a 2-by-2 block
 * Expects:
 *              none
 * Notes:
 *              none
 *
 *****************************************************************************/
float calc_y4(float a, float b, float c, float d) {
        return truncate(a + b + c + d, Y_MIN, Y_MAX);
}

/******************************* truncate ********************************
 *
 * Given a max and a min value, makes sure the number is within the max and min
 * boundaries by setting it to min if it is smaller than min and setting it to
 * max if it is greater than max
 *
 * Inputs:
 *              float num: value to be truncated
 *              float min: min value boundary for num
 *              float max: max value boundary for num
 * Return:
 *              truncated version of the value nums
 * Expects:
 *              none
 * Notes:
 *              none
 *
 *****************************************************************************/
float truncate(float num, float min, float max) 
{
        float final_num;

        if (num >= min && num <= max) {
                final_num = num;
        } else if (num > max) {
                final_num = max;
        } else {
                final_num = min;
        }
        
        return final_num;
}
