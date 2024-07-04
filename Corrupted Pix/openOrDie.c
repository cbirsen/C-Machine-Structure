/*
 *     openOrDie.c
 *     by Doga Kilinc (dkilin01) & Cansu Birsen (cbirse01), September 18
 *     Filesofpix
 *
 *     About: This file holds the openOrDie function that checks if the
 *     program is started with the correct number of arguments (1 or 2) and
 *     tries to opening a file from the given arguments or from stdin.
 *     
 */

#include <stdio.h>
#include <stdlib.h>
#include <openOrDie.h>
#include <assert.h>

/**********openOrDie********
 *
 * About: checks if the program is started with the correct number of
 *        arguments. Opens the file either from argv or accept content from 
 *        stdin.
 * Inputs:
 *      int argc: number of given arguments to start the program
 *      char *argv: an array that stores the arguments
 * Expects:
 *      If the given arguments do not equal 1 or 2, throws CRE 
 *      If the file cannot be opened, throws CRE 
 *
 ************************/
FILE *openOrDie(int argc, char *argv[]) {   
        /* check if correct number of arguments is supplied */
        assert(argc == 1 || argc == 2);

        FILE *fp; 
        /* opening the file from given arguments */
        if (argc == 2) {
                fp = fopen(argv[1], "r");
        }
        /* receiving the corrupted image content from standard input */
        else {
                fp = stdin;
        }
        
        /* checking if the file is opened correctly (runtime error) */
        assert(fp != NULL);
        return fp;
}