CCLIB=-lsdsl -ldivsufsort -ldivsufsort64 -Wno-comment -fopenmp
VLIB= -g -O0

LIB_DIR = ${HOME}/lib
INC_DIR = ${HOME}/include
MY_CXX_FLAGS= -std=c++11 -Wall -Wextra  -DNDEBUG $(CODE_COVER)
MY_CXX_OPT_FLAGS= -O3 -ffast-math -funroll-loops -m64 -fomit-frame-pointer -D_FILE_OFFSET_BITS=64
#MY_CXX_OPT_FLAGS= -m64 -fomit-frame-pointer -D_FILE_OFFSET_BITS=64
MY_CXX=/usr/bin/c++

LFLAGS = -lm -ldl
LIBOBJ = external/malloc_count/malloc_count.o

CXX_FLAGS=$(MY_CXX_FLAGS) $(MY_CXX_OPT_FLAGS) -I$(INC_DIR) -L$(LIB_DIR) $(LIBOBJ) $(LFLAGS)

CLAGS= -DSYMBOLBYTES=1

####

#INPUT = dataset/c_elegans_ests_200.fasta
INPUT = ../dataset/all_ests.fasta
K = 4 
L = 1 # min_lcp
OUTPUT = 0
T = 4 # n_threads
####

all: main
#suffix_array_solution_prac 

suffix_array_solution_prac: external/suffix_array_solution_prac.cpp ${LIBOBJ}
	$(MY_CXX) $(CXX_FLAGS) external/suffix_array_solution_prac.cpp $(CCLIB) -o external/suffix_array_solution_prac  

main: main.cpp ${LIBOBJ}
	$(MY_CXX) $(CXX_FLAGS) main.cpp $(CCLIB) -o main  

old_algorithm: old_algorithm.cpp ${LIBOBJ}
	$(MY_CXX) $(CXX_FLAGS) old_algorithm.cpp $(CCLIB) -o old_algorithm  

strip: utils/strip.cpp
	$(MY_CXX) utils/strip.cpp -o utils/strip

clean:
	rm external/suffix_array_solution_prac -f
	rm main -f
	rm *.bin -f
	rm *.sdsl -f

run: run_main 
#run_suff 

run_suff: 
	external/suffix_array_solution_prac $(INPUT) $(K) $(L) $(OUTPUT)
	
run_main: 
	./main $(INPUT) $(K) $(L) $(OUTPUT) $(T)
	
run_old: 
	./old_algorithm $(INPUT) $(K) $(L) $(OUTPUT) $(T)

run_strip: 
	utils/strip $(INPUT) $(K)
	
valgrind_main: main
	valgrind --tool=memcheck --leak-check=full --track-origins=yes ./main $(INPUT) $(K) $(L) $(OUTPUT) $(T)


diff:
	ls -lh *.bin
	diff results_new.bin results_old.bin

