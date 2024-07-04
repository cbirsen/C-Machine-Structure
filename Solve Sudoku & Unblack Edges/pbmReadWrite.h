/*
 *     pbmReadWrite.h
 *     by Doga Kilinc (dkilin01) & Cansu Birsen (cbirse01), September 18
 *     Solve Sudoku & Unblack Edges
 *
 *     About: This program checks if a given file is in the correct pbm format.
 *            If it is, the program creates a 2D vector to store the data.
 *            The program can also be used to print out the cleared data in
 *            the 2D vector to an output file.
 *     
 */

#ifndef PBMREADWRITE_INCLUDED
#define PBMREADWRITE_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <bit2.h>

Bit2_T pbmRead (FILE *inputfp);
void pbmWrite(FILE *outputfp, Bit2_T bitmap);
void arrayFiller(int col, int row, Bit2_T array, int bit, void *p1);
void arrayPrinter(int col, int row, Bit2_T array, int bit, void *p1);

#endif