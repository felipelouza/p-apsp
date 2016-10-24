#p-apsp

This code is an implementation of p-apsp [1], a new (parallel) algorithm based
on [2] to solve the all-pairs suffix-prefix problem.


#run:

To run a test with K=100 strings from INPUT=dataset/c_elegans_ests_200.fasta, overlap threshold L=10, using T=4 threads type:

```sh
make p-apsp
make run DIR=dataset/ INPUT=c_elegans_ests_200.fasta K=100 L=10 OUTPUT=0 T=4
```

To run the sequential algorithm [external/apsp_tustumi.cpp](https://github.com/felipelouza/p-apsp/blob/master/external/apsp_tustumi.cpp), type:
```sh
make apsp_tustumi
make run_tustumi DIR=dataset/ INPUT=c_elegans_ests_200.fasta K=100 L=10 OUTPUT=0
```

**Output:**

Both algorithms output _.bin_ files at input folder (if OUTPUT=1).

In order to compare both output, change line 25 of [main.cpp](https://github.com/felipelouza/p-apsp/blob/master/main.cpp#L25):

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
make run_tustumi DIR=dataset/ INPUT=c_elegans_ests_200.fasta K=100 L=10 OUTPUT=1
make diff DIR=dataset/ INPUT=c_elegans_ests_200.fasta K=100
```


Note: all algorithms need [sdsl-lite](https://github.com/simongog/sdsl-lite).

#references:

[1] Louza, F. A, Gog, S., Zanotto, L., Araujo, G., Telles, G. P. (2016). Parallel computation for the all-pairs suffix-prefix problem. In Proc SPIRE, pp 122-132, 2016, http://link.springer.com/chapter/10.1007/978-3-319-46049-9_12.

[2] Tustumi, W. H. A., Gog, S., Telles, G. P., Louza, F.A. (2016). An improved algorithm for the all-pairs suffix-prefix problem. Journal of Discrete Algorithms, 47, 34-43, http://www.sciencedirect.com/science/article/pii/S1570866716300053.



