#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "assert.h"


void timePrinter(double time_used, int pixelNum, char *time_file_name, char *operation) {
        
        FILE *fp = fopen(time_file_name, "a");
        assert(fp != NULL);

        


}