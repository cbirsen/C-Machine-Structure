/*
 *      uarray2.c
 *      by Loc Mai (lmai02), Ethan Goldman (gethan01)
 *      Used for PPM Compression by Cansu Birsen and Ethan Goldman
 *
 *      This program contains the implementation of the UArray2 interface.
 *      It stores all the functional components used in the interface of
 *      the UArray2 type.
 */

/* Custom Header Files */
#include "uarray2.h"

/* Hanson Data Structures */
#include "uarray.h"
#include <assert.h>

/* Standard C Libraries */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define T UArray2_T
struct T {
        UArray_T uarrays;
        int width;
        int height;
        int element_size;
};

/******************************* UArray2_new *********************************
 *
 * Create a new UArray2 with the specified width, height, and element size
 *
 * Inputs:
 *              int width: the width of the new Bit2
 *              int height: the height of the new Bit2
 *              int element_size: the size in bytes of the element UArray2 is
 *                                going to store
 * Return:
 *              the new UArray2
 * Expects:
 *              width and height to be greater than or equal to 0
 *              no error in mallocing space for struct UArray2_T
 * Notes:
 *              will CRE if expectations fail
 *              it is the resposibility of the user to free the UArray2
 *                      using the UArray2_free funciton
 *
 *****************************************************************************/
extern T UArray2_new(int width, int height, int element_size)
{
        /* Check for expectations */
        assert(width >= 0);
        assert(height >= 0);
        assert(element_size > 0);

        /* Allocate memory for new UArray2 */
        T new_uarray2 = malloc(sizeof(struct T));
        assert(new_uarray2 != NULL);

        new_uarray2->uarrays = UArray_new(height, sizeof(UArray_T));
        new_uarray2->width = width;
        new_uarray2->height = height;
        new_uarray2->element_size = element_size;

        /* Set ptrs to uarrays to new UArray2 */
        for (int i = 0; i < height; i++) {
                UArray_T temp_uarrays = UArray_new(width, element_size);
                UArray_T *temp_uarrays_pt = UArray_at(new_uarray2->uarrays, i);
                *temp_uarrays_pt = temp_uarrays;
        }

        return new_uarray2;

}

/****************************** UArray2_free *********************************
 *
 * Free all memory on the heap associated with UArray2_T
 *
 * Inputs:
 *              UArray2 *uarray2_pt: a pointer to the UArray2 that is to 
 *                                   be freed
 * Return:
 *              none
 * Expects:
 *              uarray2_pt not to be NULL
 *              the UArray2 pointed to by uarray2_pt not to be NULL
 * Notes:
 *              will CRE if expectations fail
 *              it is the responsibility of the user to guarantee that they
 *                      wish to relinquish ownership of the Bit2 that they
 *                      are freeing
 *
 *****************************************************************************/
extern void UArray2_free(T *uarray2_pt)
{
        /* Check expectations */
        assert(uarray2_pt != NULL);
        assert(*uarray2_pt != NULL);

        /* Clean out each UArray in UArray2 */
        UArray2_T actual_uarray2 = (void *) *uarray2_pt;
        for (int row = 0; row < UArray2_height(actual_uarray2); row++) {
                UArray_T *temp_uarrays = UArray_at(actual_uarray2->uarrays, 
                                                   row);
                UArray_free((void *) temp_uarrays);
        }
        UArray_free(&(actual_uarray2->uarrays));

        /* Free actual UArray2 */
        free((void *) *uarray2_pt);
        *uarray2_pt = NULL;

        return;
}

/****************************** UArray2_width ********************************
 *
 * Returns the width (i.e. the number of columns) of the specified UArray2
 *
 * Inputs:
 *              UArray2 uarray2: the UArray2 of which to find the width
 * Return:
 *              the width of the UArray2
 * Expects:
 *              uarray2 not to be NULL
 * Notes:
 *              will CRE if expectations fail
 *
 *****************************************************************************/
extern int UArray2_width(T uarray2)
{
        assert(uarray2 != NULL);

        return uarray2->width;;
}

/****************************** UArray2_height ********************************
 *
 * Returns the height (i.e. the number of rows) of the specified UArray2
 *
 * Inputs:
 *              UArray2 uarray2: the UArray2 of which to find the height
 * Return:
 *              the height of the UArray2
 * Expects:
 *              uarray2 not to be NULL
 * Notes:
 *              will CRE if expectations fail
 *
 *****************************************************************************/
extern int UArray2_height(T uarray2)
{
        assert(uarray2 != NULL);

        return uarray2->height;
}

/******************************* UArray2_size ********************************
 *
 * Returns the element size of the given UArray2
 *
 * Inputs:
 *              UArray2 uarray2: the UArray2 of which to get the element size
 *                               of
 * Return:
 *              the element size of the UArray2
 * Expects:
 *              uarray2 not to be NULL
 * Notes:
 *              will CRE if expectations fail
 *
 *****************************************************************************/
extern int UArray2_size(T uarray2) 
{
        assert(uarray2 != NULL);

        return uarray2->element_size;
}

/******************************* UArray2_get *********************************
 *
 * Get the associated element at the specificed slot indicated by the given 
 * column and row
 *
 * Inputs:
 *              UArray2 uarray2: the UArray2 to get the element from
 *              int col: the column of the slot where the element will be 
 *                       retrieved from
 *              int row: the row of the slot where the element will be
 *                       retrieved from
 * Return:
 *              an element in the UArray2
 * Expects:
 *              bit2 not to be NULL
 *              col to be greater than or equal to 0
 *              row to be greater than or equal to 0
 * Notes:
 *              will CRE if expectations fail
 *
 *****************************************************************************/
void *UArray2_at(T uarray2, int col, int row)
{
        /* Check expectations */
        assert(uarray2 != NULL);
        assert(col > -1);
        assert(row > -1);
        assert(UArray2_height(uarray2) != 0);
        assert(UArray2_width(uarray2) != 0);

        /* Check slot */
        UArray_T *uarray2_row = UArray_at(uarray2->uarrays, row);
        assert(uarray2_row != NULL);

        void *value = UArray_at(*uarray2_row, col);
        return value;
}

/**************************** UArray2_map_col_major **************************
 *
 * Maps through the UArray2 by columns and applies the supplied apply() 
 * function to each element in the UArray2
 *
 * Inputs:
 *              UArray2 uarray2: the UArray2 to be mapped through
 *              void apply(): the function to apply to each element
 *                      int col: the current column of the apply function
 *                      int row: the current row of the apply function
 *                      UArray2 uarray2_a: the UArray2 to be mapped through
 *                      int *elem: pointer to the current element at (col, row)
 *                      void *cl_a: pointer to the closure variable
 *              void *cl: pointer to the closure variable
 * Return:
 *              none
 * Expects:
 *              uarray2 not to be NULL
 *              the apply function to not be a NULL function
 * Notes:
 *              will CRE if expectations fail
 *
 *****************************************************************************/
void UArray2_map_col_major(T uarray2,
                           void apply(int col, int row, UArray2_T uarray2_a,
                                      void *elem, void *cl_a),
                           void *cl)
{
        /* Check expectations */
        assert(uarray2 != NULL);
        assert(apply != NULL);
        
        /* Go through every column */
        for (int col = 0; col < UArray2_width(uarray2); col++) {
                /* Go through every row */
                for (int row = 0; row < UArray2_height(uarray2); row++) {
                        void *curr_elem = UArray2_at(uarray2, col, row);
                        apply(col, row, uarray2, curr_elem, cl);
                }
        }
        return;
}

/**************************** UArray2_map_row_major **************************
 *
 * Maps through the UArray2 by rows and applies the supplied apply() function
 * to each element in the UArray2
 *
 * Inputs:
 *              UArray2 uarray2: the UArray2 to be mapped through
 *              void apply(): the function to apply to each element
 *                      int col: the current column of the apply function
 *                      int row: the current row of the apply function
 *                      UArray2 uarray2_a: the UArray2 to be mapped through
 *                      int *elem: pointer to the current element at (col, row)
 *                      void *cl_a: pointer to the closure variable
 *              void *cl: pointer to the closure variable
 * Return:
 *              none
 * Expects:
 *              uarray2 not to be NULL
 *              the apply function to not be a NULL function
 * Notes:
 *              will CRE if expectations fail
 *
 *****************************************************************************/
void UArray2_map_row_major(T uarray2,
                           void apply(int col, int row, UArray2_T uarray2_a,
                                      void *elem, void *cl_a),
                           void *cl)
{
        /* Check expectations */
        assert(uarray2 != NULL);
        assert(apply != NULL);
        
        /* Go through every row */
        for (int row = 0; row < UArray2_height(uarray2); row++) {
                for (int col = 0; col < UArray2_width(uarray2); col++) {
                        void *curr_elem = UArray2_at(uarray2, col, row);
                        apply(col, row, uarray2, curr_elem, cl);
                }
        }
        return;
}
