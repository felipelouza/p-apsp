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
#include "lib/file.h"

#include <omp.h>

#define DEBUG 0
#define SAVE_SPACE 1 

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

inline void insert(Tl ***next, int n, int value, Tl *sentinel);
inline void remove(Tl **list, int n);
inline void prepend(Tl **list1, Tl **list2, Tl***next, int n, Tl *sentinel);

int main(int argc, char *argv[]){

	uint32_t k = 0;
	uint32_t n = 0;
	uint32_t l = 0;
	vector<uint32_t> ms;  // vector containing the length of the ests

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

	for(uint32_t i=0; i<k; i++){
		uint32_t m = strlen((char*)R[i]);
		ms.push_back(m);
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


    	uint32_t *block = (uint32_t *) malloc(k * sizeof(uint32_t));

	//Find initial position of each Block b_i
	size_t tmp=0; 
	for(size_t i=0;i<sa.size();++i){

		uint32_t t = str_int[(sa[i]+n-1)%n];	
		if( t < k ){// found whole string as suffix
			block[tmp++] = i;
		}
	}


	Tl ***next = (Tl ***) malloc(k * sizeof(Tl**));
	Tl **Ll = (Tl **)  malloc(k * sizeof(Tl*));
	Tl **Lg = (Tl **)  malloc(k * sizeof(Tl*));

	Tl *sentinel = (Tl *) malloc(sizeof(Tl));
	sentinel->value = 0;


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

		Lg[i] = sentinel;
		Ll[i] = sentinel;
		next[i] = Ll+i;
    	}
	
	cout<<"computing.."<<endl;
	//double start = clock();
	time_t t_start = time(NULL);
	clock_t c_start = clock();

	int inserts = 0;
	int removes = 0;
	int contained = 0;

	for(uint32_t p = 0; p < k; ++p){
		
		uint32_t prefix = str_int[(sa[block[p]]+n-1)%n];
		uint32_t min_lcp = UINT_MAX;

		//LOCAL solution:
		uint32_t previous =(p>0? block[p-1]: k+1);
		for(uint32_t i=block[p]-1; i>=previous; --i){

			if(min_lcp >= lcp[i+1]){

				min_lcp = lcp[i+1];
				uint32_t t = str_int[sa[i]+lcp[i+1]]-1;//current suffix     

				if(t < k)//complete overlap
					if(min_lcp >= threshold){
						insert(next, t, lcp[i+1], sentinel); inserts++;
					}
        		}
		}
		//GLOBAL solution (reusing)
		for(uint32_t i = 0; i < k; ++i){
			
			while(Lg[i]->value > min_lcp){
                		remove(Lg, i);
                		removes++;
            		}

			prepend(Lg, Ll, next, i, sentinel);
            
            		if(i!=prefix)
            		#if SAVE_SPACE
				if(Lg[i]->value)
            		#endif
				result[prefix][i] = Lg[i]->value;
			
		}

		//contained suffixes
		uint32_t q = block[p]+1;
		uint32_t tt;
		while(q < n and lcp[q] == ms[prefix] and ((tt=str_int[sa[q]+ms[prefix]]-1) < k ) ){
			if(lcp[q] >= threshold){
				contained++;	
				result[prefix][tt] = lcp[q];
			}
			q++;
		}

	}
	

	cout<<"--"<<endl;
	printf("CLOCK = %lf TIME = %lf (in seconds)\n", (clock() - c_start) / (double)(CLOCKS_PER_SEC), difftime (time(NULL),t_start));
	cout<<"--"<<endl;

   	printf("inserts %i\n", inserts);
	printf("removes %i\n", removes);
	printf("contained %i\n", contained);

	cout<<"--"<<endl;

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
                        ofstream out_file("output."+id+".seq.bin",ios::out | ios::binary);              
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
                        ofstream out_file("output."+id+".seq.bin",ios::out | ios::binary);              
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
	for(uint32_t i = 0; i<k; ++i){
	    Tl *aux = Lg[i];
	    while(aux != sentinel){
	        Lg[i] = Lg[i]->prox;
	        free(aux);
	        aux = Lg[i];
	    }
	}
	
	free(block);
	free(Lg);
	free(Ll);
	free(next);
	free(sentinel);

	return 0;
}

inline void insert(Tl ***next, int n, int value, Tl *sentinel){
    
    Tl *aux;
    aux = (Tl *) malloc(sizeof(*aux));
    aux->value = value;
    aux->prox = sentinel;    
    *(next[n]) = aux;
    next[n] = &(aux->prox);
}    
    
inline void remove(Tl **list, int n){
    
    Tl *aux;
    aux = list[n];
    list[n] = list[n]->prox;
    free(aux);
}

inline void prepend(Tl **list1, Tl **list2, Tl***next, int n, Tl *sentinel){

    *(next[n]) = list1[n];
    list1[n] = list2[n];
    list2[n] = sentinel;
    next[n] = list2 + n;
}
