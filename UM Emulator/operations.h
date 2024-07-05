/*
 *      operations.h
 *      by Cansu Birsen (cbirse01), Ayse Idil Kolabas (akolab01)
 *      November 12, 2023
 *      UM Emulator
 *
 *      Interface for the operations.c file. Functions in this file are
 *      used to load in a program and perform the operation specified in the 
 *      program, such as addition, multiplication, division, nand, output, 
 *      input, and so on.
 */



#ifndef OPERATIONS_INCLUDED
#define OPERATIONS_INCLUDED

/* Standard C Libraries */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>


extern void initiate_program(FILE *fp);

#endif