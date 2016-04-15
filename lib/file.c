#include "file.h"

const char *get_filename_ext(const char *filename) {                                                  
    const char *dot = strrchr(filename, '.');                                                         
    if(!dot || dot == filename) return "";                                                            
    return dot + 1;                                                                                   
}   

/*******************************************************************/

/* Changes to a working directory, where everything will be read
 * from and written to:
 */ 
int file_chdir(char* dir){
	
	char* oldwd = getcwd(NULL,0);
	if (!oldwd) perror("getcwd");
	if (chdir(dir) == -1) perror("chdir");

	free(oldwd);
return 0;
}

/*******************************************************************/

//Open a file and returns a pointer
FILE* file_open(char *c_file, const char * c_mode){
	
	FILE* f_in;
	
	f_in = fopen(c_file, c_mode);
	if (!f_in) perror ("file_open");
	
return f_in;
}


int file_close(FILE* f_in){
	
	fclose(f_in);
	if (!f_in) perror ("file_close");
	
return 0;
}

/*******************************************************************/

size_t file_size(FILE* f_in){

    fseek(f_in, 0, SEEK_END);
    size_t length = ftell(f_in);
    rewind(f_in);
	
return length;
}

/*******************************************************************/

char* file_load(FILE* f_in) {
	
	size_t len = 0;
	ssize_t size = 0;
 	char *c_aux = NULL;
 	
 	/*Read one line*/
 	size = getline(&c_aux, &len, f_in);
 	if (size == -1) perror("file_load");

 	/*Copy line to c_buffer*/
 	char *c_buffer = (char*) malloc((size+1)*sizeof(char));
	strncpy(c_buffer, c_aux, size);

	c_buffer[size-1] = END_MARKER;
	c_buffer[size] = '\0';

	free(c_aux);

return c_buffer;
}

/*******************************************************************/

// read line by line
char** load_multiple_txt(FILE* f_in, int k, uint32_t *n) {

	char **c_buffer = (char**) malloc(k*sizeof(char*));

	int i;
 	for(i=0; i<k; i++){
		size_t len = 0;
		c_buffer[i] = NULL;
			
		ssize_t size = getline(&c_buffer[i], &len, f_in);
		if (size == -1){
			return 0;
		}
		c_buffer[i][size-1] = 0;

		(*n) += size;
	}


return c_buffer;
}

// read sequences separeted by '@' line
char** load_multiple_fastq(FILE* f_in, int k, uint32_t *n){

	char **c_buffer = (char**) malloc(k*sizeof(char*));

	int i;
 	for(i=0; i<k; i++){

                size_t len = 0;
		char *buf = NULL;

		ssize_t size = getline(&buf, &len, f_in); // @'s line
                if (size <= 1)
                        return 0;

                c_buffer[i] = NULL;
		size = getline(&c_buffer[i], &len, f_in); // read line
                c_buffer[i][size-1] = 0;

                (*n) += size;

		getline(&buf, &len, f_in); // +'s line
		getline(&buf, &len, f_in); // @'s line

		free(buf);
	}

return c_buffer;
}

// read sequences separeted by '>' line
char** load_multiple_fasta(FILE* f_in, int k, uint32_t *n){

	char **c_buffer = (char**) malloc(k*sizeof(char*));

	char *buf = NULL;
	size_t len = 0;

	getline(&buf, &len, f_in);// first sequence
	free(buf);

	int count=0;
	int i;
 	for(i=0; i<k; i++){

		if(i!=count) return 0;

		len = 0;
		int nalloc = 128;
		c_buffer[i] = (char*) malloc(nalloc*sizeof(char));

		uint32_t p=0;
		while(getline(&buf, &len, f_in)!=-1){

			if(buf[0] == '>'){
				count++;
				break;
			}

			if(p+len>nalloc){
				nalloc += 128;
				c_buffer[i] = (char*) realloc(c_buffer[i], sizeof(char) * nalloc);
			}

			strcpy(&c_buffer[i][p], buf);
			p+=strlen(buf)-1;
			
		}

		free(buf);
		c_buffer[i][p] = 0;
		(*n) += p+1;
	}

return c_buffer;
}

/*******************************************************************/

char** file_load_multiple(char* c_file, int k, uint32_t *n) {

/* .ext
 * .txt   - strings per line
 * .fasta - strings separated by '>' line
 * .fastq - strings separated by four lines
 */

	FILE* f_in = file_open(c_file, "rb");
	if(!f_in) return 0;

	const char *type = get_filename_ext(c_file);
	char **c_buffer = NULL; // = (char**) malloc(k*sizeof(char*));

	if(strcmp(type,"txt") == 0)
		c_buffer = load_multiple_txt(f_in, k, n);

	else if(strcmp(type,"fasta") == 0)
		c_buffer = load_multiple_fasta(f_in, k, n);

	else if(strcmp(type,"fastq") == 0)
		c_buffer = load_multiple_fastq(f_in, k, n);

	fclose(f_in);

return c_buffer;
}

/*******************************************************************/