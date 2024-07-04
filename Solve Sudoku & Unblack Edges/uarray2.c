/*
 *     uarray2.c
 *     by Doga Kilinc (dkilin01) & Cansu Birsen (cbirse01), September 28
 *     Solve Sudoku & Unblack Edges
 *
 *     About: This file can be used to create a 2D Uarray where a client can
 *     store data in a 2D array. It also has functions that helps the client
 *     to get the width, height, and element size information about the array, 
 *     traverse the array in row major and column major orders, access to an
 *     element at a certain location, and free the UArray.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <mem.h>
#include <uarray2.h>
#include <uarray.h>
#include <except.h>

#define T2 UArray2_T

/**********struct T2********
 * About: This struct holds a UArray instance that represents a 2D array and
 *        the row, column, and element size information for that array.
************************/
struct T2 {
        int rows; /* number of rows in in the 2D array, at least 1 */
        int cols; /* number of cols in in the 2D array, at least 1 */
        int elmSize; /* number of elements in the 2D array */
        UArray_T data; /* UArray_T representing the 2D array*/
};


/**********UArray2_new********
 * About: This function initializes a T2 struct and assigns the given values
 *        such as row, col, elementSize to the struct variables.
 * Inputs:
 * int row: integer value that represents number of rows in the 2D array that 
 *          the T2 struct holds
 * int col: integer value that represents number of columns in the 2D array 
 *          that the T2 struct holds
 * int elementSize: integer value representing the size of an element in the 
 *          2D array that the T2 struct holds
 * Return: a struct holding a 2D UArray and information related to the 
 *         structure
 * Expects
 * - row, col, elementSize to be greater than 0
 * - that it returns a non-null T2 struct
************************/
T2 UArray2_new(int col, int row, int elementSize) {

        /* asserts the expectation for col, row, and elementSize to be > 0 */
        assert(col > 0 && row > 0 && elementSize > 0);

        /* creating an instance of the struct T2 in malloc */
        T2 array2D;
        NEW(array2D);
        assert(array2D != NULL);

        /* initializing the attributes of array2D */
        array2D->rows = row;
        array2D->cols = col;
        array2D->elmSize = elementSize;

        /* creating the array */
        array2D->data = UArray_new(row * col, elementSize);
        assert(array2D->data != NULL);

        return array2D;
}

/**********UArray2_width********
 * About: This function returns the width value (col number) of the 2D UArray
 *        held by the array struct.
 * Inputs: 
 * T2 array: struct to store the content of the given data in 2D UArray
 * Return: the width value of the 2D UArray held by the array struct
 * Expects
 * - that array is non-null, which is already checked by assertion in the 
 * UArray2_new function
************************/
int UArray2_width(T2 array) {
        
        return array->cols;
}

/**********UArray2_height********
 * About: This function returns the height value (row number) of the 2D UArray
 *        held by the array struct.
 * Inputs: 
 * T2 array: struct to store the content of the given data in 2D UArray
 * Return: the height value of the 2D array held by the array struct 
 * Expects
 * - that array is non-null, which is already checked by assertion in the 
 * UArray2_new function
************************/
int UArray2_height(T2 array) {
        
        return array->rows;
}

/**********UArray2_size********
 * About: This function returns the size of an element in the 2D UArray held
 *        by the array struct.
 * Inputs: 
 * T2 array: struct to store the content of the given data in 2D UArray
 * Return: integer value that represents the size of an element held in the 2D
 *         UArray in the T2 struct
 * Expects
 * - that array is non-null, which is already checked by assertion in the 
 * UArray2_new function
************************/
int UArray2_size(T2 array) {

        return array->elmSize;
}

/**********UArray2_at********
 * About: This function returns a pointer to the element at the given row and
 *        col values in the 2D UArray held by the array struct.
 * Inputs:
 * T2 array: struct to store the content of the given data in 2D UArray
 * int row: row index value to find the element at
 * int col: col index value to find the element at
 * Return: a void pointer to the element at the given row and col indices
 * Expects
 * - that array is non-null
 * - that row and col values are at least 0 and at most 
 *   (UArray2_height(T array)-1) and (UArray2_width(T array)-1) respectively
 * - that it returns an non-null pointer
************************/
void *UArray2_at(T2 array, int col, int row) {
        
        assert(col >= 0 && col < UArray2_width(array));
        assert(row >= 0 && row < UArray2_height(array));
        return UArray_at(array->data, row * array->cols + col);
}

/**********UArray2_map_row_major********
 * About: This function traverses the 2D UArray held in the struct such that 
 * column indices vary more rapidly than row indices
 * Inputs:
 * T2 array: struct to store the content of the given data in 2D UArray
 * apply function: the function to be applied on all the elements of the array
 * cl pointer: client specific pointer input
 * Return: none
 * Expects
 * - non-null T2 array, which is already checked by assertion in the 
 * UArray2_new function
************************/
void UArray2_map_row_major(T2 array, void apply(int col, int row, 
                                  T2 array, void *p1, void *p2), void *cl) {

        /* creating a nested loop such that column indices vary more rapidly */
        for (int i = 0; i < array->rows; i++) {
                for (int j = 0; j < array->cols; j++) {
                        apply(j, i, array, UArray2_at(array, j, i), cl);
                }
        }
}

/**********UArray2_map_col_major********
 * About: This function traverses the 2D UArray held in the struct such that 
 * row indices vary more rapidly than column indices
 * Inputs:
 * T2 array: struct to store the content of the given data in 2D UArray
 * apply function: the function to be applied on all the elements of the array
 * cl pointer: client specific pointer input
 * Return: none
 * Expects
 * - non-null T2 array, which is already checked by assertion in the 
 * UArray2_new function
************************/
void UArray2_map_col_major(T2 array, void apply(int col, int row, 
                                  T2 array, void *p1, void *p2), void *cl) {
                                        
        /* creating a nested loop such that row indices vary more rapidly */
        for (int j = 0; j < array->cols; j++) {
                for (int i = 0; i < array->rows; i++) {
                        apply(j, i, array, UArray2_at(array, j, i), cl);
                }
        }
}

/**********UArray2_free********
 * About: This function frees the memory allocated to the 2D UArray and the T2
 *        struct
 * Inputs: 
 * T2 *array: address of the 2D UArray to store the content of the given data
 * Return: none
 * Expects
 * - that array is non-null, which is already checked by assertion in the 
 * UArray2_new function
************************/
void UArray2_free(T2 *array) {

        /* freeing the UArray held by the struct */
        UArray_T arrayToFree = (*array)->data;
        UArray_free(&arrayToFree);

        /* freeing the struct */
        FREE(*array);
}

#undef T2