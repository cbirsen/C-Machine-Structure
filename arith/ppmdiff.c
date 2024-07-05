/* ppmdiff.c
 * 
 * Cansu Birsen and Ethan Goldman
 * Octber 13, 2023 
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "assert.h"
#include "pnm.h"
#include "a2methods.h"
#include "a2plain.h"

int widthHeighCheck(Pnm_ppm image1, Pnm_ppm image2);
float meanSquareDiff(Pnm_ppm image1, Pnm_ppm image2);

int main(int argc, char *argv[]) {

        (void) argc;

        FILE *fp1;
        FILE *fp2;
        
        /* Read args from command line and initialize files*/
        if (strcmp(argv[1], "-") == 0 && strcmp(argv[2], "-") == 0) {
            fprintf(stderr, "Only one of the arguments can be \"-\"\n");
            exit(EXIT_FAILURE);
        } else if (strcmp(argv[1], "-") == 0) {
                fp1 = stdin;
                fp2 = fopen(argv[2], "r");
        } else if (strcmp(argv[2], "-") == 0) {
                fp1 = fopen(argv[1], "r");
                fp2 = stdin;
        } else {
                fp1 = fopen(argv[1], "r");
                fp2 = fopen(argv[2], "r");
        }

        assert(fp1 != NULL);
        assert(fp2 != NULL);

        if (fp1 == NULL || fp2 == NULL) {
                fprintf(stderr, "File cannot be opened for reading\n");
                return EXIT_FAILURE;
        }

        /* Set method types for Pnm_ppm */
        A2Methods_T methods = uarray2_methods_plain;
        assert(methods);
        A2Methods_mapfun *map = methods->map_default; 
        assert(map);

        /* Read in files to Pnm_ppm structs */
        Pnm_ppm image1 = Pnm_ppmread(fp1, methods);
        Pnm_ppm image2 = Pnm_ppmread(fp2, methods);

        /* Print out output to stdout */
        if (widthHeighCheck(image1, image2) == 1) {
                fprintf(stdout, "%0.1f\n", 1.0);
        } else {
                fprintf(stdout, "%0.4f\n", meanSquareDiff(image1, image2));
        }

        return EXIT_SUCCESS;

}

/* Checks that the width/height of the files don't differ by more than 1 */
int widthHeighCheck(Pnm_ppm image1, Pnm_ppm image2) {

        if (fmax(image1->width, image2->width) - 
            fmin(image1->width, image2->width) > 1) {
                fprintf(stderr, "Widths differ by more than 1\n");
                return 1;
        }

        if (fmax(image1->height, image2->height) - 
            fmin(image1->height, image2->height) > 1) {
                fprintf(stderr, "Heights differ by more than 1\n");
                return 1;
        }
        
        return 0;
}

/* Calculates the mean square difference for the pixels in the two files */
float meanSquareDiff(Pnm_ppm image1, Pnm_ppm image2) {
        float width = fmin(image1->width, image2->width);
        float height = fmin(image1->height, image2->height);
        float d1 = image1->denominator;
        float d2 = image2->denominator;
        float sum = 0.0;

        for (int row = 0; row < height; row++) {
                for (int col = 0; col < width; col++) {
                        Pnm_rgb new_pixel1 
                               = image1->methods->at(image1->pixels, col, row);
                        Pnm_rgb new_pixel2 
                               = image2->methods->at(image2->pixels, col, row);
                                              
                        float redSquared 
                                = pow(((float)(new_pixel1->red / d1) - (float)(new_pixel2->red / d2)), 2);
                        float greenSquared 
                                = pow(((float)(new_pixel1->green / d1) - (float)(new_pixel2->green / d2)), 2);
                        float blueSquared 
                                = pow(((float)(new_pixel1->blue / d1) - (float)(new_pixel2->blue / d2)), 2);

                        sum += redSquared + greenSquared + blueSquared;
                }
        }

        return sqrt(sum / (float) (3 * width * height));
}

