CCLIB=-lsdsl -ldivsufsort -ldivsufsort64 -Wno-comment -fopenmp
VLIB= -g -O0

LIB_DIR = ${HOME}/lib
INC_DIR = ${HOME}/include
MY_CXX_FLAGS= -std=c++11 -Wall -Wextra -DNDEBUG -D__STDC_FORMAT_MACROS 
MY_CXX_OPT_FLAGS= -O3 -m64 
#MY_CXX_OPT_FLAGS= $(VLIB)
MY_CXX=g++

LFLAGS = -lm -ldl

LIBOBJ = external/malloc_count/malloc_count.o\
	 lib/utils.o\
	 lib/file.o

CXX_FLAGS=$(MY_CXX_FLAGS) $(MY_CXX_OPT_FLAGS) -I$(INC_DIR) -L$(LIB_DIR) $(LFLAGS)

CLAGS= -DSYMBOLBYTES=1

####

DIR = ~/dataset/
INPUT = all_ests.fasta

K = 100
L = 5 # min_lcp
OUTPUT = 0
T = 8 # n_threads
####

all: p-apsp new_alg
#apsp_tustumi
#suffix_array_solution_prac

suffix_array_solution_prac: external/suffix_array_solution_prac.cpp ${LIBOBJ}
	$(MY_CXX) $(CXX_FLAGS) external/suffix_array_solution_prac.cpp $(CCLIB) -o external/suffix_array_solution_prac  

lib: lib/file.o 
	$(MY_CXX) -c lib/file.c -o lib/file.o 

new_alg: lib new_alg.cpp ${LIBOBJ}
	$(MY_CXX) $(CXX_FLAGS) new_alg.cpp $(CCLIB) -o new_alg ${LIBOBJ} 

p-apsp: lib main.cpp ${LIBOBJ}
	$(MY_CXX) $(CXX_FLAGS) main.cpp $(CCLIB) -o p-apsp ${LIBOBJ} 

apsp_tustumi: lib external/apsp_tustumi.cpp
	$(MY_CXX) $(CXX_FLAGS) external/apsp_tustumi.cpp $(CCLIB) -o external/apsp_tustumi  ${LIBOBJ} 

strip: utils/strip.cpp
	$(MY_CXX) utils/strip.cpp -o utils/strip

clean:
	rm external/suffix_array_solution_prac -f
	rm external/malloc_count/malloc_count.o -f
	rm p-apsp -f
	rm new_alg -f
	rm external/apsp_tustumi -f
	rm *.bin -f
	rm lib/*.o -f
	rm *.sdsl -f

run: run_main run_new

run_suff: 
	external/suffix_array_solution_prac $(DIR) $(INPUT) $(K) $(L) $(OUTPUT)
	
run_main: 
	./p-apsp $(DIR) $(INPUT) $(K) $(L) $(OUTPUT) $(T)
	
run_new: 
	./new_alg $(DIR) $(INPUT) $(K) $(L) $(OUTPUT) $(T)
	
run_tustumi: 
	external/apsp_tustumi $(DIR) $(INPUT) $(K) $(L) $(OUTPUT) $(T)

run_strip: 
	utils/strip $(INPUT) $(K)
	
valgrind: 
	valgrind --tool=memcheck --leak-check=full --track-origins=yes ./p-apsp $(DIR) $(INPUT) $(K) $(L) $(OUTPUT) $(T)

valgrind_new:
	valgrind --tool=memcheck --leak-check=full --track-origins=yes ./new_alg $(DIR) $(INPUT) $(K) $(L) $(OUTPUT) $(T)

diff:
#	ls -lh $(DIR)sdsl/output.$(INPUT).$(K).*.bin
#	diff $(DIR)sdsl/output.$(INPUT).$(K).seq.bin $(DIR)sdsl/output.$(INPUT).$(K).par.bin 
	diff $(DIR)sdsl/output.$(INPUT).$(K).par.bin $(DIR)sdsl/output.$(INPUT).$(K).new.bin 

remove:
	rm $(DIR)sdsl/*.sdsl
	rm $(DIR)sdsl/*.bin

