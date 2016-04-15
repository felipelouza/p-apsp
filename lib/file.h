#ifndef FILE_H
#define FILE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <inttypes.h>


#define END_MARKER 0

/*******************************************************************/
int file_chdir(char* dir);

FILE* file_open(char *c_file, const char * c_mode);
int file_close(FILE* f_in);

size_t file_size(FILE* f_in);

char* file_load(FILE* f_in) ;
char** file_load_multiple(char* c_file, int k, uint32_t* n) ;

/*******************************************************************/



#endif
