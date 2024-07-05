/*
 *      read_and_write.h
 *      by Cansu Birsen (cbirse01), Ethan Goldman (gethan01)
 *      october 23, 2023
 *      PPM Compression
 *
 *      Interface for the read_and_write.c file. Functions in this file are
 *      used to convert between ppm files and Pnm_ppm structs
 */

#ifndef READ_AND_WRITE_INCLUDED
#define READ_AND_WRITE_INCLUDED

/* Standard C Libraries */
#include <stdlib.h>
#include <stdio.h>

/* CS40 Libraries */
#include <pnm.h>

extern Pnm_ppm read_ppm(FILE *input);
extern void write_ppm(Pnm_ppm image);

#endif
