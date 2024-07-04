/*
 *     bit2.c
 *     by Doga Kilinc (dkilin01) & Cansu Birsen (cbirse01), September 28
 *     Solve Sudoku & Unblack Edges
 *
 *     About: This file is used to create a bit vector struct where a client 
 *     can store data in a 2D bit vector. It also has functions that helps the
 *     client to get the width, height, and element size information about the 
 *     vector, traverse the vector in row major and column major order, access
 *     to an element at a certain location, and free the bit vector.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <mem.h>
#include <bit2.h>
#include <bit.h>
#include <except.h>

#define T2 Bit2_T

/**********struct T2********
 * About: This struct holds a Bit vector instance that represents a 2D vector 
 *        and the row and column for that vector.
************************/
struct T2 {
        int rows; /* number of rows in in the 2D vector, at least 1 */
        int cols; /* number of cols in in the 2D vector, at least 1 */
        Bit_T data; /* Bit_T representing the 2D vector*/
};

/**********Bit2_new********
 * About: This function initializes a T2 struct and assigns the given values
 *        such as row and col to the struct variables.
 * Inputs:
 * int row: integer value that represents number of rows in the 2D vector that 
 *          the T2 struct holds
 * int col: integer value that represents number of columns in the 2D vector 
 *          that the T2 struct holds
 * Return: a struct holding a 2D Bit vector and information related to the 
 *         structure
 * Expects
 * - row and col to be greater than 0
 * - that it returns a non-null T2 struct
************************/
T2 Bit2_new(int col, int row) {
        
        /* asserts the expectation for col and row to be > 0 */
        assert(col > 0 && row > 0);

        /* creating an instance of the struct T2 in malloc */
        T2 vector2D;
        NEW(vector2D);
        assert(vector2D != NULL);

        /* initializing the attributes of vector2D */
        vector2D->rows = row;
        vector2D->cols = col;

        /* creating the array */
        vector2D->data = Bit_new(row * col);
        assert(vector2D->data != NULL);

        return vector2D;
}

/**********Bit2_width********
 * About: This function returns the width value (col number) of the 2D vector  
 *        that the T2 struct holds
 * Inputs: 
 * T2 array: struct to store the content of the given data in 2D vector
 * Return: the width value of the 2D vector held by the array struct
 * Expects
 * - that array is non-null, which is already checked by assertion in the 
 * Bit2_new function
************************/
int Bit2_width(T2 array) {
        return array->cols;
}

/**********Bit2_height********
 * About: This function returns the height value (row number) of the 2D vector 
 *        that the T2 struct holds
 * Inputs: 
 * T2 array: struct to store the content of the given data in 2D vector
 * Return: the height value of the 2D vector held by the array struct
 * Expects
 * - that array is non-null, which is already checked by assertion in the 
 * Bit2_new function
************************/
int Bit2_height(T2 array) {
        return array->rows;
}

/**********Bit2_put********
 * About: This function updates the value at the given row and column indices 
 *        to be the value provided by the bit variable and returns the previous
 *        value at that location
 * Inputs: 
 * int row: integer value that represents number of rows in the 2D vector that 
 *          the T2 struct holds
 * int col: integer value that represents number of columns in the 2D vector 
 *          that the T2 struct holds
 * int bit: integer value that is either 0 or 1 that is to be put at the
 *          given index
 * Return:  the previous value that was stored at the given index
 * Expects
 * - that row and col values are at least 0 and at most 
 *   (Bit2_height(T array)-1) and (Bit2_width(T array)-1) respectively
 * - that array is non-null, which is already checked by assertion in the 
 * Bit2_new function
************************/
int Bit2_put(T2 array, int col, int row, int bit) {
        
        assert(col >= 0 && col < Bit2_width(array));
        assert(row >= 0 && row < Bit2_height(array));
        return Bit_put(array->data, row * array->cols + col, bit);
}

/**********Bit2_get********
 * About: This function obtains and returns the bit value that is stored at 
 *        the given index
 * Inputs: 
 * T2 array: struct to store the content of the given data in 2D vector
 * int row: integer value that represents number of rows in the 2D vector that 
 *          the T2 struct holds
 * int col: integer value that represents number of columns in the 2D vector 
 *          that the T2 struct holds
 * Return:  the bit value that is stored at the given index
 * Expects
 * - that row and col values are at least 0 and at most 
 *   (Bit2_height(T array)-1) and (Bit2_width(T array)-1) respectively
 * - that array is non-null, which is already checked by assertion in the 
 * Bit2_new function
************************/
int Bit2_get(T2 array, int col, int row) {
        assert(col >= 0 && col < Bit2_width(array));
        assert(row >= 0 && row < Bit2_height(array));
        return Bit_get(array->data, row * array->cols + col);
}

/**********Bit2_map_row_major********
 * About: This function traverses the 2D vector held in the struct such that 
 * column indices vary more rapidly than row indices
 * Inputs:
 * T2 array: struct to store the contents of the given data in 2D vector
 * apply function: the function to be applied on all the elements of the vector
 * cl pointer: client specific pointer input
 * Return: none
 * Expects
 * - non-null T2 array, which is already checked by assertion in the 
 * Bit2_new function
************************/
void Bit2_map_row_major(T2 array, void apply(int col, int row, T2 array,
                               int bit, void *p1), void *cl) {
        /* creating a nested loop such that column indices vary more rapidly */
        for (int i = 0; i < array->rows; i++) {
                for (int j = 0; j < array->cols; j++) {
                        apply(j, i, array, Bit2_get(array, j, i), cl);
                }
        }                
}

/**********Bit2_map_col_major********
 * About: This function traverses the 2D vector held in the struct such that 
 * row indices vary more rapidly than column indices
 * Inputs:
 * T2 array: struct to store the content of the given data in 2D vector
 * apply function: the function to be applied on all the elements of the vector
 * cl pointer: client specific pointer input
 * Return: none
 * Expects
 * - non-null T2 array, which is already checked by assertion in the 
 * Bit2_new function
************************/
void Bit2_map_col_major(T2 array, void apply(int col, int row, T2 array,
                               int bit, void *p1), void *cl) {
        /* creating a nested loop such that row indices vary more rapidly */
        for (int j = 0; j < array->cols; j++) {
                for (int i = 0; i < array->rows; i++) {
                        apply(j, i, array, Bit2_get(array, j, i), cl);
                }
        }                        
}

/**********Bit2_free********
 * About: This function frees the memory allocated to the 2D vector and the T2
 *        struct
 * Inputs: 
 * T2 *array: address of the 2D vector to store the content of the given data
 * Return: none
 * Expects
 * - that array is non-null, which is already checked by assertion in the 
 * Bit2_new function
************************/
void Bit2_free(T2 *array) {
        
        /* freeing the bit vector held by the struct */
        Bit_T vectorToFree = (*array)->data;
        Bit_free(&vectorToFree);

        /* freeing the struct */
        FREE(*array);
}


#undef T2