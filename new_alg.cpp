/*
* Felipe A. Louza
* 12 dec 2016
*
* New^2 (parallel) algorithm to solve the apsp problem
*/

#include <sdsl/construct.hpp>
#include <sdsl/int_vector.hpp>
#include <sdsl/config.hpp>
#include <sdsl/construct_config.hpp>
#include <iostream>
#include <climits>
#include "lib/file.h"
#include "lib/utils.h"
#include "external/malloc_count/malloc_count.h"
//#include <sdsl/bit_vectors.hpp>

#include <omp.h>

#define DEBUG 0
#define SAVE_SPACE 1
#define OMP 1

//output format [row, column]
#define RESULT 1 // 0 = [prefix, suffix], 1 = [suffix, prefix]

using namespace std;
using namespace sdsl;

typedef struct _list{    
    uint_t lcp;
    uint_t next;
} tLIST;

typedef struct _tl{    
    uint_t value;
    struct _tl *next;
} Tl;

typedef map<uint_t, uint_t> tMII;
typedef vector<tMII> tVMII;

inline void INSERT(tLIST* LIST, int_t ptr, int t, int lcp, uint_t *TOP_l,  uint_t *TOP_g, uint_t* LAST);

double start, total;

int main(int argc, char *argv[]){

	uint_t K = 0;
	uint_t n = 0;
	uint_t l = 0;

	if(argc!=7)
		return 1;

	/**/

	char* c_dir = argv[1];//dir
	char* c_file = argv[2];//file

    	sscanf(argv[3], "%" PRIdN "", &K);//number of strings

	//load the strings
	file_chdir(c_dir);
	unsigned char **R = (unsigned char**) file_load_multiple(c_file, K, &n);
	if(!R){
		fprintf(stderr, "Error: less than %" PRIdN " strings in %s\n", K, c_file);
		return 0;
	}

	n++;
	int_vector<> str_int(n);
	//vector<uint_t> ms;  // vector containing the length of the ests

	for(uint_t i=0; i<K; i++){
		uint_t m = strlen((char*)R[i]);
		//ms.push_back(m);
		for(uint_t j=0; j<m; j++)
			str_int[l++] = R[i][j]+(K+1)-'A';
		str_int[l++] = i+1; //add $_i as separator
	}	
	str_int[l]=0;

	
	//free memory
	for(uint_t i=0; i<K; i++)
		free(R[i]);
	free(R);

	/**/

	uint_t threshold;
	sscanf(argv[4], "%" PRIdN "", &threshold);
	printf("L: %" PRIdN "\n", threshold);

	uint_t output;
	sscanf(argv[5], "%" PRIdN "", &output);

	int_t n_threads;
	sscanf(argv[6], "%" PRIdN "", &n_threads);
	printf("K: %" PRIdN "\n", K);
	
	#if OMP
		omp_set_num_threads(n_threads);
	
		#pragma omp parallel
		{
		if(omp_get_thread_num()==0)
			printf("N_THREADS: %d\n", omp_get_num_threads());
		}
		printf("N_PROCS: %d\n", omp_get_num_procs());
	#endif

	cout<<"length of all strings N = "<<n<<endl; //" Number of strings K="<<K<<endl;
	printf("sizeof(int): %zu bytes\n", sizeof(int_t));

	#if SAVE_SPACE
		cout<<"SAVE_SPACE"<<endl;
	#endif

 	string dir = "sdsl";
	mkdir(dir.c_str());
    	string id = c_file;
	id += "."+to_string(K);
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

	/********/

	#if SAVE_SPACE
		tVMII result(K);//(K, tVI(K,0));
	#else
		uint_t** result = new uint_t* [K];
		for(unsigned i=0; i<K; ++i){
			result[i] = new uint_t [K];
			for(unsigned j=0; j<K; ++j)
				result[i][j] = 0;
		}
	#endif

	cout<<"computing..."<<endl;

	#if OMP
		total = omp_get_wtime();
		start = omp_get_wtime();
	#endif

    	uint_t *Block =  new uint_t[K];

	//Find position of each Block b_i
	size_t tmp=0; 
	for(size_t i=0;i<sa.size();++i){
		uint_t t = str_int[(sa[i]+n-1)%n];	
		if( t < K ){// found whole string as suffix
			Block[tmp++] = i;
		}
	}

/**/
	uint_t inserts = 0, removes = 0;
	uint_t ov=0;
	uint_t contained=0;
	
	#if OMP
		#pragma omp parallel for reduction(+:inserts) reduction(+:removes) reduction(+:ov) reduction(+:contained) 
	#endif
	for(int_t proc=0; proc < n_threads; proc++){

		uint_t start = proc*(K/n_threads);
		uint_t end = (proc+1)*(K/n_threads)-1;
		if(proc==n_threads-1) end = K-1;

		uint_t partition = end-start+1;

		#pragma omp critical
		cout<<"thread_"<<proc<<"\t"<<start<<"\t"<<end<<"\tsize = "<<partition<<endl;


		uint_t *TOP_l = (uint_t*) malloc(partition*sizeof(uint_t));
		uint_t *TOP_g = (uint_t*) malloc(partition*sizeof(uint_t));
		uint_t *LAST = (uint_t*) malloc(partition*sizeof(uint_t));
	
		uint_t *Overlaps = (uint_t*) malloc(K*sizeof(uint_t));
		uint_t *Min_lcp = (uint_t*) malloc(K*sizeof(uint_t));
	
		for(uint_t p = 0; p < partition; p++){
			TOP_l[p] = TOP_g[p] = UINT_MAX;
			LAST[p]=0;
		}

		for(uint_t p = 0; p < K; p++){
			Min_lcp[p]=Overlaps[p]=0;
		}


		//We preprocess to find the number of overlaps
		uint_t overlaps = 0;
		for(uint_t p = 0; p < K; p++){//all threads scan all SA

			//LOCAL solution:
			uint_t previous =(p>0? Block[p-1]: K+1);
			uint_t min_lcp = UINT_MAX;
			for(uint_t i=Block[p]-1; i>=previous; --i){
				if(min_lcp >= lcp[i+1]){

					min_lcp = lcp[i+1];
					uint_t t = str_int[sa[i]+lcp[i+1]]-1;//current suffix     

					if(t>=start && t<=end)//if the suffix belongs to a string charged by this thread
//					if(t < K)//complete overlap
					if(min_lcp >= threshold){
						Overlaps[p]++;
					}
        			}
			}
			overlaps += Overlaps[p];
			Min_lcp[p] = min_lcp;
		}
		
		overlaps++;
		tLIST *LIST = (tLIST*) malloc(overlaps*sizeof(tLIST));
		uint_t pos=overlaps-1;
	
		LIST[pos].lcp = 0;
		LIST[pos].next = UINT_MAX;
		uint_t min_lcp;
	
		for(uint_t p = 0; p < K; p++){
		
			uint_t ptr;
	
			//LOCAL solution:
			uint_t previous =(p>0? Block[p-1]: K+1);
			uint_t prefix = str_int[(sa[Block[p]]+n-1)%n];
	
			min_lcp = Min_lcp[p];
			
			//removes non-valid overlaps
			while(LIST[pos].lcp > min_lcp){
				pos++;
				removes++;
			}
	
			//updates TOP_g and LAST
			for(uint_t t = 0; t < partition; t++){
			
				while(TOP_g[t]<pos) TOP_g[t] = LIST[TOP_g[t]].next;
				LAST[t] = 0;
				TOP_l[t] = UINT_MAX;
			}
	
			pos -= Overlaps[p];//we know the number of local overlaps
			ptr = pos;
	
			//find the local overlaps
			min_lcp = UINT_MAX;
			for(uint_t i=Block[p]-1; i>=previous; --i){
	
				if(min_lcp >= lcp[i+1]){
	
					min_lcp = lcp[i+1];
					//access T[SA[i]+LCP[i+1]]
					uint_t t = str_int[sa[i]+lcp[i+1]]-1;//current suffix     
	
					if(t>=start && t<=end)//if the suffix belongs to a string charged by this thread
	//				if(t < K)//complete overlap
					if(min_lcp >= threshold){
	
						INSERT(LIST, ptr++, t-start, lcp[i+1], TOP_l, TOP_g, LAST);
						inserts++;
					}
	        		}
			}
			//GLOBAL solution (reusing)	
			for(uint_t t = 0; t < partition; t++){
	
				if(TOP_l[t]!=UINT_MAX) TOP_g[t] = TOP_l[t];//merge local and global
						
				if(TOP_g[t]!=UINT_MAX){
					#if SAVE_SPACE
						if(t+start!=prefix)
					#endif
					#if RESULT
						result[t+start][prefix] = LIST[TOP_g[t]].lcp;
					#else
						result[prefix][t+start] = LIST[TOP_g[t]].lcp; //major-row
					#endif
					ov++;
				}
			}

			#if SAVE_SPACE == 0
				result[p][p] = 0;
			#endif
			//contained suffixes
			uint_t q = Block[p]+1;
			//while(lcp[q] == ms[prefix] and ((tt=str_int[sa[q]+[prefix]]-1) < K ) and q < n ){
			while(str_int[sa[q]+lcp[q]] < K and q < n ){
		
				if(lcp[q] >= threshold){
				
					uint_t tt=str_int[sa[q]+lcp[q]]-1;

					if(tt>=start && tt<=end){//if the suffix belongs to a string charged by this thread
						contained++;
						#if RESULT 
							result[tt][prefix] = lcp[q];
						#else
							result[prefix][tt] = lcp[q];
						#endif
					}
				}
				else 
					break;
				q++;
			}

		}
		free(TOP_l);free(TOP_g);free(LAST);free(LIST);free(Overlaps);free(Min_lcp);
	}	

	#if OMP
		cout<<"## TOTAL"<<endl;
		printf("TIME = %f (in seconds)\n", omp_get_wtime()-total);
		cout<<"##"<<endl;

		fprintf(stderr, "%lf\n", omp_get_wtime()-total);
	#endif

	cout<<"--"<<endl;
   	printf("inserts %" PRIdN "\n", inserts);
	printf("removes %" PRIdN "\n", removes);
        printf("overlaps %" PRIdN " (%" PRIdN ")\n", ov, contained);
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
		for(uint_t i=0; i<10 && i<K; ++i){
			for(uint_t j=0; j<10 && j<K; ++j)
				cout<<result[i][j]<<" ";
			cout<<endl;
		}
		#endif

		if(output==1){
			ofstream out_file(dir+"/output."+id+".new.bin",ios::out | ios::binary);			
			for(uint_t i=0; i<K; ++i)
				for(uint_t j=0; j<K; ++j)
					out_file.write((char*)&result[i][j], sizeof(uint_t));
			out_file.close();
		}

		for(uint_t i=0; i<K; ++i)
			delete[] result[i];
		delete[] result;

	#endif

	delete[] Block;


	return 0;
}

inline void INSERT(tLIST* LIST, int_t ptr, int t, int lcp, uint_t *TOP_l,  uint_t *TOP_g, uint_t* LAST){

	if(TOP_l[t]==UINT_MAX) TOP_l[t] = ptr;

	LIST[ptr].lcp = lcp;
	LIST[ptr].next=TOP_g[t];

	LIST[LAST[t]].next = ptr;

	LAST[t] = ptr;
}

