/*
 *     openOrDie.h
 *     by Doga Kilinc (dkilin01) & Cansu Birsen (cbirse01), September 18
 *     Solve Sudoku & Unblack Edges
 *
 *     About: This file holds the openOrDie function that checks if the
 *     program is started with the correct number of arguments (1 or 2) and
 *     tries to opening a stream from the given arguments or from stdin.
 *     
 */

#ifndef OPENORDIE_INCLUDED
#define OPENORDIE_INCLUDED

#include <stdio.h>
#include <stdlib.h>

extern FILE *openOrDie(int argc, char *argv[]);


#endif