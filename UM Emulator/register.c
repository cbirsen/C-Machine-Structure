/*
 *      register.c
 *      by Cansu Birsen (cbirse01), Ayse Idil Kolabas (akolab01)
 *      November 12, 2023
 *      UM Emulator
 *
 *      Implementation for the register.h file. Functions in this file
 *      are used to create, manipulate, and free uarrays that are used
 *      to represent the 8 32-bit registers.
 */

/* Hanson Libraries */
#include <assert.h>
#include <stdint.h>
#include <except.h>

/* Custom .h files */
#include "register.h"

/* number of registers */
#define REG_SIZE 8

/* About: This exception is raised when client attempts access
 *        to an index that is out of bounds.
 */
Except_T Bad_Bounds_Register = { "Index out of bounds" };

/*********************** register_new *******************************
 *
 * used when initiating the program, to create the 8 32-bit registers
 *
 * Inputs:
 *              n/a
 * Return:
 *              UArray_T: uarray of length 8, with all values set to 0
 * Expects:
 *              n/a
 * Notes:
 *              CRE if space not allocated properly for reg.
 *              it's the user's responsibility to call register_free 
 *              to free each register and the entire uarray
 *
 ********************************************************************/
UArray_T register_new() 
{
        /* creates new uarray to represent registers */
        UArray_T reg = UArray_new(REG_SIZE, sizeof(uint32_t));
        assert(reg != NULL);
        
        /* initializes each register with value 0 */
        for (int i = 0; i < REG_SIZE; i++) { 
                *(uint32_t *)UArray_at(reg, i) = (uint32_t) 0;
        }
        
        return reg;
}

/*********************** value_at **************************************
 *
 * used when retrieving value at register with provided index
 *
 * Inputs:
 *              UArray_T reg: uarray representing the 8 registers
 *              int index: represents register index to retrieve value from
 * Return:
 *              *void: value at register when dereferenced with uint32_t
 * Expects:
 *              reg is not null and index is in bounds
 * Notes:
 *              CRE if reg is null or URE if index out of bounds with 
 *              Bad_Bounds_Register exception
 *
 **********************************************************************/
void *value_at(UArray_T reg, int index)
{
        /* checks for requirements */
        assert(reg != NULL);
        
        if (index >=  UArray_length(reg) || index < 0) {
                RAISE(Bad_Bounds_Register);
        }

        /* gets pointer to element from specified register */
        return UArray_at(reg, index);
}

/*********************** register_free **************************************
 *
 * frees all register related space
 *
 * Inputs:
 *              UArray_T *reg: pointer to uarray representing the 8 registers
 * Return:
 *              n/a
 * Expects:
 *              reg is not null and pointer to reg is not null
 * Notes:
 *              CRE if expectations fail
 *
 **********************************************************************/
void register_free(UArray_T *reg)
{
        /* checks for expectations */
        assert(reg != NULL);
        assert(*reg != NULL);

        /* frees uarray representing registers */
        UArray_free(reg);
}

#undef REG_SIZE