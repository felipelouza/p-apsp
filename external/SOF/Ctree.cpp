#include <iostream>
#include "Ctree.h"
#include <omp.h>
#define INIT_SHARE 500

using namespace std;



struct tree_node *create_tree(uchar *text,uint k,ulong n, ulong startp[],uint sorted[]){
  struct tree_node *root=create_node(0,0,0);
  for(uint i=0;i<k;i++){
    struct tree_node *curptr=root;
    struct tree_node *parent=root;
    long pos=1;long currpos=0;int choosen_child=1;
    long c= startp[sorted[i]];
    //cout<<"k:"<<i<<endl;
    while (1){
      if (curptr->pos>=pos) { //still in the same node
		  if (/*text[c]*/ decode(text,c)== decode(text, startp[sorted[curptr->from]]+currpos)  /*text[startp[sorted[curptr->from]]+currpos]*/) { 
		  //putchar(text[c]); 
		  //cout<<"startp[sorted[curptr->from]]+currpos"<<startp[curptr->from]+currpos<<endl;				
			  pos++;currpos++;c++;
			  if ( c==startp[sorted[i]+1]   /*text[c]==SEPERATOR*/) break;  /* No need to do anything since the range has been updated */
		  } else {  // Split 
			  //cout<<"enter splitting"<<endl;
			  struct tree_node *temp = create_node(pos-1,curptr->from,curptr->to); //new mother node
			  temp->ptr[get_index(decode(text,c)/*text[c]*/)]= create_node(find_pos(startp,sorted,i,k,n)-currpos,i,i);  //new node 
			  parent->ptr[choosen_child]=temp;
			  //cout<<"choosen_child:"<<choosen_child<<endl;
			  //cout<<"startp[sorted[curptr->from]]+currpos:"<<startp[sorted[curptr->from]]+currpos<<endl;
			  temp->ptr[get_index(decode(text,startp[sorted[curptr->from]]+currpos)  /*text[startp[sorted[curptr->from]]+currpos]*/)]=curptr;
			  curptr->pos= curptr->pos-pos;
			  remove_index(curptr,i);
			  break;
		 }
      } else if (curptr->pos<pos){  // moving to another node either old or new node
			if (curptr->ptr[get_index(decode(text,c) /*text[c]*/)]!=NULL){
			  //cout << "Processing string:" << sorted[i]<<endl;
			  choosen_child=get_index(decode(text,c) /*text[c]*/);
			  parent=curptr;
			  curptr= curptr->ptr[choosen_child];
			  pos=1; currpos++;c++;
			  updateNodeRange(curptr,i);
			  //display_node(curptr);
			  if (c==startp[sorted[i]+1]   /*text[c]==SEPERATOR*/) break;  /* No need to do anything since the range has been updated updated */					
			}else {
			  curptr->ptr[get_index(decode(text,c) /*text[c]*/)]= create_node(find_pos(startp,sorted,i,k,n)-currpos,i,i);
			  //display_node(curptr->ptr[get_index(decode(text,c) /*text[c]*/)]);
			  break;
			}
       }
    }

  }
  return root;
}


/* Stage 1 from method 2: Creating the tree without sorting. no update for the interval is done.*/
struct tree_node *create_tree_modified(uchar *text,uint k,ulong n, ulong startp[],struct stack_node **stacks){
  struct tree_node *root=create_node(0,0,0);
  for(uint i=0;i<k;i++){
    struct tree_node *curptr=root;
    struct tree_node *parent=root;
    ulong pos=1;long currpos=0;int choosen_child=1;
    ulong c= startp[i];
    //cout<<"k:"<<i<<endl;
    while (1){
      if (curptr->pos>=pos) { //still in the same node
			if (decode(text,c) /*text[c]*/==  decode(text,startp[curptr->from]+currpos) && c!=startp[i+1]/* added here recently*/   /*text[startp[curptr->from]+currpos]*/) { 
				//putchar(text[c]); 
				//cout<<"in a node : startp[sorted[curptr->from]]+currpos"<<startp[curptr->from]+currpos<<endl;
				pos++;currpos++;c++;
					
			} else {  // Split 
				//cout<<"enter splitting"<<endl;
				struct tree_node *temp = create_node(pos-1,curptr->from,curptr->to); //new mother node
				if (c==startp[i+1])
					temp->ptr[0] = create_node(find_pos_modified(startp,i,k,n)-currpos,i,i);  //new node for the seperator if end of string is reached
				else
					temp->ptr[get_index(decode(text,c) /*text[c]*/)]= create_node(find_pos_modified(startp,i,k,n)-currpos,i,i);  //new node 
				parent->ptr[choosen_child]=temp;
				//cout<<"choosen_child:"<<choosen_child<<endl;
				//cout<<"startp[sorted[curptr->from]]+currpos:"<<startp[sorted[curptr->from]]+currpos<<endl;
				temp->ptr[get_index(/*text[startp[curptr->from]+currpos]*/ decode(text,startp[curptr->from]+currpos))]=curptr;
				curptr->pos= curptr->pos-pos;
				//cout<<"curpos->"<<curptr->pos<<endl;
				remove_index(curptr,i);
				break;
			}
      }else if (curptr->pos<pos){  // moving to another node either old or new node
				if (curptr->ptr[get_index(decode(text,c)/*text[c]*/)]!=NULL &&  c!=startp[i+1]){
					 choosen_child=get_index(decode(text,c)/*text[c]*/);
					 parent=curptr;
					 curptr= curptr->ptr[choosen_child];
					 //cout << "move toward a child node"<<endl;
					 if (c==startp[i+1]   /*text[c]==SEPERATOR*/){  /* useless now */
						stacks[curptr->from]= create_stack_node(i,stacks[curptr->from]);
						//cout <<"matched 2 " <<curptr->from << " "<<i<<endl;
						break;
					 }
					 pos=1; currpos++;c++;
					
				}else {
				  if (/*text[c]==SEPERATOR*/ c==startp[i+1] && curptr->ptr[1]==NULL && curptr->ptr[2]==NULL && curptr->ptr[3]==NULL && curptr->ptr[4]==NULL){
					stacks[curptr->from]= create_stack_node(i,stacks[curptr->from]);
					//cout <<"create stack node" <<curptr->from << " "<<i<<endl;
					break;
				  }else if (c!=startp[i+1]  && curptr->ptr[1]==NULL && curptr->ptr[2]==NULL && curptr->ptr[3]==NULL && curptr->ptr[4]==NULL && curptr!=root){
					curptr->ptr[get_index(decode(text,c) /*text[c]*/)]= create_node(find_pos_modified(startp,i,k,n)-currpos,i,i);
					curptr->ptr[0]= create_node(0,curptr->from,curptr->to);
					
					//cout<<"create 2 new nodes"<<endl;
					break;
				  }else {
					if (c!=startp[i+1]   /*text[c]==SEPERATOR*/){
						curptr->ptr[get_index(decode(text,c) /*text[c]*/)]= create_node(find_pos_modified(startp,i,k,n)-currpos,i,i);
						//cout<<"create new nodes - not terminator"<<endl;
					}else {
						if (curptr->ptr[0]==NULL){
							curptr->ptr[0]= create_node(0,i,i);
							//cout<<"create new nodes - terminator "<<endl;
						}else {
							stacks[curptr->ptr[0]->from]= create_stack_node(i,stacks[curptr->ptr[0]->from]);
							//cout <<"create stack node 2" <<curptr->from << " "<<i<<endl;
						}
					}
					//display_node(curptr->ptr[get_index(text[c])]);
					
					break;
				  }
			    }
      }
    }

  }
  return root;
}

struct tree_node *create_node(int pos,int from,int to){
  struct tree_node*ptr = new struct tree_node();
  if (pos<0) pos=0;
  ptr->pos=pos;
  ptr->from=from;
  ptr->to=to;
  for(int i=0;i<5;i++)
    ptr->ptr[i]=NULL;
  return ptr;
}

void display_node(struct tree_node* p){
  cout<<"Pos:"<<p->pos<<endl;
  cout<<"From:"<<p->from<<endl;
  cout<<"To:"<<p->to<<endl;
}

void display_tree(struct tree_node* p){

  display_node(p);
  for(int i=0;i<5;i++){
    if (p->ptr[i]!=NULL){
      cout<<"Branch ";
      putchar(get_char(i));
      cout<<endl;
      display_tree(p->ptr[i]);
    }
  }

}


int get_index(uchar c){
  if (c==SEPERATOR)
    return 0;
  else if (c=='A')
    return 1;
  else if (c=='C')
    return 2;
  else if (c=='G')
    return 3;
  else if (c=='T')
    return 4;
  else
    return -1;
}

uchar get_char(int c){
  if (c==0)
    return SEPERATOR;
  else if (c==1)
    return 'A';
  else if (c==2)
    return 'C';
  else if (c==3)
    return 'G';
  else if (c==4)
    return 'T';
  else
    return SEPERATOR;
}

ulong find_pos(ulong startpos[],uint sorted[],int i,ulong k,ulong n){
  if (sorted[i]<k-1)
    return startpos[sorted[i]+1]-startpos[sorted[i]]-1;
  else
    return n-startpos[sorted[i]]-1;
}

ulong find_pos_modified(ulong startpos[],int i,ulong k,ulong n){
  if ((uint)i<k-1)
    return startpos[i+1]-startpos[i]-1;
  else
    return n-startpos[i]-1;
}


void updateNodeRange(struct tree_node *ptr,int i){
  if (ptr->from > i ) ptr->from =i;
  if (ptr->to < i) ptr->to=i;
}

void remove_index(struct tree_node *ptr,int i){
  if (ptr->to==i) ptr->to=i-1;
}



void find_all_pairs(struct tree_node *ptr,uchar *T,ulong N,ulong k,ulong startpos[],uint sorted[],int output
		    ,int threads,int distribution_method,int min){
  int **A;
 
  tVMII result(k);

  /*
  if (output==1){
    A= new int*[k];
    for (uint z=0;z<k;z++)
      A[z]= new int[k];

    for (uint z=0;z<k;z++){
      for (uint z1=0;z1<k;z1++)
	A[z][z1]=0;
    }
  }
  */
 
  // calculate the total load
  double load = 0;
  for(ulong z=0;z<k-1;z++){
    int len=startpos[z+1]-startpos[z]-1;
    //std::cout<<"len:"<<len<<std::endl;
    load+=(len * (len+1)/2);
    //cout<<load<<endl;
  }
  load += (N-startpos[k-1]) *(N-startpos[k-1]-1)/2; 
  double share_p = load/threads;
  //  cout << "Processor Share:"<<share_p<<endl;
  // End calculating



  //Calculating P shares
  double share_temp=0;
  int *start_p = new int[threads];
  int *end_p = new int[threads];
  int p_counter=0;

  start_p[0]=0;
  for(ulong z=0;z<k;z++){
    ulong len=startpos[z+1]-startpos[z]-1;
    if ((share_temp + (len * (len+1)/2)) <=share_p ||
	share_temp==0.0){
      share_temp += (len * (len+1)/2);
      
    }
    else{
      //cout << p_counter<<" "<<share_temp<<endl;
      end_p[p_counter++]=z-1;
      start_p[p_counter]=z;
      share_temp=(len * (len+1)/2);
      if (p_counter==threads-1) break;
    } 
  }


  end_p[p_counter++]=k-1;
  while (p_counter<=threads-1){  //make the rest idle
    
    start_p[p_counter]=k-1;
    end_p[p_counter]=k-1;
    p_counter++;
  }

  //start_p[p_counter]=k-1;
 
 
  
  

  // End Calculating...
  /*
  for(int z=0;z<threads;z++)
    cout<<"Processor:"<<z<<" start:"<<start_p[z]<<" end:"<<end_p[z]<<endl;
  */



  //std::cout<<"load:"<<load<<std::endl;


  double starttime,endtime;
  starttime = omp_get_wtime();

  if (threads==1)
     distribute_seq(threads,T,startpos,sorted,A,result,ptr,k,N,output,min);

  else if (distribution_method==2)
    distribute_k_2(threads,start_p,end_p,T,startpos,sorted,A,result,ptr,k,N,output,min);
  else if (distribution_method==1)
    distribute_k_1(threads,T,startpos,sorted,A,result,ptr,k,N,output,min);
  else
    distribute_k_3(threads,T,startpos,sorted,A,result,ptr,k,N,output,min);


  endtime = omp_get_wtime();  

  cout<<"User Time for finding all pairs using brute force: "<<endtime-starttime<<endl;
  fprintf(stderr, "%lf\n", endtime-starttime);


  if (output==1){
    for (uint z=0;z<k;z++){
      for (uint z1=0;z1<k;z1++){
	if (z==z1)
	  printf("%5d ",0);
	else
	  printf("%5d ",result[z][z1]);
      }
      cout <<endl;
    }
  }	
	
}





/* this function represents stage 2 for method 2 to construct the tree. 
   the purpose is to update all intervals for all nodes in the tree */
void traverse_tree_modified(struct tree_node *ptr,uint sorted [],int *counter,struct stack_node **stacks){
  bool first=false;
  for(int i=0;i<5;i++){
    if (ptr->ptr[i]!=NULL){
      traverse_tree_modified(ptr->ptr[i],sorted,counter,stacks);
      if (!first)			/* Since it is postorder traversal, first child should update its parent interval, the last should update the upper bound */
		 ptr->from=ptr->ptr[i]->from;
      else
		 ptr->to=ptr->ptr[i]->to;
      first=true;
    }
  }

  if (!first){   //if the node is a leaf, we check if there are matching strings.
    sorted[*counter]=ptr->from;
    struct stack_node*temp= stacks[ptr->from]; //the stack ins indexed by the old value of ptr->from.
    ptr->from=*counter;
    *counter=*counter+1;
    while (temp!=NULL){
      sorted[*counter]=temp->value;
      *counter=*counter+1;
      temp=temp->next;
    }
		
    ptr->to=(*counter)-1;
  }
}

struct stack_node* create_stack_node(uint i,struct stack_node *ptr){
  struct stack_node *temp=new struct stack_node();
	
  temp->value=i;
  if (ptr!=NULL)
    temp->next=ptr;
  else
    temp->next=NULL;

  return temp;
}

void find_all_pairs_modified(struct tree_node *ptr,uchar *T,ulong N,ulong k,ulong startpos[],uint sorted[],int output){
  int **A;

  tVMII result(k);

  /*
  if (output==1){
    A= new int*[k];
    for (uint z=0;z<k;z++)
      A[z]= new int[k];
    
    for (uint z=0;z<k;z++){
      for (uint z1=0;z1<k;z1++)
	A[z][z1]=0;
    }
  }
  */

  sdsl::stop_watch stopwatch;
  stopwatch.start();

  for(int i=1;i<5;i++){
    if (ptr->ptr[i]!=NULL)	traversal1(ptr->ptr[i],A, result,ptr,T,0,0,startpos,sorted,N,k,output);
  }

  stopwatch.stop();  
  cout<<"User Time for finding all pairs using three-pointers method: "<<stopwatch.getUserTime()<<" ms"<<endl;
  fprintf(stderr, "%lf\n", stopwatch.getUserTime());

  if (output==1){
    for (uint z=0;z<k;z++){
      for (uint z1=0;z1<k;z1++){
	if (z==z1)
	  printf("%5d ",0);
	else
//	  printf("%5d ",A[z][z1]);
	  printf("%5d ",result[z][z1]);
      }
      cout <<endl;
    }
  }

}




void traversal1(struct tree_node *ptr,int **A, tVMII& result,struct tree_node *root,uchar *T,int curpos,int curpos2,ulong startpos[],uint sorted[],ulong N,ulong k,int output){
	
  //cout<<"Outer from:"<<ptr->from<<" to:"<<ptr->to<< endl;
  for(int i=0;i<=ptr->pos;i++){
    traversal2(root->ptr[get_index(T[startpos[sorted[ptr->from]]+curpos])],0,ptr,i,T,curpos,curpos2,k,N,startpos,sorted,A, result,output);
    curpos++;
  }

  for(int n=1;n<=4;n++){
    if (ptr->ptr[n]!=NULL) traversal1(ptr->ptr[n],A,result,root,T,curpos,0,startpos,sorted,N,k,output);
  }
}

void traversal2(struct tree_node* ptr2,int i2,struct tree_node *ptr1,int i1,uchar * T,int curpos,int curpos2,uint k,int N,ulong startpos[],uint sorted[],int **A, tVMII& result,int output){
	
  if (ptr2==NULL) return;
  for (int n=i1;n<=ptr1->pos;n++){
    //cout <<"inner helper :ptr1 "<<ptr1->from <<"-"<<ptr1->to<<" ptr2:"<<ptr2->from <<"-"<<ptr2->to<<" i1="<<n<<" i2="<<i2<< " curpos1:"<<curpos<<" curpos2:"<<curpos2<<endl;	
    if (i2<=ptr2->pos){
      if (T[startpos[sorted[ptr1->from]]+curpos] != T[startpos[sorted[ptr2->from]]+curpos2])
	return;
    }else{
      ptr2=ptr2->ptr[get_index(T[startpos[sorted[ptr1->from]]+curpos])];			
      i2=0;
      if (ptr2==NULL) return;
    }
    curpos++;curpos2++;i2++;
  }


  for(int n=1;n<=4;n++){
    if (ptr1->ptr[n]!=NULL){
      traversal2(ptr2,i2,ptr1->ptr[n],0,T,curpos,curpos2,k,N,startpos,sorted,A,result,output);
    }
  }


  if (ptr1->ptr[1]==NULL && ptr1->ptr[2]==NULL && ptr1->ptr[3]==NULL && ptr1->ptr[4]==NULL){  //report a match.
    //cout<< "Report a match!"<<endl;
    for(int z=ptr1->from;z<=ptr1->to;z++){
      //cout << "curpos2:" <<curpos2<<endl;
//      if (output==1){
	for(int z1=ptr2->from;z1<=ptr2->to;z1++){ 
/*	  if (A[sorted[z]][sorted[z1]]==0 && z1!=z){
	    //cout << "set value "<<z1<<" "<<z<<endl; 
	    A[sorted[z]][sorted[z1]]= curpos2;
	  }
*/
	  if (result[sorted[z]][sorted[z1]]==0 && z1!=z){
	    //cout << "set value "<<z1<<" "<<z<<endl; 
	    result[sorted[z]][sorted[z1]]= curpos2;
	  }
	}
//      }
    }
    return;
  }	

}


void distribute_k_2(int threads,int start_p[],int end_p[],uchar *T,ulong startpos[],uint sorted[],int **A, tVMII& result,struct  tree_node* ptr,ulong k,ulong N,int output,int min)
{
  omp_set_num_threads(threads);
long int total=0;
#pragma omp parallel reduction(+:total) 
  {
    int pid= omp_get_thread_num();
    for(uint i=start_p[pid];i<=end_p[pid];i++){
      uint j=startpos[i];
      uint next=startpos[i+1];

      while (min<= next-j){	//T[j]!=SEPERATOR
		int v=j;struct tree_node *curptr=ptr;int pos=1;int curpos=0;
		while (1){
			  if (v==next){  // There is a match
				total += do_output_all_results(A,result, sorted,i,next-j,curptr,output,startpos);
				break;
			  }

			  if (pos<=curptr->pos){
				if (decode(T,v)==/*T[startpos[sorted[curptr->from]]+curpos]*/ decode(T,startpos[sorted[curptr->from]]+curpos)){
					pos++;curpos++;v++;
				}else
					break;
			  }else {
					if (curptr->ptr[get_index(/*T[v]*/ decode(T,v))]!=NULL){
						curptr=curptr->ptr[get_index(decode(T,v))];
						pos=1;curpos++;v++;
					}else 
						break;
			  }
		}
	    j++;
     }
   }
 }

fprintf(stdout, "overlaps = %llu\n", total);
}


/* this method will simply divide strings between processors */

void distribute_k_1(int threads,uchar *T,ulong startpos[],uint sorted[],int **A, tVMII& result,struct  tree_node* ptr,ulong k,ulong N,int output,int min)
{
  omp_set_num_threads(threads);

long int total=0;  
#pragma omp parallel for reduction(+:total)
  for(ulong i=0;i<k;i++){
    ulong j=startpos[i];
    ulong next=startpos[i+1];

    while (min<=next-j){	//T[j]!=SEPERATOR
      ulong v=j;struct tree_node *curptr=ptr;ulong pos=1;ulong curpos=0;
      while (1){
		if (/*T[v]==SEPERATOR*/ v==next){  // There is a match
		  total += do_output_all_results(A,result,sorted,i,next-j,curptr,output,startpos);
		  break;
	    }

		if (pos<=curptr->pos){
			if (/*T[v]==T[startpos[sorted[curptr->from]]+curpos]*/ decode(T,v)==decode(T,startpos[sorted[curptr->from]]+curpos)){
				pos++;curpos++;v++;
			}else
				break;
		}else {
			if (curptr->ptr[get_index(decode(T,v) /*T[v]*/)]!=NULL){
				curptr=curptr->ptr[get_index(decode(T,v) /*T[v]*/)];
				pos=1;curpos++;v++;
			}else 
				break;
		}
      }
      j++;
    }
  }
fprintf(stdout, "overlaps = %llu\n", total);
}


/* this method will simply divide strings between processors */

void distribute_k_3(int threads,uchar *T,ulong startpos[],uint sorted[],int **A, tVMII& result,struct  tree_node* ptr,ulong k,ulong N,int output,int min)
{

  omp_set_num_threads(threads);
  int start=-1 * INIT_SHARE;
  uint end;
long int total=0;  
#pragma omp parallel reduction(+:total)
//#pragma omp parallel 
  { 


    while (end<k){
#pragma omp critical 
      {
	start+= INIT_SHARE;
	end = start + INIT_SHARE;
	if (end>=k) end=k;
      }


      for(uint i=start;i<end;i++){
	uint j=startpos[i];
	uint next=startpos[i+1];

	while (min<=next-j){	//T[j]!=SEPERATOR
	  int v=j;struct tree_node *curptr=ptr;int pos=1;int curpos=0;
	  while (1){
	    if (v==next){  // There is a match
	      total += do_output_all_results(A,result,sorted,i,next-j,curptr,output,startpos);
	      break;
	    }

	    if (pos<=curptr->pos){
	      if (/*T[v]==T[startpos[sorted[curptr->from]]+curpos]*/ decode(T,v)==decode(T,startpos[sorted[curptr->from]]+curpos)){
			pos++;curpos++;v++;
	      }else
			break;
	    }else {
	      if (curptr->ptr[get_index(/*T[v]*/decode(T,v))]!=NULL){
				curptr=curptr->ptr[get_index(/*T[v]*/ decode(T,v))];
				pos=1;curpos++;v++;
	      }else 
		break;
	    }
	  }
	  j++;
	}
      }


    }
  }
fprintf(stdout, "overlaps = %llu\n", total);

}

void distribute_seq(int threads,uchar *T,ulong startpos[],uint sorted[],int **A, tVMII& result,struct  tree_node* ptr,ulong k,ulong N,int output,int min)
{

  
long int total=0;  

  for(ulong i=0;i<k;i++){
    ulong j=startpos[i];
    ulong next=startpos[i+1];

    while (min<=next-j){	//T[j]!=SEPERATOR
      ulong v=j;struct tree_node *curptr=ptr;ulong pos=1;ulong curpos=0;
      while (1){
	if (v==next){  // There is a match
	  total += do_output_all_results(A,result,sorted,i,next-j,curptr,output,startpos);
	  break;
	}

	if (pos<=curptr->pos){
	  if (decode(T,v)==decode(T,startpos[sorted[curptr->from]]+curpos)){
	    pos++;curpos++;v++;
	  }else
	    break;
	}else {
	  if (curptr->ptr[get_index(decode(T,v))]!=NULL){
	    curptr=curptr->ptr[get_index(decode(T,v))];
	    pos=1;curpos++;v++;
	  }else 
	    break;
	}
      }
      j++;
    }
  }

fprintf(stdout, "overlaps = %llu\n", total);
}




long int do_output_all_results(int **A, tVMII& result,uint sorted[],int i,int value,struct tree_node *curptr,int output,ulong startpos[]){
int z=0;
long int total=0;
   if (output==1){
     for(z=curptr->from;z<=curptr->to;z++){
//       if (A[i][sorted[z]]==0 && value<=(startpos[sorted[z]+1]-startpos[sorted[z]]))   /* this will work because if the value is not 0, it will be the maximum suffix prefix match. */
//			A[i][sorted[z]]= value;
       if (result[i][sorted[z]]==0 && value<=(startpos[sorted[z]+1]-startpos[sorted[z]]))   /* this will work because if the value is not 0, it will be the maximum suffix prefix match. */
			result[i][sorted[z]]= value;
     }
   }
// else
   if (output==2){
     for(z=curptr->from;z<=curptr->to;z++){
		if (value>0 && i!=sorted[z] && value<=(startpos[sorted[z]+1]-startpos[sorted[z]])){
			fprintf(stderr, "%d %d ---> %d\n", i,sorted[z], value);
			total++;
		}
	 }
   }
   if (output==3){//only counts the # of overlaps
     for(z=curptr->from;z<=curptr->to;z++){
		if (value>0 && i!=sorted[z] && value<=(startpos[sorted[z]+1]-startpos[sorted[z]])){
			total++;
		}
	 }
   }
return total;
}

char decode(uchar *final,ulong pos){
	ulong bbyte = pos/4;
	ulong bbit = (pos%4)*2+1;
	uchar c = final[bbyte];
	if (!(final[bbyte] & (1 << bbit)) && !(final[bbyte] & (1 << ((bbit+1)%8)))) return 'A' ;
	if ((final[bbyte] & (1 << bbit)) && !(final[bbyte] & (1 << ((bbit+1)%8)))) return 'C' ;
	if (!(final[bbyte] & (1 << bbit)) && (final[bbyte] & (1 << ((bbit+1)%8)))) return 'G' ;
	if ((final[bbyte] & (1 << bbit)) && (final[bbyte] & (1 << ((bbit+1)%8)))) return 'T' ;
	return 'y';
}
