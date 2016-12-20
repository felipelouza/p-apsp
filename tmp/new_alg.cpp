/*
* Felipe A. Louza
* 06 abr 2016
*
* New (parallel) algorithm to solve the apsp problem
*/

#include <sdsl/construct.hpp>
#include <sdsl/int_vector.hpp>
#include <sdsl/config.hpp>
#include <sdsl/construct_config.hpp>
#include <iostream>
#include <climits>
#include "lib/utils.h"
#include "lib/file.h"
#include "external/malloc_count/malloc_count.h"
#include <sdsl/bit_vectors.hpp>

#include <omp.h>

#define DEBUG 0
#define SAVE_SPACE 1
#define OMP 1

//output format [row, column]
#define RESULT 1 // 0 = [prefix, suffix], 1 = [suffix, prefix]

using namespace std;
using namespace sdsl;

typedef map<uint_t, uint_t> tMII;

#if SAVE_SPACE
typedef vector<tMII> tVMII;
#else
typedef uint_t** tVMII;
#endif

inline int add_overlap(tVMII& result, uint_t t, uint_t p, uint_t value);

double start, total;

int main(int argc, char *argv[]){

	uint_t k = 0;
	uint_t n = 0;
	uint_t l = 0;

	if(argc!=7)
		return 1;

	char* c_dir = argv[1];
	char* c_file = argv[2];

    	sscanf(argv[3], "%" PRIdN "", &k);

	file_chdir(c_dir);
	unsigned char **R = (unsigned char**) file_load_multiple(c_file, k, &n);
	if(!R){
		fprintf(stderr, "Error: less than %" PRIdN " strings in %s\n", k, c_file);
		return 0;
	}

	n++;
	int_vector<> str_int(n);
	//vector<uint_t> ms;  // vector containing the length of the ests

	for(uint_t i=0; i<k; i++){
		uint_t m = strlen((char*)R[i]);
		//ms.push_back(m);
		for(uint_t j=0; j<m; j++)
			str_int[l++] = R[i][j]+(k+1)-'A';
		str_int[l++] = i+1; //add $_i as separator
	}	
	str_int[l]=0;

	
	//free memory
	for(uint_t i=0; i<k; i++)
		free(R[i]);
	free(R);

	uint_t threshold;
	sscanf(argv[4], "%" PRIdN "", &threshold);
	printf("L: %" PRIdN "\n", threshold);

	uint_t output;
	sscanf(argv[5], "%" PRIdN "", &output);

	int_t n_threads;
	sscanf(argv[6], "%" PRIdN "", &n_threads);
	printf("K: %" PRIdN "\n", k);

	#if OMP
		omp_set_num_threads(n_threads);
	
		#pragma omp parallel
		{
		if(omp_get_thread_num()==0)
			printf("N_THREADS: %d\n", omp_get_num_threads());
		}
		printf("N_PROCS: %d\n", omp_get_num_procs());
	#endif

	cout<<"length of all strings N = "<<n<<endl; //" Number of strings k="<<k<<endl;

	printf("sizeof(int): %zu bytes\n", sizeof(int_t));

	#if SAVE_SPACE
		cout<<"SAVE_SPACE"<<endl;
	#endif


 	string dir = "sdsl";
	mkdir(dir.c_str());
    	string id = c_file;
	id += "."+to_string(k);
    	cache_config m_config(true, dir, id);

	store_to_cache(str_int, conf::KEY_TEXT_INT, m_config);

	int_vector<> sa;
	int_vector<> lcp;

	if(!load_from_cache(sa, conf::KEY_SA, m_config)){
		#if OMP
			start = omp_get_wtime();
		#endif
		construct_sa<0>(m_config); load_from_cache(sa, conf::KEY_SA, m_config);
		#if OMP
			printf("TIME = %f (in seconds)\n", omp_get_wtime()-start);
		#endif
	}

	if(!load_from_cache(lcp, conf::KEY_LCP, m_config)){
		#if OMP
			start = omp_get_wtime();
		#endif
		construct_lcp_PHI<0>(m_config); load_from_cache(lcp, conf::KEY_LCP, m_config);
		#if OMP
			printf("TIME = %f (in seconds)\n", omp_get_wtime()-start);
		#endif
	}

    	uint_t *Block =  new uint_t[k];
    	uint_t *Prefix = new uint_t[k];

	//Find initial position of each Block b_i
	size_t tmp=0; 
	for(size_t i=0;i<sa.size();++i){

		uint_t t = str_int[(sa[i]+n-1)%n];	
		if( t < k ){// found whole string as suffix
			Block[tmp] = i;
			Prefix[tmp] = t;
			tmp++;
		}
	}

	#if SAVE_SPACE
		tVMII result(k);//(k, tVI(k,0));
	#else
		tVMII result = new uint_t* [k];
		for(unsigned i=0; i<k; ++i){
			result[i] = new uint_t [k];
			for(unsigned j=0; j<k; ++j)
				result[i][j] = 0;
		}
	#endif

	cout<<"computing..."<<endl;

	#if OMP
		total = omp_get_wtime();
		start = omp_get_wtime();
	#endif

	uint_t inserts = 0, removes = 0;

	uint_t *Min_lcp = new uint_t[k+1];
	Min_lcp[k] = 0;

	bit_vector B = bit_vector(n, 0);

	#if OMP
		#pragma omp parallel for reduction(+:inserts)
	#endif
	for(uint_t p = 0; p < k; p++){
	
		uint_t min_lcp = UINT_MAX;

		//LOCAL solution:
		uint_t previous =(p>0? Block[p-1]: k+1);
		for(uint_t i=Block[p]-1; i>=previous; --i){

			if(min_lcp >= lcp[i+1]){

				min_lcp = lcp[i+1];
				uint_t t = str_int[sa[i]+lcp[i+1]]-1;//current suffix     

				if(t < k)//complete overlap
					if(min_lcp >= threshold){
						B[i] = 1; //it is a complete overlap.
						inserts++;
					}
        		}
		}
		Min_lcp[p] = min_lcp;
	}

	#if OMP
		cout<<"--"<<endl;
		printf("TIME = %f (in seconds)\n", omp_get_wtime()-start);
		start = omp_get_wtime();
	#endif

	//GLOBAL solution 
	uint_t overlaps=0;
	#if OMP
		#pragma omp parallel for reduction(+:overlaps)
	#endif
	for(uint_t p = 0; p < k; p++){

		uint_t previous =(p>0? Block[p-1]: k);
		for(uint_t i=previous; i<Block[p]; i++){

			if(B[i]){

				uint_t t = str_int[sa[i]+lcp[i+1]]-1;//current suffix     
				overlaps += add_overlap(result, t, Prefix[p], lcp[i+1]);//Local overlap

				uint_t next = p+1;
				while(Min_lcp[next]>=lcp[i+1]){
					overlaps += add_overlap(result, t, Prefix[next], lcp[i+1]);
					next++;
				}
			
        		}
		}
	}

	#if OMP
	cout<<"--"<<endl;
	printf("TIME = %f (in seconds)\n", omp_get_wtime()-start);
	start = omp_get_wtime();
	#endif	

	uint_t contained=0;
	//contained suffixes
	#if OMP
		#pragma omp parallel for reduction(+:contained) //firstprivate(threshold,n,k) 
	#endif
	for(uint_t p = 0; p < k; p++){

		#if SAVE_SPACE == 0
			result[p][p] = 0;
		#endif

		uint_t q = Block[p]+1;
		//while(lcp[q] == ms[Prefix[p]] and ((tt=str_int[sa[q]+[Prefix[p]]]-1) < k ) and q < n ){
		while(str_int[sa[q]+lcp[q]] < k and q < n ){
	
			if(lcp[q] >= threshold){
			
				uint_t tt=str_int[sa[q]+lcp[q]]-1;
				contained++;

				#if RESULT 
					result[tt][Prefix[p]] = lcp[q];
				#else
					result[Prefix[p]][tt] = lcp[q];
				#endif
			}
			else 
				break;
			q++;
		}

	}

	#if OMP
		cout<<"--"<<endl;
		printf("TIME = %f (in seconds)\n", omp_get_wtime()-start);

		cout<<"## TOTAL"<<endl;
		printf("TIME = %f (in seconds)\n", omp_get_wtime()-total);
		cout<<"##"<<endl;

		fprintf(stderr, "%lf\n", omp_get_wtime()-total);
	#endif

	cout<<"--"<<endl;
   	printf("inserts %" PRIdN "\n", inserts);
	printf("removes %" PRIdN "\n", removes);
        printf("overlaps %" PRIdN " (%" PRIdN ")\n", overlaps, contained);
	cout<<"--"<<endl;


	#if SAVE_SPACE
		
		#if DEBUG
		uint_t i = 0;
		for (tVMII::iterator it_row=result.begin(); it_row!=result.end(); ++it_row){			
			uint_t j = 0;
			for(tMII::iterator it_column=it_row->begin(); it_column!=it_row->end(); ++it_column){
				cout<<it_column->second<<" ";
				if(j++ > 10) break;
			}
			if(it_row->begin()!=it_row->end()){
				cout<<endl;		
				if(i++ > 10) break;
			}
		}
		#endif

		if(output==1){
			ofstream out_file(dir+"/output."+id+".new.bin",ios::out | ios::binary);			
			for (tVMII::iterator it_row=result.begin(); it_row!=result.end(); ++it_row)
				for(tMII::iterator it_column=it_row->begin(); it_column!=it_row->end(); ++it_column)
					out_file.write((char*)&it_column->second, sizeof(uint_t));
			out_file.close();
		}
	#else

		#if DEBUG
		for(uint_t i=0; i<10 && i<k; ++i){
			for(uint_t j=0; j<10 && j<k; ++j)
				cout<<result[i][j]<<" ";
			cout<<endl;
		}
		#endif

		if(output==1){
			ofstream out_file(dir+"/output."+id+".new.bin",ios::out | ios::binary);			
			for(uint_t i=0; i<k; ++i)
				for(uint_t j=0; j<k; ++j)
					out_file.write((char*)&result[i][j], sizeof(uint_t));
			out_file.close();
		}

		for(uint_t i=0; i<k; ++i)
			delete[] result[i];
		delete[] result;

	#endif

	delete[] Block;
	delete[] Prefix;
	delete[] Min_lcp;

	return 0;
}


inline int add_overlap(tVMII& result, uint_t t, uint_t p, uint_t value){

int a=0;

	#if SAVE_SPACE
	if(t==p)
		return 0;
	#endif

	#pragma omp critical
	{
		#if RESULT
	//	if(result[t][p]==0) a=1;
	
		if(result[t][p]<value){
	
			result[t][p] = value;
		}
		#else
	//	if(result[p][t]==0) a=1;
	
		if(result[p][t]<value){
	
			result[p][t] = value;
		}
		#endif
	}

return a;
}
