#ifndef _CTREE_
#define _CTREE_

#include <omp.h>
#include "Tools.h"
#include "testutils.hpp"
#define SEPERATOR '\n'


struct tree_node{
	int pos;
	int from;
	int to;
	struct tree_node *ptr[5];
};

struct stack_node{
	int value;
	struct stack_node *next;
};


struct tree_node *create_tree(uchar *text,uint k,ulong n,ulong startp[],uint sorted[]);
struct tree_node *create_tree_modified(uchar *text,uint k,ulong n,ulong startp[],struct stack_node **stacks);

struct tree_node *create_node(int pos,int from,int to);
struct stack_node* create_stack_node(uint i,struct stack_node*ptr);

int get_index(uchar c);
void display_node(struct tree_node* p);
ulong find_pos(ulong startpos[],uint sorted[], int i,ulong k,ulong n);
ulong find_pos_modified(ulong startpos[], int i,ulong k,ulong n);
void traverse_tree_modified(struct tree_node *ptr,uint sorted [],int *counter,struct stack_node **stacks);
void updateNodeRange(struct tree_node *ptr,int i);
void display_tree(struct tree_node* p);
uchar get_char(int c);
void remove_index(struct tree_node *ptr,int i);
void find_all_pairs(struct tree_node *ptr,uchar *T,ulong N,ulong K,ulong startpos[],uint sorted[],
			int output,int threads,int distribution_method,int min);
//void find_all_pairs_no_output(struct tree_node *ptr,uchar *T,ulong N,ulong K,ulong startpos[],uint sorted[]);
void find_all_pairs_modified(struct tree_node *ptr,uchar *T,ulong N,ulong k,ulong startpos[],uint sorted[],int output);
//void find_all_pairs_no_output_modified(struct tree_node *ptr,uchar *T,ulong N,ulong k,ulong startpos[],uint sorted[]);
void traversal_modified(struct tree_node *ptr,int i,int **A,struct tree_node *root,uchar *T,int curpos,int curpos2,ulong startpos[],uint sorted[],ulong N,ulong k);
void traversal_helper_modified(struct tree_node* ptr2,int i2,struct tree_node *ptr1,int i1,uchar * T,int curpos,int curpos2,uint k,int N,ulong startpos[],uint sorted[],int **A);
void traversal1(struct tree_node *ptr,int **A,struct tree_node *root,uchar *T,int curpos,int curpos2,ulong startpos[],uint sorted[],ulong N,ulong k,int output);
//void traversal1_no_output(struct tree_node *ptr,int **A,struct tree_node *root,uchar *T,int curpos,int curpos2,ulong startpos[],uint sorted[],ulong N,ulong k);
void traversal2(struct tree_node* ptr2,int i2,struct tree_node *ptr1,int i1,uchar * T,int curpos,int curpos2,uint k,int N,ulong startpos[],uint sorted[],int **A,int output);
//void traversal2_no_output(struct tree_node* ptr2,int i2,struct tree_node *ptr1,int i1,uchar * T,int curpos,int curpos2,uint k,int N,ulong startpos[],uint sorted[],int **A);
void distribute_k_2(int threads,int start_p[],int end_p[],uchar *T,ulong startpos[],uint sorted[],int **A,struct  tree_node* ptr,ulong k,ulong N,int output,int min);
void distribute_k_1(int threads,uchar *T,ulong startpos[],uint sorted[],int **A,struct  tree_node* ptr,ulong k,ulong N,int output,int min);
void distribute_k_3(int threads,uchar *T,ulong startpos[],uint sorted[],int **A,struct  tree_node* ptr,ulong k,ulong N,int output,int min);
void distribute_seq(int threads,uchar *T,ulong startpos[],uint sorted[],int **A,struct  tree_node* ptr,ulong k,ulong N,int output,int min);

void do_output_all_results(int **A,uint sorted[],int i, int value, struct tree_node *curptr,int output,ulong startpos[]);
char decode(uchar *final,ulong pos);
#endif
