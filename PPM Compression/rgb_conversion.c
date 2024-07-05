/*
 *      rgb_conversion.h
 *      by Cansu Birsen (cbirse01), Ethan Goldman (gethan01)
 *      october 23, 2023
 *      PPM Compression
 *
 *      Implementation for the rgb_conversion.h file. Functions in this file 
 *      are used to convert between scaled rgb integers and unscaled rgb floats
 */

/* Standard C Libraries*/
#include <stdlib.h>

/* Hanson Libraries */
#include <assert.h>

/* Custom .h files */
#include "rgb_conversion.h"
#include "compress_structs.h"

/* Helper function declarations */
void compress_apply(int col, int row, A2Methods_UArray2 array2b, void *elem, 
                      void *image);
void find_max_apply(int col, int row, A2Methods_UArray2 array2b, void *elem, 
                      void *max);
void find_min_apply(int col, int row, A2Methods_UArray2 array2b, void *elem, 
                     void *min);
void decompress_apply(int col, int row, A2Methods_UArray2 array2b, void *elem, 
                      void *param);



/**********struct decompress_parameters********
 * About: This struct hold the parameters Pnm_ppm image structure, and a max 
 *        and a min value keeping track of the max and min float pixels values
 *        before scaling. This struct is used to pass information into the 
 *        decompress_apply function to scale RGB values considering the max and
 *        min.
************************/
struct decompress_parameters {
        Pnm_ppm image;
        float max;
        float min;
};

/* The denominator value for the scaled RGB values */
static const float RGB_MAX = 255.0;

/************************** compress_rgb_conversion ***************************
 *
 * Updates the A2Methods_UArray2 that the Pnm_ppm struct holds so that the RGB 
 * values are floats instead of scaled integers. Also trims the 
 * A2Methods_UArray2 by a column or row if the width or height is an odd number
 *
 * Inputs:
 *              Pnm_ppm image: struct that holds the A2Methods_UArray2 storing 
 *                             RGB scaled integer values, A2Methods_T suite and
 *                             other information regarding the image.
 * Return:
 *              none - only the Pnm_ppm image is updated
 * Expects:
 *              Pnm_ppm image not to be null
 * Notes:
 *              will CRE if expectations fail
 *              it is the resposibility of the user to free the Pnm_ppm struct
 *                      they input using the Pnm_ppmfree function
 *
 *****************************************************************************/
void compress_rgb_conversion(Pnm_ppm image) 
{
        assert(image != NULL);
        
        /* Access to the pixel array and width and height information */
        A2Methods_UArray2 pixels_int = image->pixels;
        assert(pixels_int != NULL);
        int new_width = image->width;
        int new_height = image->height;

        /* If width or height is odd, update new width/height values to trim */
        if (image->width % 2 != 0) {
                new_width--;
        }
        if (image->height % 2 != 0) {
                new_height--;
        }

        /* Initialize new A2Methods_UArray2 to hold RGB floats */
        A2Methods_UArray2 pixels_float = image->methods->new(new_width, 
                                    new_height, sizeof(struct Pnm_rgb_float));
        assert(pixels_float != NULL);

        /* Update image members so they hold the new float pixel info */
        image->pixels = pixels_float;
        image->width = new_width;
        image->height = new_height;

        /* Fill pixels_float cells by using the RGB values from pixels_int */
        image->methods->map_block_major(pixels_int, compress_apply, image);

        /* Free the array that held RGB scaled integer info */
        image->methods->free(&pixels_int);
}

/*************************** compress_apply ***********************************
 *
 * compress_rgb_conversion helper function - used when mapping through the 2D
 * UArray of pixels to convert from scaled integer rgb values to unscaled float
 * rgb values
 *
 * Inputs:
 *              int col: The current column in the UArray2
 *              int row: The current row in the UArray2
 *              A2Methods_UArray2 array2b: The UArray2 to traverse (contains
 *                                         scaled integer rgb values in Pnm_rgb
 *                                         structs)
 *              void: *elem: The element at the index [col, row] in the UArray2
 *              void: *image: The UArray2 that we are mapping into (contains
 *                            unscaled float rgb values in Pnm_rgb_float 
 *                            structs)
 * Return:
 *              none - the UArray2s that we update are passed by reference
 * Expects:
 *              array2b, elem, and image are not null
 * Notes:
 *              will CRE if expectations fail
 *
 *****************************************************************************/
void compress_apply(int col, int row, A2Methods_UArray2 array2b, void *elem, 
                      void *image)
{
        assert(array2b != NULL);
        assert(elem != NULL);
        assert(image != NULL);
        
        /* Load in info about the UArray2 we are mapping to */
        Pnm_ppm new_image = image;
        A2Methods_UArray2 pixels_float = new_image->pixels;
        float d = new_image->denominator;
        int width = new_image->width;
        int height = new_image->height;

        /* Cast currect elem void pointer */
        Pnm_rgb curr_pix = elem;

        /* Map rgb values from scaled integers to unscaled floats */
        if (col < width && row < height) {
                Pnm_rgb_float new_pixel = new_image->methods->at(pixels_float,
                                                             col, row);
                new_pixel->red = ((float) curr_pix->red) / d;
                new_pixel->green = ((float) curr_pix->green) / d;
                new_pixel->blue = ((float) curr_pix->blue) / d;
        }
}

/************************* decompress_rgb_conversion **************************
 *
 * Updates the A2Methods_UArray2 that the Pnm_ppm struct holds so that the RGB 
 * values are scaled integers instead of floats. Makes sure all the RGB values 
 * are in the range 0-RGB_MAX
 *
 * Inputs:
 *              Pnm_ppm image: struct that holds the A2Methods_UArray2 storing 
 *                             RGB float values, A2Methods_T suite and
 *                             other information regarding the image.
 * Return:
 *              none - only the Pnm_ppm image is updated
 * Expects:
 *              Pnm_ppm image not to be null
 * Notes:
 *              will CRE if expectations fail
 *              it is the resposibility of the user to free the Pnm_ppm struct
 *                      they input using the Pnm_ppmfree function
 *
 *****************************************************************************/
void decompress_rgb_conversion(Pnm_ppm image) 
{
        assert(image != NULL);

        /* Access to the array holding float RGB values */
        A2Methods_UArray2 pixels_float = image->pixels;
        assert(pixels_float != NULL);
        
        /* Initialize new A2Methods_UArray2 to hold RGB scaled integers */
        A2Methods_UArray2 pixels_int = image->methods->new(image->width, 
                                       image->height, sizeof(struct Pnm_rgb));
        assert(pixels_int != NULL);
        image->pixels = pixels_int;

        /* Variables to find the min/max float RGB values */ 
        struct Pnm_rgb_float *first_p = image->methods->at(pixels_float, 0, 0);
        float *max = malloc(sizeof(float));
        float *min = malloc(sizeof(float));
        assert(min != NULL && max != NULL);
        *max = first_p->red;
        *min = first_p->red;

        /* Find min/max values to use for scaling float values */
        image->methods->map_block_major(pixels_float, find_max_apply, max);
        image->methods->map_block_major(pixels_float, find_min_apply, min);

        /* Traverse float values and scale and save integer values to image */
        image->denominator = RGB_MAX;
        struct decompress_parameters param = {image, *max, *min};
        image->methods->map_block_major(pixels_float, decompress_apply, 
                                        &param);

        /* Free the array holding integers values and min/max variables */
        image->methods->free(&pixels_float);
        free(max);
        free(min);
        max = NULL;
        min = NULL;
}

/****************************** find_min_apply ********************************
 *
 * Finds the minimum rgb value in a UArray2 containing Pnm_rgb_float structs
 *
 * Inputs:
 *              int col: The current column in the UArray2
 *              int row: The current row in the UArray2
 *              A2Methods_UArray2 array2b: The UArray2 to traverse 
 *                                         (contains Pnm_rgb_float structs)
 *              void: *elem: The element at the index [col, row] in the UArray2
 *              void: *min: The minimum value found so far
 * Return:
 *              none - the min value found is stored in min
 * Expects:
 *              array2b, elem, and min are not null
 * Notes:
 *              will CRE if expectations fail
 *
 *****************************************************************************/
void find_min_apply(int col, int row, A2Methods_UArray2 array2b, void *elem, 
                      void *min)
{
        assert(array2b != NULL);
        assert(elem != NULL);
        assert(min != NULL);

        (void) col;
        (void) row;
        
        Pnm_rgb_float curr_pix = elem;
        float *min_val = min;
        
        /* Compare rgb values for current pixel to min and if they are smaller
           update the value of min */
        if (curr_pix->red < *min_val) {
            *min_val = curr_pix->red;
        }
        if (curr_pix->green < *min_val) {
            *min_val = curr_pix->green;
        }
        if (curr_pix->blue < *min_val) {
            *min_val = curr_pix->blue;
        }
}

/****************************** find_max_apply ********************************
 *
 * Finds the maximum rgb value in a UArray2 containing Pnm_rgb_float structs
 *
 * Inputs:
 *              int col: The current column in the UArray2
 *              int row: The current row in the UArray2
 *              A2Methods_UArray2 array2b: The UArray2 to traverse 
 *                                         (contains Pnm_rgb_float structs)
 *              void: *elem: The element at the index [col, row] in the UArray2
 *              void: *max: The maximum value found so far
 * Return:
 *              none - the max value found is stored in max
 * Expects:
 *              array2b, elem, and max are not null
 * Notes:
 *              will CRE if expectations fail
 *
 *****************************************************************************/
void find_max_apply(int col, int row, A2Methods_UArray2 array2b, void *elem, 
                      void *max)
{
        assert(array2b != NULL);
        assert(elem != NULL);
        assert(max != NULL);

        (void) col;
        (void) row;
        
        Pnm_rgb_float curr_pix = elem;
        float *max_val = max;
        
        /* Compare rgb values for current pixel to max and if they are larger
           update the value of max */
        if (curr_pix->red > *max_val) {
            *max_val = curr_pix->red;
        }
        if (curr_pix->green > *max_val) {
            *max_val = curr_pix->green;
        }
        if (curr_pix->blue > *max_val) {
            *max_val = curr_pix->blue;
        }
}

/*************************** decompress_apply *********************************
 *
 * decompress_rgb_conversion helper function - used when mapping through the 2D
 * UArray of pixels to convert from unscaled float rgb values to scaled integer
 * rgb values
 *
 * Inputs:
 *              int col: The current column in the UArray2
 *              int row: The current row in the UArray2
 *              A2Methods_UArray2 array2b: The UArray2 to traverse (contains
 *                                         unscaled float rgb values)
 *              void: *elem: The element at the index [col, row] in the UArray2
 *              void: *param: The decompress_parameters struct that holds the
 *                            image with the UArray2 that we are mapping into 
 *                            (contains scaled integer rgb values), and max/min
 *                            unscaled float values in the UArray2 being 
 *                            traversed
 *               
 * Return:
 *              none - the UArray2s that we update are passed by reference
 * Expects:
 *              array2b, elem, and image are not null
 * Notes:
 *              will CRE if expectations fail
 *
 *****************************************************************************/
void decompress_apply(int col, int row, A2Methods_UArray2 array2b, void *elem, 
                      void *param)
{
        assert(array2b != NULL);
        assert(elem != NULL);
        assert(param != NULL);

        /* Load in info about the UArray2 we are mapping to and min/max vals */
        struct decompress_parameters *prm = param;
        Pnm_ppm new_image = prm->image;
        float min = prm->min;
        float scaling_factor = prm->max - min;
        A2Methods_UArray2 pixels_int = new_image->pixels;

        /* Cast currect elem void pointer */
        Pnm_rgb_float curr_pix = elem;
        
        /* Map rgb values from unscaled floats to scaled integers */
        Pnm_rgb new_pixel = new_image->methods->at(pixels_int, col, row);
        new_pixel->red = (unsigned)((curr_pix->red - min) * 
                                    (RGB_MAX / scaling_factor));
        new_pixel->green = (unsigned)((curr_pix->green - min) * 
                                      (RGB_MAX / scaling_factor));
        new_pixel->blue = (unsigned)((curr_pix->blue - min) * 
                                     (RGB_MAX / scaling_factor));
}
