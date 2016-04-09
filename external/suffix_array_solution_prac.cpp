/*
* Felipe A. Louza
* 23 fev 2015
*
* Copied from www.uni-ulm.de/in/theo/research/seqana.html
* Updated to run using sdls-lite v.2
*/

#include <sdsl/construct.hpp>
#include <sdsl/int_vector.hpp>
#include <sdsl/config.hpp>
#include <sdsl/construct_config.hpp>
#include <iostream>
#include <vector>
#include <list>
#include <stack>
#include <cassert>
#include <map>
#include "malloc_count/malloc_count.h"

#define DEBUG 0
#define SAVE_SPACE 1

using namespace std;
using namespace sdsl;

using namespace std::chrono;
using timer = std::chrono::high_resolution_clock;

typedef vector<tLI> tVLI;
typedef stack<uint32_t> tSI;
typedef vector<tSI> tVSI;
typedef map<uint32_t, uint32_t> tMII;
typedef vector<tMII> tVMII;

const uint64_t buflen = (1<<28); 
char buf[buflen];

int main(int argc, char *argv[]){

	uint32_t k = 0;
	uint32_t m = 0;
	uint32_t l = 0;
	vector<uint32_t> str; // vector for the text 
	vector<uint32_t> ms;  // vector containing the length of the ests

	if(argc!=5)
		return 1;

	ifstream f_input(argv[1]); //input file
	if(!f_input.is_open()) return 1;

	uint32_t n_strings;
	sscanf(argv[2], "%u", &n_strings);

	uint32_t threshold;
	sscanf(argv[3], "%u", &threshold);

	while( f_input.getline(buf, buflen-1) ){
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
	//register_cache_file(conf::KEY_TEXT_INT, m_config);

	construct_sa<0>(m_config);
	construct_lcp_PHI<0>(m_config);
	cout<<"constructed enhanced suffix array"<<endl;

	int_vector<> sa;
	int_vector<> lcp;

	load_from_cache(sa, conf::KEY_SA, m_config);
	load_from_cache(lcp, conf::KEY_LCP, m_config);

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

	cout<<"computing.."<<endl;
	//double start = clock();
	time_t t_start = time(NULL);
	clock_t c_start = clock();

	uint32_t max_lcp=0;
	for(size_t i=0;i<sa.size();++i){
		if(max_lcp<lcp[i])
			max_lcp = lcp[i];
	}

	tVSI stacks(k+1);
	tVLI lists(max_lcp+1);

    	int inserts = 0;
    	int removes = 0;

	tSI lcps;

	for(uint32_t i=k+1; i<m; ++i){

		#if DEBUG
			uint32_t l=sa[i];
			cout<<i<<"|\t"<<sa[i]<<"|\t";
			while(str_int[l]>k)
				printf("%d ", str_int[l++]);
			cout<<"#_"<<str_int[l++];
		#endif

		uint32_t t = str_int[(sa[i]+m-1)%m];	
		if( t < k ){// found whole string as suffix

			for(uint32_t j=0; j<k; ++j){
				if(j!=t){
					if(!stacks[j].empty()){
						if( stacks[j].top() >= threshold )
						result[t][j] = stacks[j].top();
					}
				}
			}
			uint32_t q = i+1;
			uint32_t tt;
			while(q < m and lcp[q] == ms[t] and ((tt=str_int[sa[q]+ms[t]]-1) < k ) ){
				if( ms[t] >= threshold )
					result[t][tt] = ms[t];
				q=q+1;
			}
		}
		if(i<m-1){
			if(lcp[i+1]<lcp[i]){
				uint32_t l;
				while(!lcps.empty() and (l=lcps.top())>lcp[i+1] ){
					while(lists[l].size() > 0 ){
						stacks[lists[l].back()].pop();
						lists[l].pop_back();removes++;
					}
					lcps.pop();
				}
			}else if( (t=str_int[sa[i]+lcp[i+1]]-1) < k ){

				stacks[t].push(lcp[i+1]);inserts++;
				lists[lcp[i+1]].push_back(t);
				if(lcps.empty() or lcp[i+1]>lcps.top() )
					lcps.push(lcp[i+1]);
			}	
		}
	}

	cout<<"--"<<endl;
	printf("CLOCK = %lf TIME = %lf (in seconds)\n", (clock() - c_start) / (double)(CLOCKS_PER_SEC), difftime (time(NULL),t_start));
	cout<<"--"<<endl;


	printf("inserts %i\n", inserts);
	printf("removes %i\n", removes);

	cout<<"--"<<endl;

	uint32_t output;
	sscanf(argv[4], "%u", &output);


	#if SAVE_SPACE

		/*
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
		*/

		if(output==1){
			ofstream out_file("results_suff.bin",ios::out | ios::binary);			
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
			ofstream out_file("results_suff.bin",ios::out | ios::binary);
			for(uint32_t i=0; i<k; ++i)
				for(uint32_t j=0; j<k; ++j)
					out_file.write((char*)&result[i][j], sizeof(uint32_t));
			out_file.close();
		}

		for(uint32_t i=0; i<k; ++i)
			free(result[i]);
		free(result);

	#endif

	return 0;
}
