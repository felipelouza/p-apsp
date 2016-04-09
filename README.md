#apsp

This code is an implementation of a new algorithm to solve the All-pairs suffix-prefix problem \[1\].

In an empirical evaluation we observed that the new algorithm is over two times faster and more space-efficient than the method proposed by Ohlebusch and Gog  \[2\].

The source code in [suffix_array_solution_prac.cpp](https://github.com/felipelouza/apsp/blob/master/external/suffix_array_solution_prac.cpp) is an update of \[2\] (available at www.uni-ulm.de/in/theo/research/seqana.html) to run using sdsl-lite v.2 (https://github.com/simongog/sdsl-lite).

#run:

To run a test with K=100 strings from INPUT=dataset/c_elegans_ests_200.fasta and overlap threshold T=10 type:

```sh
make
make run INPUT=dataset/c_elegans_ests_200.fasta K=100 T=10 OUTPUT=1
```

**Output:**

Both algorithms output _.bin_ files at input folder (if OUTPUT=1).
In order to compare the output files, one may run:

```sh
make diff
```

Note: both algorithms need [sdsl-lite](https://github.com/simongog/sdsl-lite).


#references:

[1] Gusfield, D., M. Landau, G., & Schieber, B. (1992). An efficient algorithm for the All Pairs Suffix-Prefix Problem. Information Processing Letters, 41(4), 181–185.

[2] Ohlebusch, E., & Gog, S. (2010). Efficient algorithms for the all-pairs suffix-prefix problem and the all-pairs substring-prefix problem. Information Processing Letters, 110(3), 123–128. doi:10.1016/j.ipl.2009.10.015


