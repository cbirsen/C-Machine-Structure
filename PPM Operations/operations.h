/*
 *     operations.h
 *     by Doga Kilinc (dkilin01) & Cansu Birsen (cbirse01), October 10
 *     PPM Operations
 *
 *     About: This file is used to read the given ppm file with the choosen
 *            method (row, col, or block); apply the choosen operations
 *            (rotations, flip, transpose, or time); and output the final image
 *            to the standard output. If the time operation is choosen, the 
 *            program prints out the necessary information to the timing file.
 */

#ifndef OPERATIONS_INCLUDED
#define OPERATIONS_INCLUDED

#include <string.h>
#include <stdio.h>
#include "assert.h"
#include "a2methods.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "pnm.h"
#include "cputiming.h"

void operationHandler(FILE *fp, A2Methods_T methods, int rotation, 
                     A2Methods_mapfun *map, char *time_file_name, 
                     char *inputFile);
void timerStarter(CPUTime_T timer, char *time_file_name);
void timerStopper(CPUTime_T timer, char *time_file_name, char *operation,
                  int pixelNum, char *inputFile, int width, int height);
void timePrinter(double time_used, int pixelNum, char *time_file_name, 
                 char *operation, char *inputFile, int width, int height);
void rotateApply(int col, int row, A2Methods_UArray2 array, void *elem, 
                 void *rotateStruct);
void rotate(A2Methods_T methods, Pnm_ppm image, A2Methods_mapfun *map, 
            int newWidth, int newHeight, int angle, CPUTime_T timer, 
            char *time_file_name, char *inputFile);


#endif