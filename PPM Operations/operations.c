/*
 *     rotations.c
 *     by Doga Kilinc (dkilin01) & Cansu Birsen (cbirse01), October 10
 *     PPM Operations
 *
 *     About: This file is used to read the given ppm file with the choosen
 *            method (row, col, or block); apply the choosen operations
 *            (rotations, flip, transpose, or time); and output the final image
 *            to the standard output. If the time operation is choosen, the 
 *            program prints out the necessary information to the timing file.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "assert.h"
#include "operations.h"
#include "a2methods.h"
#include "pnm.h"
#include "cputiming.h"

/***********************
 * rotation operation without an explicit degree were assigned an integer value
 * for easier calculation and operation. rotation operations with an explicit
 * degree were assigned to their angle values for easier operation
************************/
#define rotation0 0
#define rotation90 90
#define rotation180 180
#define rotation270 270

#define flipHorizontal 1
#define flipVertical 2
#define transpose 3


/**********struct rotateParameters********
 * About: This struct hold the parameters A2Methods_T methods suite, client 
 *        pointer, and an integer keeping track of the rotation type. This 
 *        struct is used to pass information into the applyRotate function.
************************/
struct rotateParameters {
        A2Methods_T methods;
        void *cl;
        int rotationType;
};

/**********rotationHandler********
 * About: This function copies pixel values from an image to a Pnm_ppm struct,
 *        implements the given rotation type on the image, and prints resulting
 *        image to the stdout. If the user provided a nonnull time_file_name,
 *        the user also records the timing information for the rotation 
 *        implemented
 * Inputs:
 * FILE *fp: Pointer to the ppm file provided by the user
 * A2Methods_T methods: The method suite for A2Methods_UArray2
 * int rotation: The rotation type provided by the user
 * A2Methods_mapfun *map: The mapping function that is chosen by the user to 
 * copy pixels from the source image
 * char *time_file_name: the name of the output file if the user wants to 
 * record time information; null, otherwise.
 * char *inputFile: the name of the input file, if the file was provided by the
 * user in a file; null, otherwise.
 * Return: none
 * Expects
 * - File pointer, methods, and map to be nonnull; throws CRE if any of them 
 * are null.
************************/
void operationHandler(FILE *fp, A2Methods_T methods, int rotation, 
                    A2Methods_mapfun *map, char *time_file_name, 
                    char *inputFile) 
{

        assert(fp != NULL && methods != NULL && map != NULL);
                        
        /* copy pixels from source file in the given way */
        Pnm_ppm image = Pnm_ppmread(fp, methods);

        /* keep track of width and height and initialize timer instance */
        int width = image->width;
        int height = image->height;
        CPUTime_T timer = CPUTime_New();
        assert(timer != NULL);

        /* call rotate func. with proper arguments given the rotation type */
        if (rotation == rotation0) {
                timerStarter(timer, time_file_name);
                timerStopper(timer, time_file_name, "0 degree rotation", 
                             width * height, inputFile, width, height);
        }
        else if (rotation == rotation90) {
                rotate(methods, image, map, height, width, rotation90, timer, 
                       time_file_name, inputFile);
        }
        else if (rotation == rotation180) {
                rotate(methods, image, map, width, height, rotation180, timer, 
                       time_file_name, inputFile);
        }
        else if (rotation == rotation270) {
                rotate(methods, image, map, height, width, rotation270, timer, 
                       time_file_name, inputFile);
        }
        else if (rotation == flipHorizontal) {
                rotate(methods, image, map, width, height, flipHorizontal, 
                       timer, time_file_name, inputFile);
        }
        else if (rotation == flipVertical) {
                rotate(methods, image, map, width, height, flipVertical, timer,
                       time_file_name, inputFile);
        }
        else if (rotation == transpose) {
                rotate(methods, image, map, height, width, transpose, timer, 
                       time_file_name, inputFile);
        }
        
        /* free the timer instance */
        CPUTime_Free(&timer);

        /* print out the resulting image and free the Pnm_ppm instance */
        Pnm_ppmwrite(stdout, image);
        Pnm_ppmfree(&image);
}

/**********timerStarter********
 * About: This function starts the timer if the time_file_name is nonnull,
 *        meaning that an output file for the time information is given, so
 *        the time infromation is requested from the program
 * Inputs:
 * CPUTime_T timer: CPUTime_T instance to keep track of the time for the 
 *                  chosen operations
 * char *time_file_name: the name of the output file if the user wants to 
 * record time information; null, otherwise.
 * Return: none
 * Expects
 * - timer to be nonnull; throws CRE otherwise
************************/
void timerStarter(CPUTime_T timer, char *time_file_name) 
{
        assert(timer != NULL);
        if (time_file_name != NULL) {
                CPUTime_Start(timer);
        } 
}

/**********timerStopper********
 * About: This function stops the time recording given the user asked for time
 *        information. Then, it calls timePrinter with needed information to 
 *        output timing information to the output file.
 * Inputs:
 * CPUTime_T timer: CPUTime_T instance to keep track of the time for the 
 * chosen operations
 * char *time_file_name: the name of the output file if the user wants to 
 * record time information; null, otherwise.
 * char *operation: holds information about the type of the operation
 * int pixelNum: holds the number of pixels in the inputted file
 * char *inputFile: the name of the input file, if the file was provided by the
 * user in a file; null, otherwise.
 * int width: holds the width information about the resulting image
 * int height: holds the height information about the resulting image
 * Return: none
 * Expects
 * - timer and operation to be nonnull; throws CRE if any of them 
 * are null.
************************/
void timerStopper(CPUTime_T timer, char *time_file_name, char *operation, 
                  int pixelNum, char *inputFile, int width, int height) 
{
        assert(timer != NULL && operation != NULL);
        if (time_file_name != NULL) {
                /* record the time used and call the printer function */
                double time_used = CPUTime_Stop(timer);
                timePrinter(time_used, pixelNum, time_file_name, operation, 
                            inputFile, width, height); 
        }
}

/**********timePrinter********
 * About: This function opens the given output file for the time information
 *        and prints the detailed information, which includes file name (if
 *        the input file is NOT taken from the standard input), pixels in the
 *        image, width/height, operation type, total time, and time per pixel
 * Inputs:
 * double time_used: the amount of time (in nanoseconds) from the moment that
 *                   the timer is started until the timer is stopped
 * int pixelNum: holds the number of pixels in the inputted file
 * char *time_file_name: the name of the output file if the user wants to 
 * record time information; null, otherwise.
 * char *operation: holds information about the type of the operation
 * char *inputFile: the name of the input file, if the file was provided by the
 * user in a file; null, otherwise.
 * int width: holds the width information about the resulting image
 * int height: holds the height information about the resulting image
 * Return: none
 * Expects
 * - operation and time_file_name to be nonnull; throws CRE otherwise
************************/
void timePrinter(double time_used, int pixelNum, char *time_file_name, 
                 char *operation, char *inputFile, int width, int height) 
{
        assert(operation != NULL && time_file_name != NULL);

        /* opening the output file for that time infromation */
        FILE *fp = fopen(time_file_name, "a");
        assert(fp != NULL);
        
        /* printing the image time information obtained by the timer */
        fprintf(fp, "NEW IMAGE TIME INFORMATION:\n");
        /* checksif the inputFile is null, if it is not, meaning that the
         * input is taken as a file, it prints out the file name
         */
        if (inputFile != NULL)
                fprintf(fp, "File name: %s\n", inputFile);
        fprintf(fp, "Number of pixels in the image: %d\n", pixelNum);
        fprintf(fp, "Width the image: %d\n", width);
        fprintf(fp, "Height the image: %d\n", height);
        fprintf(fp, "Operation implemented: %s\n", operation);
        fprintf(fp, "Total time for the operation: %f nanoseconds\n", 
                time_used);
        fprintf(fp, "Time per pixel for the operation: %f nanoseconds\n", 
                time_used / (float)pixelNum);
        fprintf(fp, "----------------------------------------------------\n");
        
        /* closing the output file for that time infromation */
        fclose(fp);
}

/**********rotate********
 * About: This function implements the desired type of rotation, and starts and
 *        stops the timer information if the user asked for timing.
 * Inputs:
 * A2Methods_T methods: The method suite for A2Methods_UArray2
 * Pnm_ppm image: the struct holding the pixel information from the input image
 * A2Methods_mapfun *map: The mapping function that is chosen by the user to 
 * copy pixels from the source image
 * int newWidth: width value of the rotated image
 * int newHeigh: heigth value of the rotated image
 * int rotationType: value keeping track of the type of rotation to be 
 * implemented
 * CPUTime_T timer: CPUTime_T instance to keep track of the time for the 
 * chosen operations
 * char *time_file_name: the name of the output file if the user wants to 
 * record time information; null, otherwise.
 * char *inputFile: the name of the input file, if the file was provided by the
 * user in a file; null, otherwise.
 * Return: none
 * Expects
 * - methods, map, and image to be nonnull; throws CRE if any of them 
 * are null.
************************/
void rotate(A2Methods_T methods, Pnm_ppm image, A2Methods_mapfun *map, 
            int newWidth, int newHeight, int rotationType, CPUTime_T timer, 
            char *time_file_name, char *inputFile) 
{
        assert(methods != NULL && map != NULL && image != NULL);

        /* call timerStarter to start timer if user asked for it */
        timerStarter(timer, time_file_name);
    
        /* initiate 2D array to hold rotated image info */
        A2Methods_UArray2 rotated = methods->new(newWidth, newHeight, 
                                                 methods->size(image->pixels));
                
        /* initiate rotateParameters to hold info needed for map function */
        struct rotateParameters prm = {methods, rotated, rotationType};                                   
        /* call map function with rotation apply function */
        map(image->pixels, rotateApply, &prm);
        
        /* free the current pixels in image and update it to rotated version */
        methods->free(&image->pixels);
        image->pixels = rotated;
        image->width = methods->width(rotated);
        image->height = methods->height(rotated); 

        /* record type of operation for timing information output */
        char operation[20];  
        if (rotationType == rotation0 ||rotationType == rotation90 || 
            rotationType == rotation180 || rotationType == rotation270) 
                sprintf(operation, "%d degree rotation", rotationType);
        else if (rotationType == flipHorizontal)
                strcpy(operation, "horizontal flip");
        else if (rotationType == flipVertical)
                strcpy(operation, "vertical flip");
        else if (rotationType == transpose)
                strcpy(operation, "transpose");
        
        /* stop the timer if the user asked for time information */
        timerStopper(timer, time_file_name, operation, newWidth * newHeight, 
                     inputFile, newWidth, newHeight);
}

/**********rotateApply********
 * About: This function moves the element being visited to a new location on a 
 * new 2D array. The place to move the element to is decided on depending on 
 * the type of rotation.
 * Inputs:
 * int col: column index of the current element being visited
 * int row: row index of the current element being visited
 * A2Methods_UArray2 array: the current array holding the pixels information 
 * from source image
 * void *elem: current element being visited, which holds information about the
 * pixel
 * void *rotateStruct: holds a rotateParameters struct which stores the method 
 * suite for A2Methods_UArray2, new 2D array to keep the rotated version, and
 * type of rotation
 * Return: none
 * Expects
 * - array and rotateStruct to be nonnull; throws CRE if any of them are null.
************************/
void rotateApply(int col, int row, A2Methods_UArray2 array, void *elem, 
                 void *rotateStruct) 
{
        assert(array != NULL && rotateStruct != NULL);
        
        /* casts void pointers to dereference them later */
        struct Pnm_rgb *num = elem;
        struct rotateParameters *prm = rotateStruct;

        /* access info from the rotateStruct */
        A2Methods_T methods = prm->methods;
        A2Methods_UArray2 rotated = prm->cl;
        struct Pnm_rgb *num_new;
        
        /* checks rotation type and places the current value to new location*/
        if (prm->rotationType == rotation90) {
                num_new = methods->at(rotated, 
                                      methods->height(array) - row - 1, col);
        }
        else if (prm->rotationType == rotation180) {
                num_new = methods->at(rotated, methods->width(array) - col - 1,
                                      methods->height(array) - row - 1);
        }
        else if (prm->rotationType == rotation270) {
                num_new = methods->at(rotated, row, 
                                      methods->width(array) - col - 1);
        }
        else if (prm->rotationType == flipHorizontal) {
                num_new = methods->at(rotated, methods->width(array) - col - 1,
                                      row);
        }
        else if (prm->rotationType == flipVertical) {
                num_new = methods->at(rotated, col, 
                                      methods->height(array) - row - 1);
        }
        else if (prm->rotationType == transpose) {
                num_new = methods->at(rotated, row, col);
        }

        /* assign current value to new location */
        *num_new = *num;
}
