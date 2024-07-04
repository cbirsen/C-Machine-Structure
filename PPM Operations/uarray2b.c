/*
 *     uarray2b.h
 *     by Doga Kilinc (dkilin01) & Cansu Birsen (cbirse01), October 10
 *     PPM Operations
 *
 *     About: This file can be used to create a 2D UArray2b where a client can
 *     store data in a 2D array divided into blocks. It also has functions that
 *     helps the client initialize each block size to 64KB (given element size
 *     is under 64KB), to get the width, height, element size, and block size 
 *     information about the array, traverse the array in block major order, 
 *     access to an element at a certain location, and free the UArray2b.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <uarray2b.h>
#include <uarray2.h>
#include <uarray.h>
#include <mem.h>
#include <math.h>

#define T UArray2b_T
#define KB 1024
#define blockMem 64
#define minBlockSize 1

/**********struct T********
 * About: This struct holds a UArray2_T instance that holds UArray_Ts, and
 *        row, col, blockSize, and elmSize information of the UArray2_T.
************************/
struct T {
        int rows; /* number of rows in in the 2D array */
        int cols; /* number of cols in in the 2D array */
        int blockSize; /* sqrt of the number of cells in a block */
        int elmSize; /* number of elements in the 2D array */
        UArray2_T data; /* UArray2_T holding UArray_Ts */
};

/**********struct mapParameters********
 * About: This struct hold the parameters, which are UArray2b_T, apply 
 *        function, cl pointer, that needs to be passed to the insideBlockMap 
 *        function
************************/
struct mapParameters {
        T array2b;
        void (*apply)(int col, int row, T array2b, void *elem, void *cl);
        void *cl;
};

/* function declarations */
void blockCreator(int col, int row, UArray2_T array, void *elem, 
                  void *structT);
void blockFree(int col, int row, UArray2_T array, void *elem, void *p2);
void insideBlockMap(int col, int row, UArray2_T array, void *elem,
                    void *mapStruct);


/**********UArray2b_new********
 * About: This function initializes a T struct and assigns the given values
 *        such as width, height, size, and blocksize to the struct variables.
 * Inputs:
 * int height: integer value that represents number of rows in the 2D array  
 *          that the T struct holds
 * int width: integer value that represents number of columns in the 2D array 
 *          that the T struct holds
 * int size: integer value representing the size of an element in the 
 *          2D array that the T struct holds
 * int blocksize: integer value that represents the square root of number of
 *          cells in a block in the 2D array that the T struct holds
 * Return: a struct holding a 2D array with blocks and information related to  
 *         the structure
 * Expects
 * - row, col, elementSize to be greater than or equal to 0, throws cre
 *   otherwise
 * - blocksize to be greater than or equal to 1
 * - that it returns a non-null T2 struct
 * Note: The user should call UArray2b_free to avoid valgrind after calling 
 * this function
************************/
T UArray2b_new(int width, int height, int size, int blocksize) 
{
        /* asserts the expectation for col, row, size & blocksize */
        assert(width >= 0 && height >= 0 && size >= 0 && 
               blocksize >= minBlockSize);

        /* creating an instance of the struct T in malloc */
        T array2D;
        NEW(array2D);
        assert(array2D != NULL);

        /* initializing the attributes of array2D */
        array2D->rows = height;
        array2D->cols = width;
        array2D->elmSize = size;
        array2D->blockSize = blocksize;

        /* creating the UArray2_T */
        array2D->data = UArray2_new(ceil((float)width / (float)blocksize), 
                        ceil((float)height / (float)blocksize), 
                        sizeof(UArray_T));
        assert(array2D->data != NULL);

        /* filling the UArray2_T with UArray_Ts */
        UArray2_map_row_major(array2D->data, blockCreator, array2D);

        return array2D;
}

/**********blockCreator********
 * About: This function is used to initialize and assign a UArray structure to
 *        each cell of a given UArray2
 * Inputs:
 * int col: integer value that represents number of columns in the 2D array  
 *          that the T struct holds
 * int row: integer value that represents number of rows in the 2D array 
 *          that the T struct holds
 * UArray2_T array: a 2D array
 * void *elem: pointer to the current element in the cell being visited in the 
 *             2D array
 * void *p2: pointer to a UArray2b struct
 * Return: none
 * Expects
 * - array and p2 to be non-null, throws cre otherwise
 * Note: If this function is called by the user, it's user's responsibility to
 * free the UArray_T created in this function.
************************/
void blockCreator(int col, int row, UArray2_T array, void *elem, void *structT)
{
        (void) col;
        (void) row;

        /* asserting the expectations */
        assert(array != NULL && structT != NULL);

        /* initializing UArray with blockSize * blockSize as the size */
        T array2D = structT;
        int newBlockSize = array2D->blockSize * array2D->blockSize;
        UArray_T newBlock = UArray_new(newBlockSize, array2D->elmSize);
        assert(newBlock != NULL);

        /* placing the new UArray to the current cell */
        UArray_T *currentBlock = elem;
        *currentBlock = newBlock;
}

/**********UArray2b_new_64K_block********
 * About: This function initializes a T struct such that a block will fit into 
 *        64KB of RAM, and assigns the given values such as width, height, 
 *        size, and blocksize to the struct variables.
 * Inputs:
 * int height: integer value that represents number of rows in the 2D array  
 *          that the T struct holds
 * int width: integer value that represents number of columns in the 2D array 
 *          that the T struct holds
 * int size: integer value representing the size of an element in the 
 *          2D array that the T struct holds
 * Return: a struct holding a 2D array with blocks st. a block will fit into a
 *         64KB of RAM, and information related to the structure
 * Expects,
 * - row, col, elementSize to be greater than or equal to 0, throws cre 
 *   otherwise
 * - blocksize to be greater than or equal to 1, throws cre otherwise
 * - that it returns a non-null T2 struct
 * Note: The user should call UArray2b_free to avoid valgrind after calling 
 * this function
************************/
T UArray2b_new_64K_block(int width, int height, int size) 
{
        /* asserts the expectation for col, row, and elementSize to be > 0 */
        assert(width >= 0 && height >= 0 && size >= 0);

        /* creating an instance of the struct T in malloc */
        T array2D;
        NEW(array2D);
        assert(array2D != NULL);

        /* initializing the attributes of array2D */
        array2D->rows = height;
        array2D->cols = width;
        array2D->elmSize = size;

        /* if the block doesn't fit into 64KB, the size is assigned to 1 */
        if (size > blockMem * KB)
                array2D->blockSize = minBlockSize;
        else
                array2D->blockSize = sqrt(blockMem * KB / size);
        
        /* creating the UArray2_T */
        array2D->data = UArray2_new(
                        ceil((float)width / (float)array2D->blockSize), 
                        ceil((float)height / (float)array2D->blockSize), 
                        sizeof(UArray_T));
        
        assert(array2D->data != NULL);

        /* filling the UArray2_T with UArray_Ts */
        UArray2_map_row_major(array2D->data, blockCreator, array2D);

        return array2D;
}

/**********UArray2b_free********
 * About: This function frees the memory allocated to the UArray2_T and
 *        UArray_T instances located in UArray2_T
 * Inputs: 
 * T *array2b: address of the UArray2_T instance to store the content of data
 * Return: none
 * Expects
 * - that array2b is non-null, throws cre otherwise
************************/
void UArray2b_free(T *array2b) 
{
       
        assert(*array2b != NULL && array2b != NULL);

        UArray2_T arrayToFree = (*array2b)->data;

        /* traverse the UArray2 held by the struct and free blocks */
        UArray2_map_row_major(arrayToFree, blockFree, NULL);

        /* freeing the UArray2 held by the struct */
        UArray2_free(&arrayToFree);

        /* freeing the struct */
        FREE(*array2b);
}

/**********blockFree********
 * About: This function frees the memory allocated to the UArray_T instances 
 *        located in UArray2_T
 * Inputs: 
 * int col: integer value that represents number of columns in the 2D array  
 *          that the T struct holds
 * int row: integer value that represents number of rows in the 2D array 
 *          that the T struct holds
 * UArray2_T array: a 2D array
 * void *elem: pointer to an current element visited  
 * void *p2: unused pointer
 * Return: none
 * Expects
 * - that array is non-null, throws CRE otherwise
************************/
void blockFree(int col, int row, UArray2_T array, void *elem, void *p2) 
{
        assert(array != NULL);

        (void) col;
        (void) row;
        (void) p2;

        /* freeing the UArray_Ts that are located in UArray_2T */
        UArray_T *currentBlock = elem;
        UArray_free(currentBlock);
}

/**********UArray2b_width********
 * About: This function returns the width value (col number) of the 2D UArray2b
 * Inputs: 
 * T array2b: struct to store the content of the given data in 2D
 * Return: the width value of the 2D UArray2b 
 * Expects
 * - that array2b is non-null, throws cre otherwise
************************/
int UArray2b_width(T array2b) 
{
        assert(array2b != NULL);
        return array2b->cols;
}

/**********UArray2b_height********
 * About: This function returns the height value (row number) of the 2D 
 *        UArray2b
 * Inputs: 
 * T array2b: struct to store the content of the given data in 2D
 * Return: the height value of the 2D UArray2b 
 * Expects
 * - that array2b is non-null, throws cre otherwise
************************/
int UArray2b_height(T array2b) 
{
        assert(array2b != NULL);
        return array2b->rows;
}

/**********UArray2b_size********
 * About: This function returns the size of an element in the 2D UArray2b
 * Inputs: 
 * T array2b: struct to store the content of the given data in 2D 
 * Return: integer value that represents the size of an element held in the 2D
 *         UArray2b
 * Expects
 * - that array2b is non-null, throws cre otherwise
************************/
int UArray2b_size(T array2b) 
{
        assert(array2b != NULL);
        return array2b->elmSize;
}

/**********UArray2b_blocksize********
 * About: This function returns the the square root of number of cells in a 
 *        block in the 2D array that the T struct holds
 * Inputs: 
 * T array2b: struct to store the contents of the given data in 2D 
 * Return: integer value that represents the square root of number of cells in
 *        a block in the 2D array that the T struct holds
 * Expects
 * - that array2b is non-null, throws cre otherwise
************************/
int UArray2b_blocksize(T array2b) 
{
        assert(array2b != NULL);
        return array2b->blockSize;
}

/**********UArray2b_at********
 * About: This function returns a pointer to the element at the given row and
 *        col values in the array2b
 * Inputs:
 * T array2b: struct to store the content of the given data in 2D UArray
 * int row: row index value to find the element at
 * int col: col index value to find the element at
 * Return: a void pointer to the element at the given row and col indices
 * Expects
 * - that array is non-null, throws cre otherwise
 * - that row and col values are at least 0 and at most 
 *   (UArray2b_height(T array)-1) and (UArray2b_width(T array)-1) respectively
 *   throws cre otherwise
 * - that it returns an non-null pointer
************************/
void *UArray2b_at(T array2b, int column, int row) 
{
        assert(array2b != NULL);
        
        /* checks if the col and row values are valid */
        assert(column >= 0 && column < UArray2b_width(array2b));
        assert(row >= 0 && row < UArray2b_height(array2b));

        int blockSize = array2b->blockSize;

        /* stores the block that the desired value is stored at */
        UArray_T *block = UArray2_at(array2b->data, column / blockSize, 
                                     row / blockSize);
        
        /* returns the desired value within that block */
        return UArray_at(*block, blockSize * (row % blockSize) + 
                         column % blockSize);
}

/**********UArray2b_map********
 * About: This function traverses the 2D UArray2b structure such that every 
 *        cell in one block are visited and apply function is implemented on 
 *        each element before moving to another block 
 * Inputs:
 * T array2b: struct to store the content of the given data in 2D 
 * apply function: the function to be applied on all the elements of the array
 * cl pointer: client specific pointer input
 * Return: none
 * Expects
 * - non-null T array2b, throws cre otherwise
************************/
void UArray2b_map(T array2b, 
                 void apply(int col, int row, T array2b, void *elem, void *cl),
                 void *cl) 
{
        assert(array2b != NULL);

        /* initializing a struct to hold info to be used in mapping */
        struct mapParameters prm = {array2b, apply, cl};

        /* visiting each block held by the UArray2 */
        UArray2_map_row_major(array2b->data, insideBlockMap, &prm);

}

/**********insideBlockMap********
 * About: This function traverses the blocks (UArrayTs) such that every cell
 *        in one block are visited and apply function is implemented on each 
 *        element before moving to another block 
 * Inputs:
 * int col: integer value that represents number of columns in the 2D array  
 *          that the T struct holds
 * int row: integer value that represents number of rows in the 2D array 
 *          that the T struct holds
 * UArray2_T array: a 2D array
 * void *elem: pointer to an current element visited  
 * void *p2: mapParameters instance which holds the values T array2b, the
 *           apply function and the *cl pointer
 * Return: none
 * Expects
 * - non-null T array2b, throws cre otherwise
************************/
void insideBlockMap(int col, int row, UArray2_T array, void *elem, 
                    void *mapStruct) 
{
        assert(array != NULL && mapStruct != NULL);
        
        /* casting void parameters to be dereferenced */
        UArray_T *currentBlock = elem;
        struct mapParameters *prm = mapStruct;

        /* recording block size, number of rows, and number of cols in total */
        int blockSize = (prm->array2b)->blockSize;
        int colSize = (prm->array2b)->cols;
        int rowSize = (prm->array2b)->rows;
        
        /* visiting all cells in the block */
        for (int elt = 0; elt < UArray_length(*currentBlock); elt++) {
                /* calculating col,row indices in 2D array for current elt */
                int col2b = col * blockSize + elt % blockSize;
                int row2b = row * blockSize + elt / blockSize;

                /* apply given function if the cell is not unused */
                if (col2b < colSize && row2b < rowSize) {
                        prm->apply(col2b, row2b, prm->array2b, 
                           UArray_at(*currentBlock, elt), prm->cl);
                }
        }
}

#undef T
#undef KB
#undef blockMem
#undef minBlockSize