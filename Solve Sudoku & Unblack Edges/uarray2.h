/*
 *     uarray2.h
 *     by Doga Kilinc (dkilin01) & Cansu Birsen (cbirse01), September 28
 *     Solve Sudoku & Unblack Edges
 *
 *     About: This file can be used to create a 2D array where a client can
 *     store the data. It also has functions that helps the client to get the 
 *     width, height, and element size information about the array, traverse 
 *     the array in row major and column major order and access to an element 
 *     at a certain location.
 *     
 */

#ifndef UARRAY2_INCLUDED
#define UARRAY2_INCLUDED

#define T2 UArray2_T
typedef struct T2 *T2;

extern T2 UArray2_new(int col, int row, int elementSize);
extern int UArray2_width(T2 array);
extern int UArray2_height(T2 array);
extern int UArray2_size(T2 array);
extern void *UArray2_at(T2 array, int col, int row);
extern void UArray2_map_row_major(T2 array, void apply(int col, 
                                  int row, T2 array, void *p1, 
                                  void *p2), void *cl);
extern void UArray2_map_col_major(T2 array, void apply(int col, 
                                  int row, T2 array, void *p1, 
                                  void *p2), void *cl);
extern void UArray2_free(T2 *array);

#undef T2
#endif