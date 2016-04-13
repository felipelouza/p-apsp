/*
* Felipe A. Louza
* 25 fev 2015
*
* New algorithm to solve the apsp problem
*/

#include <sdsl/construct.hpp>
#include <sdsl/int_vector.hpp>
#include <sdsl/config.hpp>
#include <sdsl/construct_config.hpp>
#include <iostream>
#include <cassert>
#include <climits>
#include "external/malloc_count/malloc_count.h"

#include <omp.h>

#define DEBUG 1
#define SAVE_SPACE 0 

//output format [row, column]
#define RESULT 1 // 0 = [prefix, suffix], 1 = [suffix, prefix]

using namespace std;
using namespace sdsl;

const uint64_t buflen = (1<<28); 
char buf[buflen];

typedef struct _list{    
    uint32_t value;
    struct _list *prox;
} Tl;

typedef map<uint32_t, uint32_t> tMII;
typedef vector<tMII> tVMII;

inline int top(Tl **list, int n);
inline void insert(Tl ***next, int n, int value, Tl *sentinel);
inline void remove(Tl **list);
inline void prepend(Tl **list1, Tl **list2, Tl***next, int n);

int main(int argc, char *argv[]){

	uint32_t k = 0;
	uint32_t m = 0;
	uint32_t l = 0;
	vector<uint32_t> str; // vector for the text 
	vector<uint32_t> ms;  // vector containing the length of the ests

	if(argc!=6)
		return 1;

	ifstream f_input(argv[1]); //input file
	if(!f_input.is_open()) return 1;

	uint32_t n_strings;
    	sscanf(argv[2], "%u", &n_strings);

	uint32_t threshold;
	sscanf(argv[3], "%u", &threshold);

	int n_threads;
	sscanf(argv[5], "%d", &n_threads);
	printf("K: %d\n", n_strings);

	omp_set_num_threads(n_threads);

	#pragma omp parallel
	{
	if(omp_get_thread_num()==0)
		printf("N_THREADS: %d\n", omp_get_num_threads());
	}

	printf("N_PROCS: %d\n", omp_get_num_procs());

	while(f_input.getline(buf, buflen-1) ){
		if(k>0 and buf[0]=='>'){
			if(k>=n_strings)
				break;
			str.push_back(k);
			ms.push_back(l);
			l=0;
		}	
		if(buf[0]>='A' and buf[0]<='Z'){
			if( l==0 )
				++k;
			uint64_t i=0;
			while( buf[i]!='\0' ){
				str.push_back(buf[i]-'A');
				++i;
			}
			l += i;
		}
	}
	ms.push_back(l);
	str.push_back(k);

	f_input.close();
	cerr<<"input successfully read"<<endl;

	for(uint32_t i=0,j=0; i<str.size(); ++i, ++j){
		uint32_t f=i;
		while(i-f < ms[j]){
			str[i]+= k+1;	
			++i;
		}
	}

	/*	
	for(uint32_t i=0; i<str.size(); ++i)
		cout<<str[i]<<' ';
	cout<<endl;
	*/

	cout<<"length of all ESTS m="<<str.size()<<" Number of ESTS k="<<k<<endl;
	#if SAVE_SPACE
		cout<<"SAVE_SPACE"<<endl;
	#endif

	str.push_back(0);
	m = str.size();

	int_vector<> str_int(m);
	for(uint32_t i=0; i<m; i++)
		str_int[i] =  str[i];

 	string dir = ".";
    	string id = "tmp";
    	cache_config m_config(true, dir, id);

	store_to_cache(str_int, conf::KEY_TEXT_INT, m_config);

	construct_sa<0>(m_config);
	construct_lcp_PHI<0>(m_config);
	cout<<"constructed enhanced suffix array"<<endl;

	int_vector<> sa;
	int_vector<> lcp;

	load_from_cache(sa, conf::KEY_SA, m_config);
	load_from_cache(lcp, conf::KEY_LCP, m_config);

//FELIPE
	/**/


    	uint32_t *block = (uint32_t *) malloc(k * sizeof(uint32_t));
    	uint32_t *Prefix = (uint32_t *) malloc(k * sizeof(uint32_t));

	//Find initial position of each Block b_i
	size_t tmp=0; 
	for(size_t i=0;i<sa.size();++i){

		uint32_t t = str_int[(sa[i]+m-1)%m];	
		if( t < k ){// found whole string as suffix
			block[tmp] = i;
			Prefix[tmp] = t;
			tmp++;
		}
	}


//	Tl ***next = (Tl ***) malloc(k * sizeof(Tl**));
//	Tl **Ll = (Tl **)  malloc(k * sizeof(Tl*));
//	Tl **Lg = (Tl **)  malloc(k * sizeof(Tl*));

	Tl *sentinel = (Tl *) malloc(sizeof(Tl));
	sentinel->value = 0;
	sentinel->prox = NULL;

//FELIPE
//	Tl *Llocal[k][k];
//	Tl *Lglobal[k];
//	Tl **Next[k][k];

	Tl ***Llocal = (Tl ***)  malloc(k * sizeof(Tl**));
	Tl **Lglobal = (Tl **)   malloc(k * sizeof(Tl*));
	Tl ****Next   = (Tl ****)  malloc(k * sizeof(Tl***));

	for(uint32_t i = 0; i < k; i++){
		Llocal[i] = (Tl **)  malloc(k * sizeof(Tl**));
		Next[i] = (Tl ***)  malloc(k * sizeof(Tl***));
	}

//FELIPE

	#if SAVE_SPACE
		tVMII result(k);//(k, tVI(k,0));
	#else
		uint32_t** result  = (uint32_t**) malloc(k * sizeof(uint32_t*));
		for(unsigned i=0; i<k; ++i){
			result[i]  = (uint32_t *) malloc(k * sizeof(uint32_t));
			for(unsigned j=0; j<k; ++j)
				result[i][j] = 0;
		}
	#endif

	for(uint32_t i = 0; i < k; i++){

//		Lg[i] = sentinel;
//		Ll[i] = sentinel;
//		next[i] = Ll+i;
		Lglobal[i] = sentinel;

		for(uint32_t j = 0; j < k; j++){
			Llocal[i][j] =  sentinel;
			Next[i][j] = &Llocal[i][j];
		}

    	}
	
	cout<<"computing.."<<endl;

	time_t t_total = time(NULL);
	clock_t c_total = clock();

	//double start = clock();
	time_t t_start = time(NULL);
	clock_t c_start = clock();

	int inserts = 0;
	int removes = 0;

// FELIPE	
//	# pragma omp parallel num_threads(n_threads)

//FELIPE
	uint32_t *Min_lcp = (uint32_t*) malloc(k*sizeof(uint32_t));


	#pragma omp parallel 
	#pragma omp for //reduction(+:inserts)
	for(int32_t p = k-1; p >= 0; p--){
	
//		printf("### [%d] = %d ###\n", omp_get_thread_num(), p);

//		uint32_t prefix = str_int[(sa[block[p]]+m-1)%m];
//		uint32_t prefix = Prefix[p];
		uint32_t min_lcp = UINT_MAX;
//		Min_lcp[p] = UINT_MAX;

		//LOCAL solution:
		uint32_t previous =(p>0? block[p-1]: k+1);
		for(uint32_t i=block[p]-1; i>=previous; --i){

			if(min_lcp >= lcp[i+1]){

				min_lcp = lcp[i+1];
				uint32_t t = str_int[sa[i]+lcp[i+1]]-1;//current suffix     

				if(t < k)//complete overlap
					if(min_lcp >= threshold){
//						insert(next, t, lcp[i+1], sentinel); inserts++;
						insert(Next[t], p, lcp[i+1], sentinel); inserts++;
					}
        		}
		}
		Min_lcp[p] = min_lcp;
	}

	cout<<"--"<<endl;
	printf("CLOCK = %lf TIME = %lf (in seconds)\n", (clock() - c_start) / (double)(CLOCKS_PER_SEC), difftime (time(NULL),t_start));
	cout<<"--"<<endl;

	t_start = time(NULL);
	c_start = clock();

	//GLOBAL solution (reusing)
	
	#pragma omp parallel //reduction(+:removes)
	{
	
		if(omp_get_thread_num()==0)
			printf("N_THREADS: %d\n", omp_get_num_threads());
	
		#pragma omp for //nowait 
		for(uint32_t t = 0; t < k; t++){

	//	printf("### [%d] = %d ###\n", omp_get_thread_num(), t);
		
			uint32_t min_lcp;
			Tl *Lg = sentinel;

			for(uint32_t p = 0; p < k; ++p){
		
	//			uint32_t prefix = str_int[(sa[block[p]]+m-1)%m];
				min_lcp = Min_lcp[p];
				while(Lg->value > min_lcp){
		        		remove(&Lg);
	//		       		removes++;
		        	}
		
				prepend(&Lg, Llocal[t], Next[t], p);

				Llocal[t][p]->value = Lg->value;

				//uint32_t prefix = Prefix[p];
			}
		}

		#pragma omp for //nowait 
		for(uint32_t t = 0; t < k; t++){

			for(uint32_t p = 0; p < k; ++p){

		        	#if SAVE_SPACE
					if(Llocal[t][p]->value)
		        	#endif
		        		if(t!=Prefix[p])
					#if RESULT
						result[t][Prefix[p]] = Llocal[t][p]->value;
					#else
						result[Prefix[p]][t] = Llocal[t][p]->value;
					#endif
			}
		}
	}

	cout<<"--"<<endl;
	printf("CLOCK = %lf TIME = %lf (in seconds)\n", (clock() - c_start) / (double)(CLOCKS_PER_SEC), difftime (time(NULL),t_start));
	cout<<"--"<<endl;

	t_start = time(NULL);
	c_start = clock();

	//contained suffixes
	#pragma omp parallel for 
	for(uint32_t p = 0; p < k; p++){
		uint32_t prefix = Prefix[p];

		//contained suffixes
		uint32_t q = block[p]+1;
		uint32_t tt;
		while(q < m and lcp[q] == ms[prefix] and ((tt=str_int[sa[q]+ms[prefix]]-1) < k ) ){
			if(lcp[q] >= threshold)
				#if RESULT 
					result[tt][prefix] = lcp[q];
				#else
					result[prefix][tt] = lcp[q];
				#endif
			q++;
		}

	}

	cout<<"--"<<endl;
	printf("CLOCK = %lf TIME = %lf (in seconds)\n", (clock() - c_start) / (double)(CLOCKS_PER_SEC), difftime (time(NULL),t_start));
	cout<<"--"<<endl;

	cout<<"##"<<endl;
	printf("CLOCK = %lf TIME = %lf (in seconds)\n", (clock() - c_total) / (double)(CLOCKS_PER_SEC), difftime (time(NULL),t_total));
	cout<<"##"<<endl;

   	printf("inserts %i\n", inserts);
	printf("removes %i\n", removes);

	cout<<"--"<<endl;

	uint32_t output;
	sscanf(argv[4], "%u", &output);

	#if SAVE_SPACE
		
		/**/
		uint32_t i = 0;
		for (tVMII::iterator it_row=result.begin(); it_row!=result.end(); ++it_row){			
			uint32_t j = 0;
			for(tMII::iterator it_column=it_row->begin(); it_column!=it_row->end(); ++it_column){
				cout<<it_column->second<<" ";
				if(j++ > 10) break;
			}
			if(it_row->begin()!=it_row->end()){
				cout<<endl;		
				if(i++ > 10) break;
			}
		}
		/**/

		if(output==1){
			ofstream out_file("results_new.bin",ios::out | ios::binary);			
			for (tVMII::iterator it_row=result.begin(); it_row!=result.end(); ++it_row)
				for(tMII::iterator it_column=it_row->begin(); it_column!=it_row->end(); ++it_column)
					out_file.write((char*)&it_column->second, sizeof(uint32_t));
			out_file.close();
		}
	#else

		for(uint32_t i=0; i<10 && i<k; ++i){
			for(uint32_t j=0; j<10 && j<k; ++j)
				cout<<result[i][j]<<" ";
			cout<<endl;
		}

		if(output==1){
			ofstream out_file("results_new.bin",ios::out | ios::binary);
			for(uint32_t i=0; i<k; ++i)
				for(uint32_t j=0; j<k; ++j)
					out_file.write((char*)&result[i][j], sizeof(uint32_t));
			out_file.close();
		}

		for(uint32_t i=0; i<k; ++i)
			free(result[i]);
		free(result);

	#endif

	//free Lists
/*
	for(uint32_t i = 0; i<k; ++i){
	    Tl *aux = Lg[i];
	    while(aux != sentinel){
	        Lg[i] = Lg[i]->prox;
	        free(aux);
	        aux = Lg[i];
	    }
	}
*/	
	free(block);
//	free(Lg);
//	free(Ll);
//	free(next);
	free(sentinel);

	return 0;
}

inline int top(Tl **list, int n){
    return list[n]->value;
}

inline void insert(Tl ***next, int n, int value, Tl *sentinel){
    
    Tl *aux;
    aux = (Tl *) malloc(sizeof(*aux));
    aux->value = value;
    aux->prox = sentinel;    
    *(next[n]) = aux;
    next[n] = &(aux->prox);
}    
    
inline void remove(Tl **list){
    
    Tl *aux;
    aux = *list;
    *list = (*list)->prox;
    free(aux);
}

inline void prepend(Tl **list1, Tl **list2, Tl***next, int n){

    *(next[n]) = *list1;
    *list1 = list2[n];
    //list2[n] = sentinel;
    next[n] = list2 + n;
}
