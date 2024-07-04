/*
 *     pbmReadWrite.c
 *     by Doga Kilinc (dkilin01) & Cansu Birsen (cbirse01), September 18
 *     Solve Sudoku & Unblack Edges
 *
 *     About: This program checks if a given file is in the correct pbm format.
 *            If it is, the program creates a 2D Bit2_T object to store the
 *            data. The program can also be used to print out the cleared 
 *            data in the 2D Bit2_T object to an output file.
 *     
 */

#include <stdio.h>
#include <stdlib.h>
#include <pbmReadWrite.h>
#include <assert.h>
#include <bit2.h>
#include <pnmrdr.h>
#include <except.h>
#include <mem.h>

/**********pbmRead********
 *
 * About: This function takes a pbm file and stores the data into a newly
 *        created Bit2_T bitVector
 * Inputs: 
 * FILE *inputfp: a pointer to a file to get the data using Pnmrdr_new
 * Return: Bit2_T bitVector where the opened data from the file is stored
 * Expects: 
 * - the given file input is in the pbm format
 * - the given file input is a bitmap and has the correct dimensions
 ************************/
Bit2_T pbmRead(FILE *inputfp) {
        /* checking if the input is in the pbm format */
        Except_T Pnmrdr_Badformat;
        Pnmrdr_T image;
        
        TRY 
                image = Pnmrdr_new(inputfp);
        EXCEPT(Pnmrdr_Badformat) {
                fclose(inputfp);
                fprintf(stderr, "pbm file promised but not delivered\n");
                exit(EXIT_FAILURE);
        }       
        END_TRY;

        /* checking if the input is a bitmap and has the correct dimensions */
        Pnmrdr_mapdata imageMap = Pnmrdr_data(image);
        if (imageMap.type != 1 || imageMap.width == 0 || 
            imageMap.height == 0) {
                Pnmrdr_free(&image);
                fclose(inputfp);
                fprintf(stderr, "pbm file promised but not delivered\n");
                exit(EXIT_FAILURE);
        }
        Bit2_T bitVector = Bit2_new(imageMap.width, imageMap.height);
        
        /* reading input from the file and fillind the bitVector */
        Bit2_map_row_major(bitVector, arrayFiller, image);
        Pnmrdr_free(&image);
        return bitVector;
}

/**********pbmWrite********
 *
 * About: This function prints the values in a 2D bit vector in the P1 format 
 *        to the output file. 
 * Inputs:
 * FILE *outputfp: a pointer to an output file where the output is printed at
 * Bit2_T bitmap: a 2D bitVector where the data is stored at
 * Return: none
 ************************/
void pbmWrite(FILE *outputfp, Bit2_T bitmap) {
        
        /* obtaining the size info of the unblacked data in P1 format */
        int width = Bit2_width(bitmap);
        int height = Bit2_height(bitmap);
        
        /* printing the header information to the output */
        fprintf(outputfp, "P1\n%d %d\n", width, height);

        /* printing the unblacked bitVector to the output */
        Bit2_map_row_major(bitmap, arrayPrinter, outputfp);
}

/**********arrayFiller********
 *
 * About: This function is an apply function for Bit2_map_row_major. It is 
 *        used to get a single data from the *p1 pointer (a Pnmrdr_T object)
 *        and put that data in the Bit2_T array at the given row and col
 *        indices
 * Inputs:
 * int col: the column value of the index where the data is going to be put at
 * int row: the row value of the index where the data is going to be put at
 * Bit2_T array: a 2D Bit2_T object where the whole data is stored at
 * int bit: integer value of the current data being visited
 * void *p1: pointer to an object where the data is read from ( Pnmrdr_T
 *           object for this program)
 * Return: none
 * Expects: 
 * - *p1 to be non-null which is handled in the pbmRead function
 ************************/
void arrayFiller(int col, int row, Bit2_T array, int bit, void *p1) {
        (void) bit;

        /* reading the initial data from p1 and filling out 2D bit vector */
        unsigned num = Pnmrdr_get(p1);
        Bit2_put(array, col, row, num);
}

/**********arrayPrinter********
 *
 * About: This function is an apply function for Bit2_map_row_major. It is 
 *        used to print out the data to the given output file
 * Inputs:
 * int col: the column value of the data that is to be printed
 * int row: the row value of the data that is to be printed
 * Bit2_T array: a 2D Bit2_T object where the whole data is stored at
 * int bit: integer value of the current data being visited
 * void *p1: pointer to an object where the output data is printed at (an
 *           output file for this program)
 * Return: none
 * Expects: 
 * - *p1 to be non-null 
 ************************/
void arrayPrinter(int col, int row, Bit2_T array, int bit, void *p1) {

        (void) bit;
        assert(p1 != NULL);

        /* printing the data utilizing the Bit2_get function */
        fprintf(p1, "%d", Bit2_get(array, col, row));

        /* printing a new line after done printing one row */
        if (col == Bit2_width(array) - 1)
                fprintf(p1, "\n");
}