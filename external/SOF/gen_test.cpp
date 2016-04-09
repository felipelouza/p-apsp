/* written by Maan Haj Rachid */

#include <iostream>
#include <cstdlib>
#include <string.h>
#include <stdio.h>

using namespace std;

char choose_terminator();
char choose_char();
char counter=0;

int main(){
  unsigned long  k,n;
  char equal;
  cout <<"Enter the number of strings:";
  cin>>k;
  cout<<"Enter the size of the whole string:";
  cin>>n;
  cout <<"Equal Length?(y or n)";
  cin >>equal;
  FILE *fp,*fp1;
  fp=fopen("test.txt", "w");
  fp1=fopen("test.fasta","w");
  srand(time(NULL));
  long wk=0,countk=0;
  fputc('>',fp1);
  fputc('\n',fp1);
  for(long i=1;i<=n;i++){
    if (equal=='y'){ 
      if (i%(n/k)==0){
	char temp=choose_terminator();
	fputc(temp,fp);
	fputc('\n',fp1);
	fputc('\n',fp1);
	if (i<n){
	  fputc('>',fp1);
	  fputc('\n',fp1);
	}
      } else{
	char temp=choose_char();
	fputc(temp,fp);
	fputc(temp,fp1);
      }
    }else{
      wk++;
	  int h;
	  unsigned long  term;
	  do {
		h =rand() % (n/k);
		term=((wk-1)*(n/k)) + 1 +  h;    /* random size for each string */
	  }while (h==0);
      //      cout <<term << " " << h <<" "<<wk*n<<endl;
      if (countk==k-1){
	while (i<n){
	  char temp=choose_char();
	  fputc(temp,fp);
	  fputc(temp,fp1);
	  i++;
	}
      }else {
	while (i<term){
	  char temp=choose_char();
	  fputc(temp,fp);
	  fputc(temp,fp1);
	  i++;
	}
      }
      //      cout <<"Terminator :"<<i<<endl;
      fputc(choose_terminator(),fp);
      if (countk<k-1){
	fputc('\n',fp1);
	fputc('\n',fp1);
	fputc('>',fp1);
	fputc('\n',fp1);
      }
      countk++;
    }
  }
  fclose(fp);
  fclose(fp1);
  return 0;


}

char choose_char(){
  //  srand(time(NULL));
  char z= rand()%4;
  if (z==0) 
    return 'A';
  else if (z==1)
    return 'C';
  else if (z==2)
    return 'G';
  else
    return 'T';
}

char choose_terminator(){

  return '\n'; //counter+32;
}
