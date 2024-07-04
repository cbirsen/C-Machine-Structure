/*
 *     a2plain.h
 *     by Doga Kilinc (dkilin01) & Cansu Birsen (cbirse01), October 10
 *     PPM Operations
 *
 *     About: This file helps the client to create a UArray2 instance, which
 *            represents a 2D array. THe functions to get information or
 *            modify the 2D array are implemented for the A2Methods_UArray2
 *            suite.
 *              
 */
 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <a2plain.h>
#include "uarray2.h"

/******************
 * All the possible checked runtime errors are checked by UArray2.c.
 * Please refer UArray2.c to see the explanations of the CRE in more detail.
 ************************/

/**********new********
 * About: This function is used to initialize a UArray2 instance, which 
 *        represents a 2D array, with the given dimensions
 * Inputs:
 * int width: integer value that represents number of columns in the 2D array  
 * int height: integer value that represents number of rows in the 2D array 
 * int size: integer value that represents the size value that each cell
 *           can store
 * Return: UArray2 instance with the given dimensions
************************/
static A2Methods_UArray2 new(int width, int height, int size)
{
        return UArray2_new(width, height, size);
}

/**********new********
 * About: This function is used to initialize a UArray2 instance, which 
 *        represents a 2D array, with the given dimensions
 * Inputs:
 * int width: integer value that represents number of columns in the 2D array  
 * int height: integer value that represents number of rows in the 2D array 
 * int size: integer value that represents the size value that each cell
 *           can store
 * Return: UArray2 instance with the given dimensions
 * Note: the given blocksize parameters is not used in this function
************************/
static A2Methods_UArray2 new_with_blocksize(int width, int height, int size,
                                            int blocksize)
{
        (void) blocksize;
        return UArray2_new(width, height, size);
}

/**********a2free********
 * About: This function frees the memory allocated to for the 2D array
 * Inputs: 
 * A2Methods_UArray2 *array2: address of the A2Methods_UArray2 instance that is
 *                            used to store the content of data
 * Return: none
************************/
static void a2free(A2Methods_UArray2 *array2)
{
        UArray2_free((UArray2_T *) array2);
}

/**********width********
 * About: This function returns the width value (col number) of the 2D 
 *        A2Methods_UArray2 instance
 * Inputs: 
 * T A2Methods_UArray2 array2: 2D array that is used to store data
 * Return: the width value of the 2D array 
************************/
static int width(A2Methods_UArray2 array2)
{
        return UArray2_width(array2);
}

/**********height********
 * About: This function returns the height value (row number) of the 2D 
 *        A2Methods_UArray2 instance
 * Inputs: 
 * T A2Methods_UArray2 array2: 2D array that is used to store data
 * Return: the height value of the 2D array 
************************/
static int height(A2Methods_UArray2 array2)
{
        return UArray2_height(array2);
}

/**********size********
 * About: This function returns the size of an element in the 2D UArray2b
 *        A2Methods_UArray2 instance
 * Inputs: 
 * A2Methods_UArray2 array2: 2D array that is used to store data
 * Return: integer value that represents the size of an element held in the 2D
 *         array
************************/
static int size(A2Methods_UArray2 array2)
{
        return UArray2_size(array2);
}

/**********blocksize********
 * About: This function is supposed to return the square root of number of 
 *        cells in a  block in the 2D A2Methods_UArray2 instance. However,
 *        since this file uses UArray2_new to create the A2Methods_UArray2
 *        instance and therefore A2Methods_UArray2 instance does not contain
 *        any blocks, it returns 1 by defualt.
 * Inputs: 
 * A2Methods_UArray2 array2: 2D array that is used to store data
 * Return: 1 by default
************************/
static int blocksize(A2Methods_UArray2 array2)
{
        (void) array2;
        return 1;
}

/**********at********
 * About: This function returns a pointer to the element at the given row and
 *        col values in the 2D A2Methods_UArray2 array
 * Inputs:
 * A2Methods_UArray2 array2: 2D array that is used to store data
 * int col: col index value to find the element at
 * int row: row index value to find the element at
 * Return: a void pointer to the element at the given row and col indices
************************/
static A2Methods_Object *at(A2Methods_UArray2 array2, int col, int row)
{
        return UArray2_at(array2, col, row);
}

/**********UArray2_applyfun********
 * About: This function is an apply function that is used by map_row_major
 *        and map_col_major.
************************/
typedef void UArray2_applyfun(int col, int row, UArray2_T array2, void *elem, 
                              void *cl);

/**********map_row_major********
 * About: This function traverses the 2D A2Methods_UArray2 instance such that 
 *        column indices vary more rapidly than the row indices
 * Inputs:
 * A2Methods_UArray2 array2: 2D array that is used to store data
 * apply function: the function to be applied on all the elements of the array
 * cl pointer: client specific pointer input
 * Return: none
************************/
static void map_row_major(A2Methods_UArray2 uarray2,
                          A2Methods_applyfun apply,
                          void *cl)
{
        UArray2_map_row_major(uarray2, (UArray2_applyfun*)apply, cl);
}

/**********map_col_major********
 * About: This function traverses the 2D A2Methods_UArray2 instance such that 
 *        row indices vary more rapidly than the column indices
 * Inputs:
 * A2Methods_UArray2 array2: 2D array that is used to store data
 * apply function: the function to be applied on all the elements of the array
 * cl pointer: client specific pointer input
 * Return: none
************************/
static void map_col_major(A2Methods_UArray2 uarray2,
                          A2Methods_applyfun apply,
                          void *cl)
{
        UArray2_map_col_major(uarray2, (UArray2_applyfun*)apply, cl);
}

struct small_closure {
        A2Methods_smallapplyfun *apply; 
        void                    *cl;
};

static void apply_small(int i, int j, UArray2_T uarray2,
                        void *elem, void *vcl)
{
        struct small_closure *cl = vcl;
        (void)i;
        (void)j;
        (void)uarray2;
        cl->apply(elem, cl->cl);
}

static void small_map_row_major(A2Methods_UArray2        a2,
                                A2Methods_smallapplyfun  apply,
                                void *cl)
{
        struct small_closure mycl = { apply, cl };
        UArray2_map_row_major(a2, apply_small, &mycl);
}

static void small_map_col_major(A2Methods_UArray2        a2,
                                A2Methods_smallapplyfun  apply,
                                void *cl)
{
        struct small_closure mycl = { apply, cl };
        UArray2_map_col_major(a2, apply_small, &mycl);
}

/**********struct A2Methods_T********
 * About: This struct wraps the functions for UArray2 in the A2Methods_T suite
 *        format
************************/
static struct A2Methods_T uarray2_methods_plain_struct = {
        new,
        new_with_blocksize,
        a2free,
        width,
        height,
        size,
        blocksize,
        at,
        map_row_major,              
        map_col_major,               
        NULL,  
        map_row_major,              /* map_default */
        small_map_row_major,        
        small_map_col_major,        
        NULL,
        small_map_row_major,       /* small map_default */
};


A2Methods_T uarray2_methods_plain = &uarray2_methods_plain_struct;
