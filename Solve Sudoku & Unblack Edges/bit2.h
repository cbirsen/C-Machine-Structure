/*
 *     bit2.h
 *     by Doga Kilinc (dkilin01) & Cansu Birsen (cbirse01), September 28
 *     Solve Sudoku & Unblack Edges
 *
 *     About: This file can be used to create a 2D vector where a client can
 *     store the bit data in. It also has functions that helps theclient to 
 *     get the width, height, and element size information about the vector,
 *     traverse the vector in row major and column major order, and access
 *     to an element at a certain location.
 *     
 */

#ifndef BIT2_INCLUDED
#define BIT2_INCLUDED

#define T2 Bit2_T
typedef struct T2 *T2;

#include <bit.h>

extern T2 Bit2_new(int col, int row);
extern int Bit2_width(T2 array);
extern int Bit2_height(T2 array);
extern int Bit2_put(T2 array, int col, int row, int bit);
extern int Bit2_get(T2 array, int col, int row);
extern void Bit2_map_row_major(T2 array, void apply(int col, int row, T2 array,
                               int bit, void *p1), void *cl);
extern void Bit2_map_col_major(T2 array, void apply(int col, int row, T2 array,
                               int bit, void *p1), void *cl);
extern void Bit2_free(T2 *array);

#undef T
#endif