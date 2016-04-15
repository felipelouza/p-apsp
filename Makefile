CCLIB=-lsdsl -ldivsufsort -ldivsufsort64 -Wno-comment -fopenmp
VLIB= -g -O0

LIB_DIR = ${HOME}/lib
INC_DIR = ${HOME}/include
MY_CXX_FLAGS= -std=c++11 -Wall -Wextra  -DNDEBUG $(CODE_COVER)
MY_CXX_OPT_FLAGS= -O3 -ffast-math -funroll-loops -m64 -fomit-frame-pointer -D_FILE_OFFSET_BITS=64
#MY_CXX_OPT_FLAGS= -m64 -fomit-frame-pointer -D_FILE_OFFSET_BITS=64
MY_CXX=g++

LFLAGS = -lm -ldl

LIBOBJ = external/malloc_count/malloc_count.o\
	 lib/file.o

CXX_FLAGS=$(MY_CXX_FLAGS) $(MY_CXX_OPT_FLAGS) -I$(INC_DIR) -L$(LIB_DIR) $(LFLAGS)

CLAGS= -DSYMBOLBYTES=1

####

#DIR = ~/dataset/ests/
#INPUT = all_ests.fasta

DIR = ~/dataset/reads/
INPUT = reads.fastq

K = 4 
L = 1 # min_lcp
OUTPUT = 0
T = 4 # n_threads
####

all: main
#suffix_array_solution_prac 

suffix_array_solution_prac: external/suffix_array_solution_prac.cpp ${LIBOBJ}
	$(MY_CXX) $(CXX_FLAGS) external/suffix_array_solution_prac.cpp $(CCLIB) -o external/suffix_array_solution_prac  

#lib/file.o
lib: 
	$(MY_CXX) -c lib/file.c -o lib/file.o 

main: lib main.cpp
	$(MY_CXX) $(CXX_FLAGS) main.cpp $(CCLIB) -o main ${LIBOBJ} 

old_algorithm: lib old_algorithm.cpp
	$(MY_CXX) $(CXX_FLAGS) old_algorithm.cpp $(CCLIB) -o old_algorithm  ${LIBOBJ} 

strip: utils/strip.cpp
	$(MY_CXX) utils/strip.cpp -o utils/strip

clean:
	rm external/suffix_array_solution_prac -f
	rm main -f
	rm *.bin -f
	rm lib/*.bin -f
	rm *.sdsl -f

run: run_main 
#run_suff 

run_suff: 
	external/suffix_array_solution_prac $(DIR) $(INPUT) $(K) $(L) $(OUTPUT)
	
run_main: 
	./main $(DIR) $(INPUT) $(K) $(L) $(OUTPUT) $(T)
	
run_old: 
	./old_algorithm $(DIR) $(INPUT) $(K) $(L) $(OUTPUT) $(T)

run_strip: 
	utils/strip $(INPUT) $(K)
	
valgrind: 
	valgrind --tool=memcheck --leak-check=full --track-origins=yes ./main $(DIR) $(INPUT) $(K) $(L) $(OUTPUT) $(T)

diff:
	diff $(DIR)output.tmp.$(K).seq.bin $(DIR)output.tmp.$(K).par.bin 

remove:
	rm $(DIR)*.sdsl
	rm $(DIR)*.bin

