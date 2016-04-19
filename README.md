#p-apsp

This code is an implementation of a new (parallel) algorithm to solve the All-pairs suffix-prefix problem.

**main.cpp**: parallel version.

**apsp.cpp**: sequential version (JDA's submission).

#run:

To run a test with K=100 strings from INPUT=dataset/c_elegans_ests_200.fasta, overlap threshold L=10, and using T=4 threads type:

```sh
make
make run DIR=dataset/ INPUT=c_elegans_ests_200.fasta K=100 L=10 OUTPUT=0 T=4
```

To run the sequential version, type:
```sh
make apsp
make run_apsp DIR=dataset/ INPUT=c_elegans_ests_200.fasta K=100 L=10 OUTPUT=0
```

**Output:**

Both algorithms output _.bin_ files at input folder (if OUTPUT=1).

In order to compare both output, one can change line 24 of main.cpp:

```sh
#define RESULT 1
```
to
```sh
#define RESULT 0
```

And type:

```
make
make run DIR=dataset/ INPUT=c_elegans_ests_200.fasta K=100 L=10 OUTPUT=1 T=4
make run_apsp DIR=dataset/ INPUT=c_elegans_ests_200.fasta K=100 L=10 OUTPUT=1
make diff DIR=dataset/  K=100
```


Note: both algorithms need [sdsl-lite](https://github.com/simongog/sdsl-lite).



