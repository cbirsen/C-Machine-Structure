/*
 *      emulator.c
 *      by Cansu Birsen (cbirse01), Ayse Idil Kolabas (akolab01)
 *      November 12, 2023
 *      UM Emulator
 *
 *      This is the driver module for the um program. Expects a file with the
 *      .um extension to read and implement instructions using the operations
 *      module. If the file cannot be opened properly or not supplied, returns
 *      with EXIT_FAILURE.
 */

#include <stdio.h>
#include "operations.h"

FILE *open_or_die(int argc, char *argv[]);

int main(int argc, char *argv[]) {
        /* open input file */
        FILE *fp = open_or_die(argc, argv);
        if (fp == NULL) {
                return EXIT_FAILURE;
        }

        /* Calls operations module to implement the instructions */
        initiate_program(fp);

        fclose(fp);

        return 0;
}


/**********open_or_die********
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
FILE *open_or_die(int argc, char *argv[]) {   
        /* check if correct number of arguments is supplied */
        if (argc != 2) {
                fprintf(stderr, "Incorrect number of arguments.\n");
                return NULL;
        }

        FILE *fp; 
        fp = fopen(argv[1], "r");
        
        return fp;
}