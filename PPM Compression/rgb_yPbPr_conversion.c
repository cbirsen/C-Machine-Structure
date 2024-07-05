/*
 *      rgb_yPbPr_conversion.h
 *      by Cansu Birsen (cbirse01), Ethan Goldman (gethan01)
 *      october 23, 2023
 *      PPM Compression
 *
 *      Implementation for the rgb_yPbPr_conversion.h file. Functions in this 
 *      file are used to convert between unscaled rgb floats and Y, Pb, Pr 
 *      floats
 */

/* Standard C Libraries*/
#include <stdlib.h>

/* CS40 Libraries */
#include <a2blocked.h>
#include <a2plain.h>
#include <a2methods.h>

/* Hanson Libraries */
#include <assert.h>

/* Custom .h files */
#include "rgb_yPbPr_conversion.h"
#include "compress_structs.h"

void compress_yPbPr_apply(int col, int row, A2Methods_UArray2 array2b,  
                          void *elem, void *component_pixels);
float calc_y(float r, float g, float b);
float calc_pb(float r, float g, float b);
float calc_pr(float r, float g, float b);

void decompress_yPbPr_apply(int col, int row, A2Methods_UArray2 array2b,  
                          void *elem, void *component_pixels);
float calc_r(float y, float pb, float pr);
float calc_bl(float y, float pb, float pr);
float calc_g(float y, float pb, float pr);


/**********struct compress_parameters********
 * About: This struct hold the parameters A2Methods_UArray2 pixels and its
 *        A2Methods_T methods. They are grouped together to allow them to
 *        be passed together into mapping functions during compression steps
************************/
struct compress_parameters {
        A2Methods_UArray2 pixels;
        A2Methods_T methods;
};

/* Range for truncating y values */
static const float Y_MAX = 1.0;
static const float Y_MIN = 0.0;

/* Range for truncating Pb, Pr */
static const float PB_PR_RANGE = 0.5;

/* Range for truncating rgb values */
static const float RGB_UNSCALED_MAX = 1.0;
static const float RGB_UNSCALED_MIN = 0.0;

/************************* compress_yPbPr_conversion **************************
 *
 * Uses the A2Methods_UArray2 that the Pnm_ppm struct holds to extract RGB 
 * float values and calculate component video floats Y, Pb, and Pr. Returns the
 * newly calculated values in a A2Methods_UArray2 with 2 by 2 blocks. 
 *
 * Inputs:
 *              Pnm_ppm image: struct that holds the A2Methods_UArray2 storing 
 *                             RGB unscaled float values, A2Methods_T suite and
 *                             other information regarding the image.
 * Return:
 *              A2Methods_UArray2 component_pixels that store Y, Pb, Pr values
 * Expects:
 *              Pnm_ppm image not to be null
 * Notes:
 *              will CRE if expectations fail
 *              the user accepts that Pnm_ppm image is freed
 *              it is the resposibility of the user to free the 
 *                      returned component_pixels array
 *
 *****************************************************************************/
A2Methods_UArray2 compress_yPbPr_conversion(Pnm_ppm image) 
{
        assert(image != NULL);

        /* Load in image information */
        A2Methods_UArray2 pixels = image->pixels;
        int width = image->width;
        int height = image->height;

        /* Initialize method suite */
        A2Methods_T methods = uarray2_methods_blocked;
        assert(methods != NULL && pixels != NULL);

        /* Initialize 2-by-2 blocked array for component video variables */
        A2Methods_UArray2 component_pixels = methods->new_with_blocksize(width,
                                    height, sizeof(struct component_video), 2);
        assert(component_pixels != NULL);

        /* Traverse RGB values and store them as y/pb/pr values in new array */
        struct compress_parameters params = {component_pixels, methods};                    
        methods->map_block_major(pixels, compress_yPbPr_apply, &params);

        /* Free the image struct */
        Pnm_ppmfree(&image);

        return component_pixels;

}

/*********************** compress_yPbPr_apply *********************************
 *
 * compress_yPbPr_conversion helper function - used when mapping through the 2D
 * UArray of pixels to convert from unscaled rgb floats to Y, Pb, Pr floats
 *
 * Inputs:
 *              int col: The current column in the UArray2
 *              int row: The current row in the UArray2
 *              A2Methods_UArray2 array2b: The UArray2 to traverse (contains
 *                                         unscaled float rgb values in
 *                                         Pnm_rgb_float structs)
 *              void: *elem: The element at the index [col, row] in the UArray2
 *              void: *params: Struct containing the UArray2 that we are 
 *                             mapping into (contains unscaled float Y, Pb, Pr 
 *                             values in component_video structs) and its
 *                             associated methods suite
 * Return:
 *              none - the UArray2s that we update are passed by reference
 * Expects:
 *              array2b, elem, and params are not null
 * Notes:
 *              will CRE if expectations fail
 *
 *****************************************************************************/
void compress_yPbPr_apply(int col, int row, A2Methods_UArray2 array2b,  
                          void *elem, void *params)
{
        assert(array2b != NULL);
        assert(elem != NULL);
        assert(params != NULL);
        
        /* Dereferences the UArray2, its methods, and the current elem*/
        struct compress_parameters *compress_prm = params;
        Pnm_rgb_float curr_pix = elem;
        A2Methods_UArray2 comp_pix = compress_prm->pixels;
        A2Methods_T methods = compress_prm->methods;

        /* Calculates and stores the Y, Pb, Pr values */
        struct component_video *new_pixel = methods->at(comp_pix, col, row);
        new_pixel->y = calc_y(curr_pix->red, curr_pix->green, curr_pix->blue);
        new_pixel->pb = calc_pb(curr_pix->red, curr_pix->green, curr_pix->blue);
        new_pixel->pr = calc_pr(curr_pix->red, curr_pix->green, curr_pix->blue);

}

/*********************************** calc_y ***********************************
 *
 * Calcuates unscaled float Y using unscaled float rgb values
 *
 * Inputs:
 *              float r: red pixel value
 *              float g: green pixel value
 *              float b: blue pixel value
 * Return:
 *              Unscaled float representation of Y
 * Expects:
 *              r, g, b are in range 0 to 1
 * Notes:
 *              will CRE if expectations fail
 *
 *****************************************************************************/
float calc_y(float r, float g, float b)
{
        assert(r >= RGB_UNSCALED_MIN && r <= RGB_UNSCALED_MAX);
        assert(g >= RGB_UNSCALED_MIN && g <= RGB_UNSCALED_MAX);
        assert(b >= RGB_UNSCALED_MIN && b <= RGB_UNSCALED_MAX);
        return 0.299 * r + 0.587 * g + 0.114 * b;
}

/*********************************** calc_pr **********************************
 *
 * Calcuates unscaled float Pr using unscaled float rgb values
 *
 * Inputs:
 *              float r: red pixel value
 *              float g: green pixel value
 *              float b: blue pixel value
 * Return:
 *              Unscaled float representation of Pr
 * Expects:
 *              r, g, b are in range 0 to 1
 * Notes:
 *              will CRE if expectations fail
 *
 *****************************************************************************/
float calc_pr(float r, float g, float b)
{
        assert(r >= RGB_UNSCALED_MIN && r <= RGB_UNSCALED_MAX);
        assert(g >= RGB_UNSCALED_MIN && g <= RGB_UNSCALED_MAX);
        assert(b >= RGB_UNSCALED_MIN && b <= RGB_UNSCALED_MAX);
        return 0.5 * r - 0.418688 * g - 0.081312 * b;
}

/*********************************** calc_Pb **********************************
 *
 * Calcuates unscaled float Pb using unscaled float rgb values
 *
 * Inputs:
 *              float r: red pixel value
 *              float g: green pixel value
 *              float b: blue pixel value
 * Return:
 *              Unscaled float representation of Pb
 * Expects:
 *              r, g, b are in range 0 to 1
 * Notes:
 *              will CRE if expectations fail
 *
 *****************************************************************************/
float calc_pb(float r, float g, float b)
{
        assert(r >= RGB_UNSCALED_MIN && r <= RGB_UNSCALED_MAX);
        assert(g >= RGB_UNSCALED_MIN && g <= RGB_UNSCALED_MAX);
        assert(b >= RGB_UNSCALED_MIN && b <= RGB_UNSCALED_MAX);
        return -0.168736 * r - 0.331264 * g + 0.5 * b;
}

/************************ decompress_yPbPr_conversion *************************
 *
 * Converts the Y, Pb, Pr values in component_pixels to RGB float values and 
 * stores the calculated values in an array held by a newly initiliazed Pnm_ppm
 * struct. 
 *
 * Inputs:
 *              A2Methods_UArray2 component_pixels: array that holds  
 *                             component_video structs with Y, Pb, Pr values
 * Return:
 *              Pnm_ppm image struct that holds the UArray with RGB float 
 *              values
 * Expects:
 *              A2Methods_UArray2 component_pixels not to be null
 * Notes:
 *              will CRE if expectations fail
 *              the user accepts that component_pixels is freed
 *              it is the resposibility of the user to free the 
 *                      returned image struct
 *
 *****************************************************************************/
Pnm_ppm decompress_yPbPr_conversion(A2Methods_UArray2 component_pixels)
{
        assert(component_pixels != NULL);

        /* Initialize method suite and load in width/height info */
        A2Methods_T methods = uarray2_methods_blocked;
        assert(methods != NULL);
        int width = methods->width(component_pixels);
        int height = methods->height(component_pixels);

        /* Initialize the array for rgb floats */
        A2Methods_UArray2 rgb_pixels = methods->new(width, height,
                                            sizeof(struct Pnm_rgb_float));
        assert(rgb_pixels != NULL);
        
        /* Traverse y/pb/pr values and store them as RGB values in new array */
        struct compress_parameters params = {rgb_pixels, methods};                   
        methods->map_block_major(component_pixels, decompress_yPbPr_apply, 
                                 &params);
        
        /* Free the component video values array */
        methods->free(&component_pixels);

        /* Malloc a Pnm_ppm struct and store the rgb_pixels array in it */
        Pnm_ppm image = malloc(sizeof(struct Pnm_ppm));
        assert(image != NULL);
        image->width = width;
        image->height = height;
        image->denominator = -1;
        image->pixels = rgb_pixels;
        image->methods = methods;

        return image;
}

/*********************** decompress_yPbPr_apply *******************************
 *
 * decompress_yPbPr_conversion helper function - used when mapping through the
 * UArray2 of pixels to convert from Y, Pb, Pr floats to unscaled rgb floats
 * Inputs:
 *              int col: The current column in the UArray2
 *              int row: The current row in the UArray2
 *              A2Methods_UArray2 array2b: The UArray2 to traverse (contains 
                                           unscaled float Y, Pb, Pr values in
                                           component_video structs)
 *              void: *elem: The element at the index [col, row] in the UArray2
 *              void: *params: Struct containing the UArray2 that we are 
 *                             mapping into (contains unscaled float rgb values
 *                             in Pnm_rgb_float structs) and its
 *                             associated mapping functions
 * Return:
 *              none - the UArray2s that we update are passed by reference
 * Expects:
 *              array2b, elem, and params are not null
 * Notes:
 *              will CRE if expectations fail
 *
 *****************************************************************************/
void decompress_yPbPr_apply(int col, int row, A2Methods_UArray2 array2b,  
                          void *elem, void *params)
{
        assert(array2b != NULL);
        assert(elem != NULL);
        assert(params != NULL);
        
        /* Dereferences the UArray2, its methods, and the current elem*/
        struct compress_parameters *decompress_prm = params;
        struct component_video *curr_pix = elem;
        A2Methods_UArray2 rgb_pix = decompress_prm->pixels;
        A2Methods_T methods = decompress_prm->methods;

        /* Calculates and stores the rgb values */
        Pnm_rgb_float new_pixel = methods->at(rgb_pix, col, row);
        new_pixel->red = calc_r(curr_pix->y, curr_pix->pb, curr_pix->pr);
        new_pixel->green = calc_g(curr_pix->y, curr_pix->pb, curr_pix->pr);
        new_pixel->blue = calc_bl(curr_pix->y, curr_pix->pb, curr_pix->pr);

}

/*********************************** calc_r **********************************
 *
 * Calcuates unscaled float r value in RGB using unscaled float component video
 * values
 *
 * Inputs:
 *              float y: y component video pixel value
 *              float pb: pb component video pixel value
 *              float pr: pr component video pixel value
 * Return:
 *              Unscaled float representation of r in RGB
 * Expects:
 *              y to be in the range 0 to 1 and pb/pr to be in the range -0.5 
 *              to 0.5
 * Notes:
 *              will CRE if expectations fail
 *
 *****************************************************************************/
float calc_r(float y, float pb, float pr) {
        assert(y >= Y_MIN && y <= Y_MAX);
        assert(pb >= -PB_PR_RANGE && pb <= PB_PR_RANGE);
        assert(pr >= -PB_PR_RANGE && pr <= PB_PR_RANGE);
        return 1.0 * y + 0.0 * pb + 1.402 * pr;
}

/*********************************** calc_b **********************************
 *
 * Calcuates unscaled float b value in RGB using unscaled float component video
 * values
 *
 * Inputs:
 *              float y: y component video pixel value
 *              float pb: pb component video pixel value
 *              float pr: pr component video pixel value
 * Return:
 *              Unscaled float representation of b in RGB
 * Expects:
 *              y to be in the range 0 to 1 and pb/pr to be in the range -0.5 
 *              to 0.5
 * Notes:
 *              will CRE if expectations fail
 *
 *****************************************************************************/
float calc_bl(float y, float pb, float pr) {
        assert(y >= Y_MIN && y <= Y_MAX);
        assert(pb >= -PB_PR_RANGE && pb <= PB_PR_RANGE);
        assert(pr >= -PB_PR_RANGE && pr <= PB_PR_RANGE);
        return 1.0 * y + 1.772 * pb + 0.0 * pr;
}

/*********************************** calc_g **********************************
 *
 * Calcuates unscaled float g value in RGB using unscaled float component video
 * values
 *
 * Inputs:
 *              float y: y component video pixel value
 *              float pb: pb component video pixel value
 *              float pr: pr component video pixel value
 * Return:
 *              Unscaled float representation of g in RGB
 * Expects:
 *              y to be in the range 0 to 1 and pb/pr to be in the range -0.5 
 *              to 0.5
 * Notes:
 *              will CRE if expectations fail
 *
 *****************************************************************************/
float calc_g(float y, float pb, float pr) {
        assert(y >= Y_MIN && y <= Y_MAX);
        assert(pb >= -PB_PR_RANGE && pb <= PB_PR_RANGE);
        assert(pr >= -PB_PR_RANGE && pr <= PB_PR_RANGE);
        return 1.0 * y - 0.344136 * pb - 0.714136 * pr;
}