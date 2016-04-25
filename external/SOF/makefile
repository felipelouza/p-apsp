CC = g++
CPPFLAGS =  -fopenmp -ansi -pedantic -O3 -ldl

objects = Apsp.o Tools.o testutils.o Ctree.o 

default: $(objects)
	cc -c -ldl ../malloc_count/malloc_count.c -o ../malloc_count/malloc_count.o 
	$(CC) $(CPPFLAGS) -o Apsp $(objects) ../malloc_count/malloc_count.o
	$(CC) gen_test.cpp -o gen
	$(CC) converter_fasta_to_plain.cpp -o converter

clean:
	rm -f core *.exe *.o *~ Apsp ../malloc_count/malloc_count.o  



