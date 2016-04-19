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
#include "lib/file.h"
#include "external/malloc_count/malloc_count.h"

#include <omp.h>

#define DEBUG 0
#define SAVE_SPACE 1
#define OMP 1

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

/**/ //SAVE_SPACE
typedef map<uint32_t, Tl*> tML;
typedef vector<tML> tVL;

typedef map<uint32_t, Tl**> tMLL;
typedef vector<tMLL> tVLL;
/**/

inline void insert(tML& Llocal, tMLL& Next, int p, int value);
inline void remove(Tl **list);
inline void prepend(Tl **Lg, tMLL& Next, tML& Llocal, int p);

double start, total;

int main(int argc, char *argv[]){

	uint32_t k = 0;
	uint32_t n = 0;
	uint32_t l = 0;

	if(argc!=7)
		return 1;

	char* c_dir = argv[1];
	char* c_file = argv[2];

    	sscanf(argv[3], "%u", &k);

	file_chdir(c_dir);
	unsigned char **R = (unsigned char**) file_load_multiple(c_file, k, &n);
	if(!R){
		fprintf(stderr, "Error: less than %d strings in %s\n", k, c_file);
		return 0;
	}

	n++;
	int_vector<> str_int(n);
	//vector<uint32_t> ms;  // vector containing the length of the ests

	for(uint32_t i=0; i<k; i++){
		uint32_t m = strlen((char*)R[i]);
		//ms.push_back(m);
		for(uint32_t j=0; j<m; j++)
			str_int[l++] = R[i][j]+(k+1)-'A';
		str_int[l++] = i+1; //add $_i as separator
	}	
	str_int[l]=0;

	
	//free memory
	for(uint32_t i=0; i<k; i++)
		free(R[i]);
	free(R);

	uint32_t threshold;
	sscanf(argv[4], "%u", &threshold);

	uint32_t output;
	sscanf(argv[5], "%u", &output);

	int n_threads;
	sscanf(argv[6], "%d", &n_threads);
	printf("K: %d\n", k);

	#if OMP
		omp_set_num_threads(n_threads);
	
		#pragma omp parallel
		{
		if(omp_get_thread_num()==0)
			printf("N_THREADS: %d\n", omp_get_num_threads());
		}
		printf("N_PROCS: %d\n", omp_get_num_procs());
	#endif

	cout<<"length of all strings n = "<<n<<endl; //" Number of strings k="<<k<<endl;
	#if SAVE_SPACE
		cout<<"SAVE_SPACE"<<endl;
	#endif

 	string dir = ".";
    	string id = "tmp."+to_string(k);
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

    	uint32_t *Block =  new uint32_t[k];
    	uint32_t *Prefix = new uint32_t[k];

	//Find initial position of each Block b_i
	size_t tmp=0; 
	for(size_t i=0;i<sa.size();++i){

		uint32_t t = str_int[(sa[i]+n-1)%n];	
		if( t < k ){// found whole string as suffix
			Block[tmp] = i;
			Prefix[tmp] = t;
			tmp++;
		}
	}

	tVL Llocal(k);
	tVLL Next(k);


	#if SAVE_SPACE
		tVMII result(k);//(k, tVI(k,0));
	#else
		uint32_t** result = new uint32_t* [k];
		for(unsigned i=0; i<k; ++i){
			result[i] = new uint32_t [k];
			for(unsigned j=0; j<k; ++j)
				result[i][j] = 0;
		}
	#endif

	cout<<"computing.."<<endl;

	#if OMP
		total = omp_get_wtime();
		start = omp_get_wtime();
	#endif

	int inserts = 0, removes = 0;

	uint32_t *Min_lcp = new uint32_t[k];

	#if OMP
		#pragma omp parallel for reduction(+:inserts)
	#endif
	for(uint32_t p = 0; p < k; p++){
	
		uint32_t min_lcp = UINT_MAX;

		//LOCAL solution:
		uint32_t previous =(p>0? Block[p-1]: k+1);
		for(uint32_t i=Block[p]-1; i>=previous; --i){

			if(min_lcp >= lcp[i+1]){

				min_lcp = lcp[i+1];
				uint32_t t = str_int[sa[i]+lcp[i+1]]-1;//current suffix     

				if(t < k)//complete overlap
					if(min_lcp >= threshold){
						insert(Llocal[t], Next[t], p, lcp[i+1]); inserts++;
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

	//GLOBAL solution (reusing)	
	
	#if OMP
		#pragma omp parallel for reduction(+:removes) 
	#endif
	for(uint32_t t = 0; t < k; t++){

//	printf("### [%d] = %d ###\n", omp_get_thread_num(), t);

		uint32_t min_lcp;
		Tl *Lg = NULL;

		for(uint32_t p = 0; p < k; ++p){
	
			min_lcp = Min_lcp[p];
				
			while(Lg!=NULL){
				if(Lg->value > min_lcp) remove(&Lg);
				else break;
				removes++;
	        	}
	
			if(Llocal[t].find(p)!=Llocal[t].end()){

				if(Lg) prepend(&Lg, Next[t], Llocal[t], p);
				else Lg = Llocal[t][p];
			}

			if(Lg)
			#if SAVE_SPACE
				if(t!=Prefix[p])
			#endif
			#if RESULT
				result[t][Prefix[p]] = Lg->value;
			#else
				result[Prefix[p]][t] = Lg->value;
			#endif

		}

		//free	
		while(Lg!=NULL){
			remove(&Lg);
	        }
	}

	#if OMP
	cout<<"--"<<endl;
	printf("TIME = %f (in seconds)\n", omp_get_wtime()-start);
	start = omp_get_wtime();
	#endif	

	int contained=0;
	//contained suffixes
	#if OMP
		#pragma omp parallel for reduction(+:contained) //firstprivate(threshold,n,k) 
	#endif
	for(uint32_t p = 0; p < k; p++){

		#if SAVE_SPACE == 0
			result[p][p] = 0;
		#endif

		uint32_t q = Block[p]+1;
		//while(lcp[q] == ms[Prefix[p]] and ((tt=str_int[sa[q]+[Prefix[p]]]-1) < k ) and q < n ){
		while(str_int[sa[q]+lcp[q]] < k and q < n ){
	
			if(lcp[q] >= threshold){
			
				uint32_t tt=str_int[sa[q]+lcp[q]]-1;
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

		cout<<"##"<<endl;
		printf("TIME = %f (in seconds)\n", omp_get_wtime()-total);
		cout<<"##"<<endl;
	#endif

   	printf("inserts %i\n", inserts);
	printf("removes %i\n", removes);
   	printf("contained %i\n", contained);

	cout<<"--"<<endl;


	#if SAVE_SPACE
		
		#if DEBUG
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
		#endif

		if(output==1){
			ofstream out_file("output."+id+".par.bin",ios::out | ios::binary);			
			for (tVMII::iterator it_row=result.begin(); it_row!=result.end(); ++it_row)
				for(tMII::iterator it_column=it_row->begin(); it_column!=it_row->end(); ++it_column)
					out_file.write((char*)&it_column->second, sizeof(uint32_t));
			out_file.close();
		}
	#else

		#if DEBUG
		for(uint32_t i=0; i<10 && i<k; ++i){
			for(uint32_t j=0; j<10 && j<k; ++j)
				cout<<result[i][j]<<" ";
			cout<<endl;
		}
		#endif

		if(output==1){
			ofstream out_file("output."+id+".par.bin",ios::out | ios::binary);			
			for(uint32_t i=0; i<k; ++i)
				for(uint32_t j=0; j<k; ++j)
					out_file.write((char*)&result[i][j], sizeof(uint32_t));
			out_file.close();
		}

		for(uint32_t i=0; i<k; ++i)
			delete[] result[i];
		delete[] result;

	#endif

	delete[] Block;
	delete[] Prefix;
	delete[] Min_lcp;

	return 0;
}

inline void insert(tML& Llocal, tMLL& Next,int p, int value){

	Tl *aux;
   	aux = new Tl;
   	aux->value = value;
    	aux->prox = NULL;    

	#pragma omp critical
	{
		if(Next.find(p)!=Next.end() ){
	
			*(Next[p]) = aux;
		}
		else{ //first element
	
			Llocal[p] = aux;
		}
		Next[p] = &(aux->prox);
	}
}

inline void remove(Tl **list){
    
    Tl *aux = *list;
    *list = (*list)->prox;
    delete aux;
}

inline void prepend(Tl **Lg, tMLL& Next, tML& Llocal, int p){

    *(Next[p]) = *Lg;
    *Lg = Llocal[p];
    Llocal[p] = NULL;
}

