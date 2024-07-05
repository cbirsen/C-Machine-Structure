/*
 *      bitpack.c
 *      by Cansu Birsen (cbirse01), Ethan Goldman (gethan01)
 *      october 24, 2023
 *      PPM Compression
 *
 *      Implementation for the bitpack.h file. Functions in this file are used
 *      to create and read from bitpacked words
 */

/* Standard C Libraries */
#include <stdio.h>

/* CS40 Libraries */
#include <bitpack.h>

/* Hanson Libraries */
#include <assert.h>

/* Size of the codewords that the bitpack class can perform operations on */
static const unsigned BITSIZE = 64;

/* Error message for bitpack overflow */
Except_T Bitpack_Overflow = { "Overflow packing bits" };


/**************************** Bitpack_fitsu ***********************************
 *
 * Tells whether the unsigned argument n can be represented in width bits
 *
 * Inputs:      uint64_t n: Unsigned value being checked whether it can 
 *                          be represented in wdith bits
 *              unsigned width: The number of bits to represent n
 * Return:
 *              true if n can be represented in width bits, false otherwise
 * Expects:
 *              none
 * Notes:
 *              none
 *
 *****************************************************************************/
bool Bitpack_fitsu(uint64_t n, unsigned width) 
{
        uint64_t max_val = ~((uint64_t)0);
        
        /* Set the max value that can be represented using width bits */
        max_val = max_val >> (BITSIZE - width);
        if (n <= max_val) {
                return true;
        }
        return false;
}

/*************************** Bitpack_fitss ************************************
 *
 * Tells whether the signed argument n can be represented in width bits
 *
 * Inputs:      int64_t n: Signed value being checked whether it can 
 *                          be represented in wdith bits
 *              unsigned width: The number of bits to represent n
 * Return:
 *              true if n can be represented in width bits, false otherwise
 * Expects:
 *              none
 * Notes:
 *              none
 *
 *****************************************************************************/
bool Bitpack_fitss(int64_t n, unsigned width)
{
        /* Set max and min values that can be represented with width bits */
        int64_t max_val = ((uint64_t)1 << (width - 1)) - (uint64_t)1;
        int64_t min_val = ~max_val;
        if (min_val <= n && n <= max_val) {
                return true;
        }
        return false;
}

/******************************* Bitpack_getu *********************************
 *
 * Unsigned version of get. Gets the subword of width width at lowest
 * significant bit lsb
 *
 * Inputs:
 *              uint64_t word: the codeword where the subword is stored
 *              unsigned width: the width of the subword
 *              unsigned lsb: the lowest significant bit of the subword
 * Return:
 *              the uint64_t representation of the subword
 * Expects:
 *              0 <= width <= 64
 *              width + lsb <= 64
 * Notes:
 *              will CRE if expectations fail
 *              if width = 0, return value is 0
 *
 *****************************************************************************/
uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb) 
{
        assert(width <= 64 && (width + lsb) <= 64);
        
        if(width == 0) {
                return (uint64_t)0;
        }
        
        /* Create mask to get subword */
        uint64_t comp_zero = ~((uint64_t)0);
        uint64_t make_width = comp_zero >> (BITSIZE - width);
        uint64_t mask = make_width << lsb;
        
        /* Use mask to get subword */
        uint64_t sub_word = (word & mask) >> lsb;
        
        return sub_word;
}

/******************************* Bitpack_gets *********************************
 *
 * Signed version of get. Gets the subword of width width at lowest
 * significant bit lsb
 *
 * Inputs:
 *              uint64_t word: the codeword where the subword is stored
 *              unsigned width: the width of the subword
 *              unsigned lsb: the lowest significant bit of the subword
 * Return:
 *              the int64_t representation of the subword
 * Expects:
 *              0 <= width <= 64
 *              width + lsb <= 64
 * Notes:
 *              will CRE if expectations fail
 *              if width = 0, return value is 0
 *
 *****************************************************************************/
int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb) 
{
        /* Use the unsigned version of get to get the binary digits of the word
           out of the word*/
        uint64_t sub_word_positive = Bitpack_getu(word, width, lsb);
        
        uint64_t comp_zero = ~((uint64_t)0);
        uint64_t make_width = comp_zero >> (BITSIZE - width);

        uint64_t trailing_one = (uint64_t)1;
        uint64_t sub_word_leading_one = trailing_one << (width - 1);

        /* If the subword is negative, calcalculate the negative signed 64 bit
           version of the subword and return it */
        if((sub_word_leading_one & sub_word_positive) != 0) {
                int64_t sub_word_negative = sub_word_positive - 
                                            (make_width + 1);
                return sub_word_negative;
        } 

        /* Otherwise, return the positive signed version of the subword */
        return sub_word_positive;
}

/**************************** Bitpack_newu ***********************************
 *
 * Updates word such that the field of width "width" with least significant bit
 * at lsb will have been replaced by a width-bit representation of unsigned 
 * value.
 *
 * Inputs:      uint64_t word: Unsigned word to be updated
 *              unsigned width: The number of bits to update
 *              unsigned lsb: the least significant bit location of the field 
 *                            to be updated
 *              uint64_t value: the value that will be substituted in the word
 * Return:
 *              an updated version of word
 * Expects:
 *              width to be less than BITSIZE
 *              sum of width and lsb to be less than BITSIZE
 *              value to be represented in width bits
 * Notes:
 *              will CRE if width or the sum of width and lsb is more than 
 *                       BITSIZE
 *              will raise Bitpack_Overflow if value cannot be represented in
 *              width many bits
 *
 *****************************************************************************/
uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb, 
                      uint64_t value) 
{
        assert(width <= BITSIZE && (width + lsb) <= BITSIZE);

        /* Check if value can be represented in width bits */
        if(!Bitpack_fitsu(value, width)) {
                RAISE(Bitpack_Overflow);
        }

        /* Create mask with 0's at the field to update and 1's for the rest */
        uint64_t comp_zero = ~((uint64_t)0);
        uint64_t make_width = comp_zero >> (BITSIZE - width);
        uint64_t mask_temp = make_width << lsb;
        uint64_t mask = ~mask_temp;

        /* Clear the field to be updated in the word by making it all 0's */
        uint64_t clear_subword = mask & word;

        /* Shift value so that it aligns with the field in word */
        uint64_t value_shifted = value << lsb;
        
        /* Merge cleared word and shifted value */
        return clear_subword | value_shifted;
}

/**************************** Bitpack_news ***********************************
 *
 * Updates word such that the field of width "width" with least significant bit
 * at lsb will have been replaced by a width-bit representation of signed 
 * value.
 *
 * Inputs:      uint64_t word: Unsigned word to be updated
 *              unsigned width: The number of bits to update
 *              unsigned lsb: the least significant bit location of the field 
 *                            to be updated
 *              uint64_t value: the value that will be substituted in the word
 * Return:
 *              an updated version of word
 * Expects:
 *              width to be less than BITSIZE
 *              sum of width and lsb to be less than BITSIZE
 *              value to be represented in width bits
 * Notes:
 *              will CRE if width or the sum of width and lsb is more than 
 *                       BITSIZE
 *              will raise Bitpack_Overflow if value cannot be represented in
 *              width many bits
 *
 *****************************************************************************/
uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned lsb, 
                      int64_t value)
{
        assert(width <= BITSIZE && (width + lsb) <= BITSIZE);

        /* Check if value can be represented in width bits */
        if(!Bitpack_fitss(value, width)) {
                RAISE(Bitpack_Overflow);
        }

        uint64_t u_value = value;

        /* Check if value is negative */
        if(value < 0) {
                /* Calculate uint version of value represented in width bits */
                int64_t two_to_pow_width = (int64_t)1 << width;
                u_value = (uint64_t)(two_to_pow_width + value);
        }

        /* Treat value like an unsigned integer */
        return Bitpack_newu(word, width, lsb, u_value); 
}
