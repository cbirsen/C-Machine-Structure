/*
 *     unblackedges.c
 *     by Doga Kilinc (dkilin01) & Cansu Birsen (cbirse01), September 28
 *     Solve Sudoku & Unblack Edges
 *
 *     About: Given a pbm file, this program cleans the bits that create black
 *     edges by making them white. The bits are stored in a Bit2_T 2D bit 
 *     vector, and black edge bits are defined as pixels with value 1 and 
 *     located at the edges, or pixels with value 1 and a neighbour that is a 
 *     black edge pixel.
 */

#include <bit2.h>
#include <openOrDie.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <mem.h>
#include <pnmrdr.h>
#include <except.h>
#include <pbmReadWrite.h>
#include <seq.h>

/* function declarations */
void clearEdges(int col, int row, Bit2_T array, int bit, void *p1);
void addToStack(Seq_T stack, int col, int row);
void getTopStack(Seq_T stack, int *col, int *row);
bool stackHandler(int col1, int row1, Bit2_T array, void *p1);

/**********main********
 *
 * About: Opens the file or accepts information from stdin, calls pbmRead to 
 *        store bit values in a 2D bit vector, calls Bit2_map_row_major with
 *        clearEdges to clear the black edges of the file, and prints cleaned
 *        bit results to stdout
 * Inputs:
 * int argc: number of given arguments to start the program
 * char *argv: an array that stores the arguments
 * Return: EXIT_SUCCESS if the program comletes without any errors
 * Expects: argc to be 1 or 2, which is checked by openOrDie
 ************************/
int main(int argc, char *argv[]) {
        /* trying to open the file correctly */
        FILE *fp = openOrDie(argc, argv);

        /* calling pgmHandler to see if the input is valid & store the input */
        Bit2_T bitVector = pbmRead(fp);


        Seq_T neighbourStack = Seq_new(100);
        Bit2_map_row_major(bitVector, clearEdges, neighbourStack);
        
        /* freeing the sequence itself */
        Seq_free(&neighbourStack); 

        /* pbm write */
        pbmWrite(stdout, bitVector);
        
        /* freeing the 2D array and close the input stream */
        Bit2_free(&bitVector);
        fclose(fp);
        
        return EXIT_SUCCESS;
}

/**********addToStack********
 * About: This function is used to store information about the column and row 
 *        indices of a bit in a stack as a char array. 
 * Inputs:
 * Seq_T stack: The sequence structure used as a stack to store information
 * int col: column index value in the location info to be stored
 * int row: row index value in the location info to be stored
 * Expects
 * - Seq_T to be non-null, which is handled by Seq_new function
************************/
void addToStack(Seq_T stack, int col, int row) {
        /* calculatign the length of location info and create a char array */
        int maxLength = snprintf(NULL, 0, "%d,%d", col, row) + 1;
        char *locationInfo = (char*)malloc(sizeof(char) * maxLength);
        assert(locationInfo != NULL);

        /* placing the col and row indices into the locationInfo char array */
        snprintf(locationInfo, maxLength, "%d,%d", col, row);

        /* pushing locationInfo to tack */
        Seq_addhi(stack, locationInfo);
}

/**********getTopStack********
 * About: This function is used to access the  information about the column and
 *        row indices of a bit at the top of a stack as a char array
 * Inputs:
 * Seq_T stack: The sequence structure used as a stack to store information
 * int *col: pointer to column index value extracted from the location info 
 * int *row: pointer to row index value extracted from the location info 
 * Expects
 * - Seq_T to be non-null, which is handled by Seq_new function
 * - *col and *row to be non-null, which is handled by clearEdges function
************************/
void getTopStack(Seq_T stack, int *col, int *row) {
        /* accessing the top of the stack */
        char *locationInfo = Seq_get(stack, Seq_length(stack) - 1);
        int colInfo = 0;
        int rowInfo = 0;
        
        /* keeping track whether we are parsing col or row */
        bool isParsingCol = true; 
        
        for (int i = 0; locationInfo[i] != '\0'; i++) {
                char currentChar = locationInfo[i];
                /* if a comma is read, switch from parsing col to row */
                if (currentChar == ',') {
                        isParsingCol = false;
                } 
                else {
                        /* updating col or row */
                        if (isParsingCol) {
                                colInfo = colInfo * 10 + (currentChar - '0');
                        } 
                        else {
                                rowInfo = rowInfo * 10 + (currentChar - '0');
                        }
                }
        }
        *row = rowInfo;
        *col = colInfo;
}

/**********clearEdges********
 * About: This function is used by the Bit2_map_row_major as an apply function.
 * It updates the bit values such that there are no black edge bits or black 
 * bits that are neighbours to black edges. 
 * Inputs:
 * int col: column index of the bit being visited
 * int row: row index of the bit being visited
 * Bit2_T array: 2D bit vector storing the bit values
 * int bit: value of the current bit being visited 
 * int *p1: pointer to the stack used to store bit location information 
 * Expects
 * - array to be non-null, which is handled by Bit2_new function
 * - *p1 to be non-null, which is handled by Seq_new function
 * - bit to be 0 or 1, which is a requirement of the pbm file and checked by
 * Pnmrdr interface
************************/
void clearEdges(int col, int row, Bit2_T array, int bit, void *p1) {
        /* if the bit is white, no need to unblack */
        if (bit == 0) 
                return;
        /* if the bit is not an edge bit, we skip */
        if (col != 0 && row != 0 && col != Bit2_width(array) - 1 &&
            row != Bit2_height(array) - 1) {
                return;
        }
        /* making the bit white after adding it to the stack */
        addToStack(p1, col, row);
        Bit2_put(array, col, row, 0);
       
        /* while stack not empty, get top element and check its neighbors */
        while (Seq_length(p1) != 0) {
                int *row1 = (int*)malloc(sizeof(int) * 10);
                int *col1 = (int*)malloc(sizeof(int) * 10);
                assert(row1 != NULL && col1 != NULL);
                getTopStack(p1, col1, row1);

                bool hasBlackNeighbor = stackHandler(*col1, *row1, array, p1);
                /* if bit has no black neighbors, remove bit from stack */
                if (!hasBlackNeighbor) {
                        free(Seq_remhi(p1));
                }
                free(row1);
                free(col1);
        }  
}

/**********stackHandler********
 * About: This function is used to see if the neighbors of the bit at 
 * [col1, row1] are black, and if they are, the function pushes their location
 * to the stack. 
 * Inputs:
 * int col1: column index of the bit being visited
 * int row1: row index of the bit being visited
 * Bit2_T array: 2D bit vector storing the bit values
 * int *p1: pointer to the stack used to store bit location information 
 * Returns: true if the current bit has any black neighbors, and false 
 * otherwise
 * Expects
 * - array to be non-null, which is handled by Bit2_new function
 * - *p1 to be non-null, which is handled by Seq_new function
************************/
bool stackHandler(int col1, int row1, Bit2_T array, void *p1) {
        bool hasBlackNeighbor = false;

        /* checking neighbors of the bit being visited and inserting the  
         * location info to the stack if the neighbor is black */
        if (col1 != 0 && Bit2_get(array, col1 - 1, row1) == 1) {
                addToStack(p1, col1 - 1, row1);
                Bit2_put(array, col1 - 1, row1, 0);
                hasBlackNeighbor = true;       
        }
        else if (row1 != 0 && Bit2_get(array, col1, row1 - 1) == 1) {
                addToStack(p1, col1, row1 - 1);
                Bit2_put(array, col1, row1 - 1, 0);
                hasBlackNeighbor = true;      
        }
        else if (col1 != Bit2_width(array) - 1 && 
                 Bit2_get(array, col1 + 1, row1) == 1) {
                addToStack(p1, col1 + 1, row1);
                Bit2_put(array, col1 + 1, row1, 0);
                hasBlackNeighbor = true;
        }
        else if (row1 != Bit2_height(array) - 1 && 
                 Bit2_get(array, col1, row1 + 1) == 1) {
                addToStack(p1, col1, row1 + 1);
                Bit2_put(array, col1, row1 + 1, 0);
                hasBlackNeighbor = true;
        }
        return hasBlackNeighbor;
}