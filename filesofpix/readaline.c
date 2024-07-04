/*
 *     readaline.c
 *     by Doga Kilinc (dkilin01) & Cansu Birsen (cbirse01), September 18
 *     Filesofpix
 *
 *     About: This file holds the readaline function which accepts pointers to
 *     a file to read from and a char array to store the content of a single 
 *     line. The function returns the number of bytes needed to store the 
 *     characters in a single line.
 */

#include <stdio.h>
#include <stdlib.h>
#include <readaline.h>
#include <assert.h>

/**********readaline********
 *
 * About: reads a single line from the input file and stores its contents to 
 *        datapp
 * Inputs:
 *		FILE *inputfd: pointer to the input file to read the lines from
 *      char **datapp: pointer to pointer to a char array to store the contents
 *      of one line from the file
 * Return: number of bytes used to store one line from the file
 * Notes
 *      will CRE, if either of the supllied arguments is null
 *      may CRE if malloc fails
 *      it's the callers responsibilty to free datapp unless there are no lines
 *      to read
 ************************/
size_t readaline(FILE *inputfd, char **datapp) {
        /* Handle if the supplied arguments is NULL */
        assert(datapp != NULL && inputfd != NULL); 
        /* creating datapp assuming max number of characteres is the limit */
        size_t limit = 1001;
        char *line = (char*)malloc(sizeof(char) * limit);
        assert(line != NULL); /* Handle memory allocation fail */
        
        char c; /* reading the lines character by character */
        size_t numBytes = 0;
        while ((c = fgetc(inputfd)) != EOF)  {
                if (numBytes == limit) {
                        limit = limit * 3;
                        line = realloc(line, sizeof(char) * limit);
                        assert(line != NULL); 
                }
                if (c == '\n') { /* if c is '\n', the line ends here */
                        line[numBytes] = '\n';
                        numBytes++;
                        *datapp = line;
                        return numBytes;
                }
                line[numBytes] = (char)c;
                numBytes++;
        }
        *datapp = line;
        if (numBytes == 0) /* freeing line when there are no  lines to read */
                free(line);
        return numBytes;
}



