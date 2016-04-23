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
#include "lib/utils.h"
#include "external/malloc_count/malloc_count.h"

#include <omp.h>

#define DEBUG 0
#define SAVE_SPACE 1
#define OMP 1

//output format [row, column]
#define RESULT 1 // 0 = [prefix, suffix], 1 = [suffix, prefix]

using namespace std;
using namespace sdsl;


typedef struct _list{    
    uint_t value;
    struct _list *prox;
} Tl;

typedef map<uint_t, uint_t> tMII;
typedef vector<tMII> tVMII;

/**/ //SAVE_SPACE
typedef map<uint_t, Tl*> tML;
typedef vector<tML> tVL;

typedef map<uint_t, Tl**> tMLL;
typedef vector<tMLL> tVLL;
/**/

inline void insert(tML& Llocal, tMLL& Next, int_t p, int_t value);
inline void remove(Tl **list);
inline void prepend(Tl **Lg, tMLL& Next, tML& Llocal, int_t p);

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

	tVL Llocal(k);
	tVLL Next(k);


	#if SAVE_SPACE
		tVMII result(k);//(k, tVI(k,0));
	#else
		uint_t** result = new uint_t* [k];
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

	uint_t *Min_lcp = new uint_t[k];

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
						insert(Llocal[t], Next[t], p, lcp[i+1]); 
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

	uint_t overlaps=0;
	//GLOBAL solution (reusing)	
	
	#if OMP
		#pragma omp parallel for reduction(+:removes) reduction(+:overlaps) 
	#endif
	for(uint_t t = 0; t < k; t++){

//	printf("### [%d] = %d ###\n", omp_get_thread_num(), t);

		uint_t min_lcp;
		Tl *Lg = NULL;

		if(!Llocal[t].empty()){

			for(uint_t p = 0; p < k; ++p){
		
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
	
				if(Lg){
				#if SAVE_SPACE
					if(t!=Prefix[p])
				#endif
				#if RESULT
					result[t][Prefix[p]] = Lg->value;
				#else
					result[Prefix[p]][t] = Lg->value;
				#endif
					overlaps++;
				}
	
			}
	
			//free	
			while(Lg!=NULL){
				remove(&Lg);
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
			ofstream out_file(dir+"/output."+id+".par.bin",ios::out | ios::binary);			
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
			ofstream out_file(dir+"/output."+id+".par.bin",ios::out | ios::binary);			
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

inline void insert(tML& Llocal, tMLL& Next,int_t p, int_t value){

	#pragma omp critical
	{
		Tl *aux;
		aux = new Tl;
		aux->value = value;
		aux->prox = NULL;    

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

inline void prepend(Tl **Lg, tMLL& Next, tML& Llocal, int_t p){

	*(Next[p]) = *Lg;
	*Lg = Llocal[p];
	Llocal[p] = NULL;
}

