/*
 *     ppmtrans.c
 *     by Doga Kilinc (dkilin01) & Cansu Birsen (cbirse01), October 10
 *     PPM Operations
 *
 *     About: This file accepts command line inputs to copy pixels from a given
 *     ppm image in the format stated by the user (or the default method) and 
 *     performs a 0, 90, 180, 270 degree rotation, horizontal/vertical flip, or
 *     transpose depending on what the user asks for (or defaults to 0 degree)
 *     rotation. The program prints the resulting image to the standard output
 *     in binary ppm format. If the user desires, they can also time the 
 *     rotation operation with "-time" command followed by the name of the file
 *     to output the timing information.
 *              
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "assert.h"
#include "a2methods.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "pnm.h"
#include "operations.h"

/******************
 * rotation operation without an explicit degree were assigned an integer value
 * for easier calculation and operation
 ************************/
#define flipHorizontal 1
#define flipVertical 2
#define transpose 3

#define SET_METHODS(METHODS, MAP, WHAT) do {                    \
        methods = (METHODS);                                    \
        assert(methods != NULL);                                \
        map = methods->MAP;                                     \
        if (map == NULL) {                                      \
                fprintf(stderr, "%s does not support "          \
                                WHAT "mapping\n",               \
                                argv[0]);                       \
                exit(1);                                        \
        }                                                       \
} while (false)

static void
usage(const char *progname)
{
        fprintf(stderr, "Usage: %s [-rotate <angle>] "
                        "[-{row,col,block}-major] [filename]\n",
                        progname);
        exit(1);
}

/**********main********
 * About: Expects command line argument for rotation, method for copying image
 * pixels, timing operations, and/or either an input file name or input from 
 * stdin. If no rotation is given, assumes 0 degree rotation, and if no method
 * for copying pixels is given, uses the default best mapping method.
 * Inputs:
 * int argc: number of given arguments to start the program
 * char *argv: an array that stores the arguments
 * Return: EXIT_SUCCESS if the program completes without any errors
 ************************/
int main(int argc, char *argv[]) 
{
        char *time_file_name = NULL;
        int   rotation       = 0;
        int   i;
        FILE *fp = NULL; 

        /* keep track of the filename if the input was given through a file */
        char *inputFile = NULL;

        /* default to UArray2 methods */
        A2Methods_T methods = uarray2_methods_plain; 
        assert(methods);

        /* default to best map */
        A2Methods_mapfun *map = methods->map_default; 
        assert(map);

        for (i = 1; i < argc; i++) {
                if (strcmp(argv[i], "-row-major") == 0) {
                        SET_METHODS(uarray2_methods_plain, map_row_major, 
                                    "row-major");
                } else if (strcmp(argv[i], "-col-major") == 0) {
                        SET_METHODS(uarray2_methods_plain, map_col_major, 
                                    "column-major");
                } else if (strcmp(argv[i], "-block-major") == 0) {
                        SET_METHODS(uarray2_methods_blocked, map_block_major,
                                    "block-major");
                } else if (strcmp(argv[i], "-rotate") == 0) {
                        if (!(i + 1 < argc)) {      /* no rotate value */
                                usage(argv[0]);
                        }
                        char *endptr;
                        rotation = strtol(argv[++i], &endptr, 10);
                        if (!(rotation == 0 || rotation == 90 ||
                            rotation == 180 || rotation == 270)) {
                                fprintf(stderr, 
                                        "Rotation must be 0, 90 180 or 270\n");
                                usage(argv[0]);
                        }
                        if (!(*endptr == '\0')) {    /* Not a number */
                                usage(argv[0]);
                        }
                } else if (strcmp(argv[i], "-flip") == 0) { 
                        char *flip = argv[++i]; /* check for flip type */
                        if (strcmp(flip, "horizontal") != 0 &&
                            strcmp(flip, "vertical") != 0) {
                                fprintf(stderr, 
                                      "Flip must be horizontal or vertical\n");
                                usage(argv[0]);
                        } /* assign flip type to rotation */
                        if (strcmp(flip, "horizontal") == 0)
                                rotation = flipHorizontal;
                        else
                                rotation = flipVertical;
                } else if (strcmp(argv[i], "-transpose") == 0) {
                        /* assign transpose command to rotation */
                        rotation = transpose; 
                } else if (strcmp(argv[i], "-time") == 0) {
                        time_file_name = argv[++i];
                } else if (*argv[i] == '-') {
                        fprintf(stderr, "%s: unknown option '%s'\n", argv[0],
                                argv[i]);
                        usage(argv[0]);
                } else if (argc - i > 1) {
                        fprintf(stderr, "Too many arguments\n");
                        usage(argv[0]);
                } else {   
                        /* handle the command line argument being input file */
                        fp = fopen(argv[i], "r");
                        inputFile = argv[i];
                        if (fp == NULL) {
                                fprintf(stderr, 
                                        "File cannot be opened for reading\n");
                                return EXIT_FAILURE;
                        }
                        break;
                }
        }

        /* if no input file is provided, expect input from stdin */
        if (fp == NULL) {
                fp = stdin;
        }

        /* call operation handler with the given rotation type */
        operationHandler(fp, methods, rotation, map, time_file_name, 
                         inputFile);

        fclose(fp);
        return EXIT_SUCCESS;
}




