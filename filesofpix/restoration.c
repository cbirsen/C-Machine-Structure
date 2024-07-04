/*
 *     restoration.c
 *     by Doga Kilinc (dkilin01) & Cansu Birsen (cbirse01), September 18
 *     Filesofpix
 *
 *     About: This program is used to restore plain P2 pgm files corrupted with
 *     an infusion pattern and repeated lines. It separates patterns and int
 *     values in each line with the help of readaline function. Then, compares
 *     patterns to find the non-unique one, which is the true infuion pattern.
 *     After finding that, the program converts the original lines to P5 format
 *     and outputs the result.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readaline.h>
#include <stdbool.h>
#include <seq.h>
#include <table.h>
#include <atom.h>
#include <pnmrdr.h>
#include <assert.h>
#include <openOrDie.h>

/* function declarations */
//FILE *openOrDie(int argc, char *argv[]); 
const char *lineSeparater(char *datapp, Seq_T intSeq, char *charArray);
bool tableHandler(Table_T patternMap, const char *word, Seq_T intSeq, 
                  Seq_T finalSeq);
void tableFree(Table_T patternMap);
static void freeIntAndSequence(const void *key, void **value, void *cl);
void fileReader(char *datapp, bool *discovered, Table_T patternMap, 
                Seq_T finalSeq, const char **truePattern);
void outputP5(Seq_T finalSeq);

/**********main********
 *
 * About: Initializes the data structures needed for the program such as 
 *        patternMap, finalSeq, discovered, truePattern, and datapp. Calls
 *        readaline to read every line in the file and calls the appropriate 
 *        functions to recover the pgm file.
 * Inputs:
 *      int argc: number of given arguments to start the program
 *      char *argv: an array that stores the arguments
 * Return: EXIT_SUCCESS if the program ends succesfully
 ************************/
int main(int argc, char *argv[]) {
        /* trying to open the file */
        FILE *fp = openOrDie(argc, argv);
        
        /* creating the table, and final seq to int seqs from original lines */
        Table_T patternMap = Table_new(1000, NULL, NULL);
        Seq_T finalSeq = Seq_new(1000);
        /* bool to see if pattern is found & char to store infusion pattern */
        bool discovered = false;
        const char *truePattern;

        /* retrieving a line from the file until EOF */
        char *datapp;
        while (readaline(fp, &datapp) != 0) {
                fileReader(datapp, &discovered, patternMap, finalSeq, 
                           &truePattern);
                free(datapp);
        }
        /* convert cleaned file to P5 and output the result */
        outputP5(finalSeq);

        /* freeing finalSeq and the sequences in it */
        for (int i = 0; i < Seq_length(finalSeq); i++) {
                void *intSeq_ptr = Seq_remlo(finalSeq);
                freeIntAndSequence(NULL, &intSeq_ptr, NULL);
        }
        Seq_free(&finalSeq);
        fclose(fp);
        return EXIT_SUCCESS;
}

/**********outputP5********
 *
 * About: Converts P2 formatted values to P5 using ASCII conversion and outputs
 *        results to stdout
 * Inputs:
 *		Seq_T finalSeq: sequence that stores the values from the original
 *      lines
 * Return: none
 * Expects
 *      Seq_T finalSeq to be nonnull
 ************************/
void outputP5(Seq_T finalSeq) {

        /* obtaining the size info of the recovered file in P5 format */
        int width = Seq_length(Seq_get(finalSeq, 0));
        int height = Seq_length(finalSeq);

        /* printing the header information to the output */
        fprintf(stdout, "P5\n%d %d\n%d\n", width, height, 255);
        
        /* converting the integers to P5 format and printing them to output */
        for (int i = 0; i < Seq_length(finalSeq); i++) {
                Seq_T intSeq_ptr = Seq_get(finalSeq, i);
                for (int j = 0; j < Seq_length(intSeq_ptr); j++) {
                        int *num = Seq_get(intSeq_ptr, j);
                        unsigned char converted = (unsigned char)*num;
                        fprintf(stdout, "%c", converted);
                }
        }
}

/**********fileReader********
 *
 * About: obtains the non-int pattern and the int sequence by calling the
 *        lineSeparater function. According to the status of the bool 
 *        discovered, stores the pair to either patternMap or finalSeq
 * Inputs:
 *		char *datapp: pointer to a character array that stores the characters
 *      of a single line from the corrupted file
 *      bool *discovered: pointer to a bool variable to keep track if the
 *      infusion pattern is found
 *      Table_T patternMap: Hanson table that stores the pattern and the
 *      integer sequence from the same line as a pair
 *      Seq_T finalSeq: Hanson sequence of sequences that stores the integer
 *      values from the original P2 file
 *      const char **truePattern: Hanson atom string that stores the infusion
 *      pattern once it is found
 * Return: none
 * Expects
 *      all arguments to be nonnull
 * Notes:
 *		May CRE if malloc fails for *charArray or *word
 ************************/
void fileReader(char *datapp, bool *discovered, Table_T patternMap, 
                Seq_T finalSeq, const char **truePattern) {
        /* creating one seq to store ints and one char array for pattern */
        Seq_T intSeq = Seq_new(1000);
        char *charArray = (char*)malloc(sizeof(char)*10000);
        assert(charArray != NULL);

        /* obtaining the char pattern by calling lineSeparater */
        const char *word = lineSeparater(datapp, intSeq, charArray);
        assert(word != NULL);

        /* if the true pattern is not found, calls tableHandler */
        if (!(*discovered)) {
                *discovered = tableHandler(patternMap, word, intSeq, finalSeq);
                if (*discovered) 
                        *truePattern = Atom_new(word, Atom_length(word));
        }
        /* if the true pattern is found, adds the integers to the finalSeq */
        else {
                if (word == *truePattern) 
                        Seq_addhi(finalSeq, intSeq);
        }
        free(charArray);
}

/**********tableHandler********
 *
 * About: given a key-value pair, checks if the key is already found on the
 *        patternMap. If it is not, adds the pair to the patternMap. 
 *        Otherwise, adds the existing value and the current value to 
 *        finalSeq
 * Inputs:
 *      Table_T patternMap: Hanson table that stores the pattern and the
 *      integer sequence from the same line as a pair
 *      const char *word: pointer to an Atom string that stores the pattern
 *      of a single line
 *      Seq_T intSeq: Hanson sequence that stores the int values of a single
 *      line
 *      Seq_T finalSeq: Hanson sequence of sequences that stores the integer
 *      values from the original P2 file
 *      const char **truePattern: Hanson atom string that stores the infusion
 *      pattern once it is found
 * Return: the status of discovered; true if the infusion pattern is found (if
 *         a key repeates in patternMap), false otherwise
 * Expects
 *      all arguments to be nonnull
 * Notes:
 *		frees the patternMap if discovered is true
 ************************/
bool tableHandler(Table_T patternMap, const char *word, Seq_T intSeq, 
                  Seq_T finalSeq) {
    
        /* a bool to keep track whether the true pattern is found or not */
        bool discovered = false;
        
        /* checking if the key-value pair already exists in the table */
        Seq_T valueSeq = Table_get(patternMap, word);
        
        /* if the key is unique, pushes the key-value pair to the table */
        if(valueSeq == NULL) { 
                Table_put(patternMap, word, intSeq);
        }
        /* if the key pattern is not unique, the true pattern is found */
        else {
                discovered = true;
                
                /* putting the existing val in seq & removing it from table */
                Seq_addhi(finalSeq, valueSeq); 
                Table_remove(patternMap, word);
                /* putting the current value */
                Seq_addhi(finalSeq, intSeq);  
                
                /* call free table function */
                tableFree(patternMap);
        }

        return discovered;
}

/**********freeIntAndSequence********
 *
 * About: frees the Seq_T data structures stored as values in patternMap and
 *        malloced integer values stored in these Seq_T structures
 * Inputs:
 *      const void *key: pointer to an arbitrary key of the patternMap
 *      void **value: pointer to a pointer to an arbitrary value of the
 *      patternMap
 *      void *cl: application-specific pointer, required by the Hanson 
 *      interface
 * Return: none
 * Expects
 *      key and value to be nonnull
 ************************/
static void freeIntAndSequence(const void *key, void **value, void *cl) {
    
        Seq_T valueSeq = *value;
        
        /* traversing the integer sequence */
        for (int i = 0; i < Seq_length(valueSeq); i++) {
                /* obtaining the integer from the sequence */
                int *num = Seq_remlo(valueSeq);
                free(num);
        }
        
        /* freeing the sequence itself */
        Seq_free(&valueSeq); 

        (void) key;
        (void) cl;
}

/**********tableFree********
 *
 * About: frees the patternMap and the values of the patternMap by calling
 *        freeIntAndSequence
 * Inputs:
 *      Table_T patternMap: Hanson table that stores the pattern and the
 *      integer sequence from the same line as a pair
 * Return: none
 * Expects
 *      patternMap to be nonnull
 ************************/
void tableFree(Table_T patternMap) {

        /* freeing the integers in the sequences and the table values */
        Table_map(patternMap, freeIntAndSequence, NULL);
        Table_free(&patternMap);

}

/**********lineSeparater********
 *
 * About: given a line, it separates the non-int and int characters by storing
 *        them into two separate data structures: *charArray and intSeq
 * Inputs:
 *      Table_T patternMap: Hanson table that stores the pattern and the
 *      integer sequence from the same line as a pair
 *      Seq_T intSeq: Hanson sequence that stores the int values of a single
 *      line
 *      char *charArray: pointer to a character array that stores the non-int
 *      characters from a line
 * Return: Atom string that stores the corruption pattern
 * Expects
 *      arguments to be nonnull
 ************************/
const char *lineSeparater(char *datapp, Seq_T intSeq, char *charArray) {
        size_t i = 0;
        int index = 0;
        /* reading char by char & storing non-ints and int chars separately */
        while (datapp[i] != '\n') {
                int num = 0;
                bool a = false; /* keeping track whether the char is int */

                /* grouping the consecutive int characters */
                while (48 <= datapp[i] && datapp[i] <= 57) {
                        a = true;
                        num = num * 10 + (datapp[i++] - '0');
                }
                /* inserting ints to the intSeq or non-ints to char array */
                if (a) {
                        int *num_ptr = malloc(sizeof(int));
                        if (num_ptr == NULL) 
                                return NULL;
                        *num_ptr = num;
                        Seq_addhi(intSeq, num_ptr);
                }
                else {
                        charArray[index++] = datapp[i++];
                }
        }
        return Atom_new(charArray, index); /* convert to and return an atom */
}
        