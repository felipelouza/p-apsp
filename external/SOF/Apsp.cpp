
#include <iostream>

#include <omp.h>
#include "stdlib.h"
#include "Tools.h"
#include "testutils.hpp"
#include "Ctree.h"
#include <stdio.h>
#include <string.h>
#include "../malloc_count/malloc_count.h"



#define MAX_K 1000000
#define MAX_N 1000000000
#define BUFFER 10


using namespace std;

void displayhelp();
void print_Sorted(uchar *a,uint *So);
void print(uchar *a);
bool isTerminator(uchar i);
bool isTerminatorfile(uchar i);
int choose_index(int a);
void msd(uchar *a, int l, int r, int d, uint *So);
void display_choices(char *filename, char *output, char* sorting,int threads,int min,int distribution_method);
void encode(uchar *final,ulong counter,int bitnum,char c);
ulong N;
uint K;
ulong *startS;




void APSP(char *filename, char *output, char* sorting,int threads,int min,int distribution_method)
{
    


  // N is the total length of the all strings. we have k strings.   
  uint *Sorted= new uint[MAX_K];
  uchar *text;
  std::ifstream::pos_type posSize;
  std::ifstream file ((char *)filename, std::ios::in|std::ios::binary|std::ios::ate);
  ulong counter=0,bitnum=1,reminder=0,counterk=0,pos=0;
  // counterk is to count strings, counter to count the size of text (without the seperators), pos to get the current pos in final array.
  startS = new ulong[MAX_K];
  struct stack_node **matched = new stack_node*[MAX_K];    /* used for matched strings in the modified tree */
  startS[0]=0;
 

  if (file.is_open())
  {
        posSize = file.tellg();
        ulong size = posSize;
        if (MAX_N != 0 && size > MAX_N)
            size = MAX_N;
        char *memblock = new char [size/BUFFER + 1];
		text = new uchar[size/4];
		reminder = size%BUFFER;

		for(int i=0;i<BUFFER;i++){
			file.seekg (i*(size/BUFFER), std::ios::beg);
			file.read (memblock, size/BUFFER);
			for(ulong z=0;z<size/BUFFER;z++){
				if (memblock[z]!=SEPERATOR){
					//cout <<"encoding:"<<memblock[z]<<" pos:"<<pos<<endl;
					encode(text,pos,bitnum,memblock[z]);
					if (memblock[z]=='A' || memblock[z]=='C' || memblock[z]=='G' || memblock[z]=='T') {
						counter++;
						bitnum+=2;
						//cout<<"bitnum now "<<bitnum<<endl;
						if (bitnum==9) {
							bitnum=1;
							pos++;
						}
					}
				}else {
					startS[++counterk]=counter;
					Sorted[counterk-1]=counterk-1;
					matched[counterk-1]=NULL;
				}
			}
		}

		//cout <<"reminder:"<<reminder<<endl;
		if (reminder>0){
			file.seekg (BUFFER*(size/BUFFER), std::ios::beg);
			file.read (memblock, reminder);
			for(ulong z=0;z<reminder;z++){
				if (memblock[z]!=SEPERATOR){
					encode(text,pos,bitnum,memblock[z]);
					if (memblock[z]=='A' || memblock[z]=='C' || memblock[z]=='G' || memblock[z]=='T') {
						counter++;
						bitnum+=2;
						if (bitnum==9) {
							bitnum=1;
							pos++;
						}
					}
				}else {
					startS[++counterk]=counter;
					Sorted[counterk-1]=counterk-1;
					matched[counterk-1]=NULL;
				}
			}
		}

		startS[counterk]=counter;
		Sorted[counterk]=counterk;
		matched[counterk]=NULL;
		
		N = counter;
		file.close();
		cout<<"Size of File:" << size << endl;
		cout<<"Number of Strings: K "<< counterk<<endl;
		cout <<"Size of strings:" << counter <<endl;
		K=counterk;

		/* decoding code
		for(int i=0;i<counterk;i++){
			//cout << "start" <<i<< ":"<<startS[i]<<endl;
			for(ulong z=startS[i];z<startS[i+1];z++){				
				char v= decode(text,z);
				//cout<<"decoding byte "<<bbyte<<" bit "<<bbit<<" "<<v <<endl;
				//cout<<v;
			}
			//cout <<SEPERATOR;
		}*/

		
  }

  
  
  /*
    for(uint u=0;u<K;u++)
    cout<<"Sorted:"<<Sorted[u]<<endl; */
  struct tree_node *ptr;
	
  sdsl::stop_watch stopwatch;

  if (sorting[0]=='1') {
    stopwatch.start();
    msd(text,0, K, 0,Sorted);
    stopwatch.stop();  
    cout<<"User Time for Sorting: "<<stopwatch.getUserTime()<<" ms"<<endl;
    //print(text);
    //print_Sorted(text,Sorted);
    stopwatch.start();
    cout<<"Creating tree After Sorting: "<<endl;
    ptr=create_tree(text,K,N,startS,Sorted);
	
    stopwatch.stop();
    cout<<"User Time for Constructing tree: "<<stopwatch.getUserTime()<<" ms"<<endl;
	
    //cout<<"displaying tree "<<endl;
    //display_tree(ptr);
  }else {

    sdsl::stop_watch stopwatch;
    stopwatch.start();
    cout<<"Creating tree: "<<endl;
	
    ptr=create_tree_modified(text,K,N,startS,matched);
    int counter1=0;
    traverse_tree_modified(ptr,Sorted,&counter1,matched);
    stopwatch.stop();
    cout<<"User Time for Constructing tree with no sorting required: "<<stopwatch.getUserTime()<<" ms"<<endl;
    //cout<<"displaying tree: "<<endl;
    //display_tree(ptr);
    //print(text);
    //print_Sorted(text,Sorted);
  }
  

  //  if (method[0]=='0'){
  find_all_pairs(ptr,text,N,K,startS,Sorted,atoi(output),threads,distribution_method,min);
    // }else {
    //find_all_pairs_modified(ptr,text,N,K,startS,Sorted,atoi(output));
    //}


  cout<<"Press any key to continue: "<<endl;
//  cin >>N;

  // Clean up    
  delete [] startS;
    
}

int main(int argc,char *argv[])
{

 char *filename;
 char *sorting=new char[2];
 char *output=new char[2];
  int method,processors,minlength;
  // Default values 
  
  processors = omp_get_num_procs();
  minlength = 1;
  output[0] = '0';
  sorting[0]='0';
  
  if (argc<2){
      cout <<"wrong number of arguments, run Apsp to get a help"<<endl;
      displayhelp();
      return 0;
  }
  
  filename = argv[1];
  method = 1;
  

  for(int i=2;i<argc;i++){
    if (argv[i][0]=='-'){
      if (argv[i][1]=='p')
	processors = atoi(argv[i+1]);
      else if (argv[i][1]=='m')
	minlength=atoi(argv[i+1]);
      else if (argv[i][1]=='d')
	method=atoi(argv[i+1]);
      else if (argv[i][1]=='o')
	output=argv[i+1];
      else if (argv[i][1]=='s')
	sorting=argv[i+1];
    }
  }

  display_choices(filename,output,sorting,processors,minlength,method);

  /*
  if (argc!=6){
    cout <<"wrong number of arguments"<<endl;
    cout <<"Apsp filename output sorting threads distribution_method"<<endl;
    return 0;
    }*/
  
  APSP(filename,output,sorting,processors,minlength,method);
  return 0;
}


void msd(uchar *a, int l, int r, int d, uint *So)
{
  int count[5]; 
  int *temp = new int[K];
	

  //cout << "l , r :"<<l<< " "<< r <<endl;
  if (r <= l+1) {
    delete temp;
    return;
  }

  for (int i=0;i<5;i++)
    count[i]=0;
	
  for (int i = l; i < r; i++){
    if (startS[So[i]]+ d >= N)
      count[0]++;
    else if (startS[So[i]]+ d < startS[So[i]+1]/*-1*/) {
	  //cout << "decode(a,startS[So[i]]+d):"<<decode(a,startS[So[i]]+d) << endl;
      count[choose_index(decode(a,startS[So[i]]+d)/*a[startS[So[i]]+d]*/)]++;
	} else
      count[0]++;
  }

  for (int k = 1; k <5 ; k++)
    count[k] += count[k-1];
	
  /*
  for (int k = 0; k <5 ; k++)
    cout<<"count k Before:"<<count[k]<<endl;
	*/

  for (int i = l; i < r; i++){
    //cout<<"choose:"<<choose_index(a[startS[So[i]]+d])<<endl;
    if (startS[So[i]]+ d >= N) {
      temp[count[0]-1+l]=So[i];
      count[0]--;
    }
    else if (startS[So[i]]+ d < startS[So[i]+1]/*-1*/){
      temp[count[choose_index(/*a[startS[So[i]]+d]*/ decode(a, startS[So[i]]+d))]-1+l] = So[i];
      count[choose_index(/*a[startS[So[i]]+d]*/  decode(a, startS[So[i]]+d))]--;
      //cout <<"i , So[i]:" << i <<" "<<So[i]<<endl;
    } else {
      temp[count[0]-1+l]=So[i];
      count[0]--;
    }
  }


  for (int i=  l; i < r; i++)
    So[i] = temp[i];
	
  /*
    for (int i = l; i < r; i++)
    cout<<"So After:"<<So[i]<<endl;

    for (int k = 0; k <5 ; k++)
    cout<<"count k After:"<<count[k]<<endl; 
  */
  delete temp;

	


  //	cout <<"-------------"<<endl;
  for (int i = 1; i < 4; i++){
    msd(a,  l+ count[i],  l+ count[i+1], d+1,So);
  }

	
  msd(a,  l+ count[4],  r, d+1,So);
	
}


int choose_index(int a){
  if (a=='A') 
    return 1;
  else if (a=='C')
    return 2;
  else if (a=='G')
    return 3;
  else if (a=='T')
    return 4;
  else
    return 0;
}




bool isTerminatorfile(uchar i){

  return (i==SEPERATOR);
}

void print(uchar *a){
  int con=0;
  int strs=0;
  while (a[con]!='\0'){
    if (a[con]=='#'){ 
      cout<<" "<<strs<<endl;strs++; 
    }else 
      putchar(a[con]);
    con++;
  }

  cout<<"------------" <<endl;
}

void print_Sorted(uchar *a,uint * So){
  int con=0;

  for(uint i=0;i<K;i++){
    con=0;
    while (startS[So[i]] + con  /*a[startS[So[i]]]*/< startS[So[i]+1])    /*a[startS[So[i]+1]]*/{
      putchar(decode(a,startS[So[i]]+con)   /*a[startS[So[i]]+con]*/);
      con++;
    } 
    cout << " "  <<endl;
  }

  cout<<"------------" <<endl;
}



void displayhelp(){
  cout<<"*******************************************************"<<endl;
  cout<<"This program has one parameter and four optional parameters:"<<endl;
  cout<<"Apsp_first filename [method] [number_of_processors] [output] [minimum_length]"<<endl;
  cout<<"filename\t is the name of the data file."<<endl;
  cout<<"-d\tdistribution method. Default value is 1."<<endl;
  cout<<"-p\tnumber of processors(threads). Default value is the maximum."<<endl;
  cout<<"-o\toutput:\n\t 1 (default) for producing an output in an array.\n\t 0 for executing without an output.\n\t 2 for outputing all overlaps."<<endl;
  cout<<"-m\tminimum length for matching, 1 is the default value."<<endl;
  cout<<"*******************************************************"<<endl;
}


void display_choices(char *filename, char *output, char* sorting,int threads,int min,int distribution_method){

  cout<<"These are your choices:"<<endl;
  cout<<"File name:"<<filename<<endl;
  cout<<"Output:";
  if (output[0]=='0')
    cout<<" No Output" <<endl;
  else if (output[0]=='1')
    cout<<" Maximum suffix-prefix matches are shown in two dimentional array."<<endl;
  else
    cout<<" All overlaps are shown "<<endl;


  cout<<"Sorting: ";
  if (sorting[0]=='0')
    cout <<" No Sorting"<<endl;
  else 
    cout <<" with sorting"<<endl;

  cout<<"Number of threads:"<<threads<<endl;
  cout<<"Minimum Match Length:"<<min<<endl;
  cout<<"Distribution Method:";
 
  if (distribution_method==1)
    cout<<" Divide k strings equally between the processors"<<endl;
  else if (distribution_method==2)
    cout<<" Estimate an approximately equal share for each processor."<<endl;
  else
    cout<<" Each processor starts with an initial share."<<endl;
   cout<<"----------------------"<<endl;
}

void encode(uchar *final,ulong counter,int bitnum, char c){
	if (c=='A'){
		final[counter] &= ~(1<<bitnum);
		final[counter] &= ~(1<<((bitnum+1)%8));
	}else if (c=='C'){
		final[counter] |= (1<<bitnum);
		final[counter] &= ~(1<<((bitnum+1)%8));
	}else if (c=='G'){
		final[counter] &= ~(1<<bitnum);
		final[counter] |= (1<<((bitnum+1)%8));
	}else if (c=='T'){
		final[counter] |= (1<<bitnum);
		final[counter] |= (1<<((bitnum+1)%8));
	}
}


