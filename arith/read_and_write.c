/*
 *      read_and_write.h
 *      by Cansu Birsen (cbirse01), Ethan Goldman (gethan01)
 *      october 23, 2023
 *      PPM Compression
 *
 *      Implementation for the read_and_write.h file. Functions in this file
 *      are used to convert between ppm files and Pnm_ppm structs
 */

/* CS40 Libraries */
#include <a2blocked.h>

/* Hanson Libraries */
#include <assert.h>

/* Custom .h files */
#include "read_and_write.h"


/******************************* read_ppm *********************************
 *
 * Reads in the image information from th input file and initializes a Pnm_ppm
 * struct that stores the image information in a A2Methods_UArray2.
 *
 * Inputs:
 *              FILE *input: pointer to the file to read the image info from
 * Return:
 *              the new Pnm_ppm struct
 * Expects:
 *              input file pointer not to be null
 * Notes:
 *              will CRE if expectations fail
 *              it is the resposibility of the user to free the Pnm_ppm struct
 *                      using the Pnm_ppmfree function
 *
 *****************************************************************************/
Pnm_ppm read_ppm(FILE *input) 
{

        /* Assert that the file pointer is not null */
        assert(input != NULL);

        /* Set method types for Pnm_ppm */
        A2Methods_T methods = uarray2_methods_blocked;
        assert(methods != NULL);
        A2Methods_mapfun *map = methods->map_default; 
        assert(map != NULL);

        /* Read in files to Pnm_ppm structs */
        Pnm_ppm image = Pnm_ppmread(input, methods);

        return image;
}

/******************************* write_ppm *********************************
 *
 * Writes the data from the Pnm_ppm struct image to stdout
 *
 * Inputs:
 *              Pnm_ppm image: Pnm_ppm struct with data that we are are
 *              printing to stdout
 * Return:
 *              none - all output is sent to stdout
 * Expects:
 *              input Pnm_ppm struct ptr not to be null
 * Notes:
 *              will CRE if expectations fail
 *              the user accepts that the Pnm_ppm struct image is freed in
 *                      this function
 *
 *****************************************************************************/
void write_ppm(Pnm_ppm image) 
{
        /* Assert that the pnm_ppm struct is not null */
        assert(image != NULL);
        
        /* print out the resulting image and free the Pnm_ppm instance */
        Pnm_ppmwrite(stdout, image);
        Pnm_ppmfree(&image);
}