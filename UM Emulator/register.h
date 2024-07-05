/*
 *      register.h
 *      by Cansu Birsen (cbirse01), Ayse Idil Kolabas (akolab01)
 *      November 12, 2023
 *      UM Emulator
 *
 *      Interface for the register.c file. Functions in this file are
 *      used to create a register structure with 8 32-bit containers, retrieve
 *      the value stored at a register, and free the structure. 
 * 
 */


#ifndef REGISTER_INCLUDED
#define REGISTER_INCLUDED

/* Standard C Libraries */
#include <stdlib.h>
#include <stdio.h>

/* Hanson Libraries */
#include <uarray.h>

extern UArray_T register_new();
extern void *value_at(UArray_T reg, int index);
extern void register_free(UArray_T *reg);

#endif