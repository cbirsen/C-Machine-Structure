/*
 *     sudoku.c
 *     by Doga Kilinc (dkilin01) & Cansu Birsen (cbirse01), September 28
 *     Solve Sudoku & Unblack Edges
 *
 *     About: Given a pgm file that represents a solved sudoku problem with 9 
 *     by 9 dimension and 9 as the max value, this program checks if the given 
 *     solution is valid. The program checks to see if each value in the same 
 *     row, same column, or same 3-by-3 subsudoku are unique, and exits with 
 *     success if the solution is valid.
 */

#include <uarray2.h>
#include <openOrDie.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <mem.h>
#include <pnmrdr.h>
#include <except.h>

/* function declarations */
UArray2_T pgmHandler(FILE *fp);
void arrayFiller(int col, int row, UArray2_T array, void *p1, void *p2);
void zeroChecker(int col, int row, UArray2_T array, void *p1, void *p2);
bool checkSubmaps(UArray2_T array);
bool checkSubmapsHelper(UArray2_T array, int col, int row);
bool checkRow(UArray2_T array);
bool checkCol(UArray2_T array);


/**********main********
 *
 * About: Opens the file or accepts information from stdin, calls pgmHandler
 *        to store sudoku values in a 2D array, calls checker functions to see
 *        if the values fit to the sudoku rules, and depending on the result,
 *        exits with failure or success
 * Inputs:
 * int argc: number of given arguments to start the program
 * char *argv: an array that stores the arguments
 * Return: EXIT_SUCCESS if the sudoku is valid, EXIT_FAILURE otherwise
 * Expects: argc to be 1 or 2, which is checked by openOrDie
 ************************/
int main(int argc, char *argv[]) {
        /* trying to open the file correctly */
        FILE *fp = openOrDie(argc, argv);

        /* calling pgmHandler to see if the input is valid & store the input */
        UArray2_T sudokuArray = pgmHandler(fp);

        /* checking if the input matches with the sudoku rules */
        bool result = checkRow(sudokuArray) && checkCol(sudokuArray) &&
                      checkSubmaps(sudokuArray);
        
        /* free the 2D array and close the input stream */
        UArray2_free(&sudokuArray);
        fclose(fp);
        
        if (result) {
                return EXIT_SUCCESS;
        }
                
        return EXIT_FAILURE;
}

/**********pgmHandler********
 * About: This function reads information from a pgm file given a file pointer.
 *        It checks if the file is in pgm format, with 9 by 9 dimensions and
 *        9 as the max value, as the file is supposed to represent a solved
 *        sudoku problem. Then it reads and inserts the values from the file to
 *        a 2D UArray, and finally checks that the none of the values are 0.
 * Inputs:
 * FILE *fp: a pointer to a file to read information from
 * Return: a UArray2_T structure that holds the values from the solved sudoku
 *         problem
 * Expects
 * - The file to be in Pnmrdr interface readable pgm format with 9 by 9 
 * dimensions and 9 as the max value
 * - The file to not contain any pixels with 0 intensity value
************************/
UArray2_T pgmHandler(FILE *fp) {
        Except_T Pnmrdr_Badformat; /* checks if input is a pnm and is 9x9 */
        Pnmrdr_T image;
        TRY 
                image = Pnmrdr_new(fp);
        EXCEPT(Pnmrdr_Badformat) {
                fclose(fp);
                exit(EXIT_FAILURE);
        }       
        END_TRY;
        Pnmrdr_mapdata iMap = Pnmrdr_data(image);
        if (iMap.type != 2 || iMap.width != 9 || iMap.height != 9 
            || iMap.denominator != 9) {
                Pnmrdr_free(&image);
                fclose(fp);
                exit(EXIT_FAILURE);
        } /* initialize, fill a UArray2_T & check that sudoku values are > 0 */
        UArray2_T sudoku = UArray2_new(iMap.width, iMap.height, sizeof(long));
        UArray2_map_row_major(sudoku, arrayFiller, image);
        bool zeroExists = false;
        UArray2_map_row_major(sudoku, zeroChecker, &zeroExists);
        if (zeroExists) {
                UArray2_free(&sudoku);
                Pnmrdr_free(&image);
                fclose(fp);
                exit(EXIT_FAILURE);
        }
        Pnmrdr_free(&image);
        return sudoku;
}

/**********arrayFiller********
 * About: This function is written to be used as the apply function of 
 *        UArray2_map_row_major, and fills the index being visited with the 
 *        value obtained from p2
 * int col: the column value of the cell that is going to be filled
 * int row: the row value of the cell that is going to be filled
 * UArray2_T array: array that is being traversed by the UArray2_map_row_major
 * void *p1: pointer to an current element visited   
 * void *p2: pointer to a Pnmrdr_T object
 * Return: none
 * Expects
 * - p1 and p2 to be non-null, which is handled by UArray_at and pgmHandler
 * respectively
************************/
void arrayFiller(int col, int row, UArray2_T array, void *p1, void *p2) {
        (void) col;
        (void) row;
        (void) array;

        /* get the next value in the pgm file */
        unsigned num = Pnmrdr_get(p2);

        /* update p1 pointer so it points to the value extracted from p2 */
        long *arrayElt = p1;
        *arrayElt = num;
}

/**********zeroChecker********
 * About: This function is written to be used as the apply function of 
 *        UArray2_map_row_major, and checks the UArray2 data structure to see 
 *        if there is any 0 value.
 * Inputs:
 * int col: the column value of the cell that is going to be checked
 * int row: the row value of the cell that is going to be checked
 * UArray2_T array: array that is being traversed by the UArray2_map_row_major
 * void *p1: pointer to an current element visited   
 * void *p2: pointer to a boolean keeping track whether a zero value was seen
 * Return: none
 * Expects
 * - p1 and p2 pointers to be non-null, which is handled by UArray_at and 
 * pgmHandler respectively
************************/
void zeroChecker(int col, int row, UArray2_T array, void *p1, void *p2) {
        (void) col;
        (void) row;
        (void) array;
        
        long *arrayElt = p1;
        bool *zeroExists = p2;

        /* check if the current element being visited is zero */
        if (*arrayElt == 0) {
                *zeroExists = true;
        }
}

/**********checkSubmaps********
 * About: This function checks if each 3x3 sub sudokus is valid with the help
 *        of checkSubmapsHelper function
 * Inputs:
 * UArray2_T array: array that where the data is stored
 * Return: return true if the submap is valid, false otherwise
************************/
bool checkSubmaps(UArray2_T array) {
        
        /* loops the middle element in each submap and sends those indices
         * to  checkSubmapsHelper */
        for (int i = 1; i < UArray2_height(array); i += 3) {
                for (int j = 1; j < UArray2_width(array); j += 3) {
                        if (!checkSubmapsHelper(array, j, i)) {
                                return false;
                        }
                }
        }

        return true;
}

/**********checkSubmapsHelper********
 * About: Given a index (the middle element of the sub-sudoku), this function
 *        checks if the 3x3 sub-sudoku is valid
 * Inputs:
 * int col: the column value of the middle cell of the submap
 * int row: the row value of the middle cell of the submap
 * UArray2_T array: array that where the data is stored
 * Return: return true if the submap is valid, false otherwise
  * Expects
 * - char subArray to be non-null
************************/
bool checkSubmapsHelper(UArray2_T array, int col, int row) {

        /* initializing a char array with the size of submap element size */
        char *subArray = (char*)malloc(sizeof(char) * 9);
        assert(subArray != NULL);  
        /* filling the char array with 'a's */
        for (unsigned int i = 0; i < sizeof(subArray); i++) {
                subArray[i] = 'a';
        }  
        /* checking if the number in the submap is from 0 to 9, changing
         * char array value to 'f' */
        for (int i = row - 1; i <= row + 1; i++) {
                for (int j = col - 1; j <= col + 1; j++) {
                        int *num = UArray2_at(array, j, i);
                        subArray[*num - 1] = 'f';
                }
        }
        /* if there is any 'a' left in the char array, it means that the
        * submap doesn't have all values from 0 to 9 */
        for (unsigned int k = 0; k < sizeof(subArray); k++) {
                if (subArray[k] == 'a') {
                        free(subArray);
                        return false;
                }
        }
        free(subArray);
        return true;
}

/**********checkRow********
 * About: This function is used to check that there are no two same intensity 
 *        values on the same row.
 * Inputs: 
 * UArray2_T array: 2D data structure to check the rows from 
 * Return: a boolean variable indicating whether each value on the same row is
 *         unique
 * Expects
 * - rowArray to be non-null
************************/
bool checkRow(UArray2_T array) {
        /* create a char array to keep track of values on the same row */
        char *rowArray = (char*)malloc(sizeof(char) * 9);
        assert(rowArray != NULL);
        /* make all values on char array 'a' to indicate unseen */
        for (unsigned int i = 0; i < sizeof(rowArray); i++) {
                rowArray[i] = 'a';
        }
        /* traverse a row of the array and keep track of unique values */
        for (int i = 0; i < UArray2_height(array); i++) {
                for (int j = 0; j < UArray2_width(array); j++) {
                        int *num = UArray2_at(array, j, i);
                        rowArray[*num - 1] = 'f';
                }
                /* see if there are any unseen values from 1-9 */
                for (unsigned int k = 0; k < sizeof(rowArray); k++) {
                        /* if yes, there needs to be non-unique intensity */
                        if (rowArray[k] == 'a') {
                                free(rowArray);
                                return false;
                        }
                }
                /* set values of rowArray back to unseen */
                for (unsigned int l = 0; l < sizeof(rowArray); l++) {
                        rowArray[l] = 'a';
                }
        }
        free(rowArray);
        return true;
}

/**********checkCol********
 * About: This function is used to check that there are no two same intensity 
 *        values on the same column.
 * Inputs:
 * UArray2_T array: 2D data structure to check the columns from
 * Return: a boolean variable indicating whether each value on the same column
 *         is unique
 * Expects
 * - colArray to be non-null
************************/
bool checkCol(UArray2_T array) {
        /* create a char array to keep track of values on the same column */
        char *colArray = (char*)malloc(sizeof(char) * 9);
        assert(colArray != NULL);
        /* make all values on char array 'a' to indicate unseen */
        for (unsigned int i = 0; i < sizeof(colArray); i++) {
                colArray[i] = 'a';
        }
        /* traverse a column of the array and keep track of unique values */
        for (int j = 0; j < UArray2_width(array); j++) {
                for (int i = 0; i < UArray2_height(array); i++) {
                        int *num = UArray2_at(array, j, i);
                        colArray[*num - 1] = 'f';
                }
                /* see if there are any unseen values from 1-9 */
                for (unsigned int k = 0; k < sizeof(colArray); k++) {
                        /* if yes, there needs to be non-unique intensity */
                        if (colArray[k] == 'a') {
                                free(colArray);
                                return false;
                        }
                }
                /* set values of rowArray back to unseen */
                for (unsigned int l = 0; l < sizeof(colArray); l++) {
                        colArray[l] = 'a';
                }
        }
        free(colArray);
        return true;
}



