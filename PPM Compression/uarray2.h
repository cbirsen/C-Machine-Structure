/*
 *      uarray2.h
 *      by Loc Mai (lmai02), Ethan Goldman (gethan01)
 *      Used for PPM Compression by Cansu Birsen and Ethan Goldman
 *
 *      This program contains the header file for the UArray2_T implementation.
 *      It stores all the function defintions used in the implementation of
 *      the UArray2 type.
 */

#ifndef UARRAY2_INCLUDED
#define UARRAY2_INCLUDED

#define T UArray2_T
typedef struct T *T;

extern T UArray2_new(int width, int height, int element_size);
extern void UArray2_free(T *uarray2_pt);
extern int UArray2_width(T uarray2);
extern int UArray2_height(T uarray2);
extern int UArray2_size(T uarray2);
void *UArray2_at(T uarray2, int col, int row);
void UArray2_map_col_major(T uarray2,
                           void apply(int col, int row, UArray2_T uarray2_a,
                                      void *elem, void *cl_a),
                           void *cl);
void UArray2_map_row_major(T uarray2,
                           void apply(int col, int row, UArray2_T uarray2_a,
                                      void *elem, void *cl_a),
                           void *cl);
                           
#undef T
#endif
