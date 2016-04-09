/* written by Maan Haj Rachid */
#include <iostream>
#include <stdio.h>
#include <cstdlib>

#define SEPERATOR '\n'
using namespace std;


char choose_char();



int main(int argc, char *argv[]){
  FILE *f1,*f2;
  f1= fopen(argv[1],"r");
  f2= fopen(argv[2],"w");


  char i;
  int choice;
  bool first=false;
  int counter=0;
  int k=0;

  do {
    i=fgetc(f1);
    if (i=='>'){
      if (first && counter>0){
	fputc(SEPERATOR,f2);
	k++;
	/*	if (k==200){
	  fclose(f1);
	  fclose(f2);
	  return 0;
	  }*/
      }
      first=true;
      counter=0;
      choice=1;   // wait for \n
    }else if (i=='\n' && choice==1){
      choice=2;   // wait for char
    }else if ((i==65 || i==67 || i==71 || i==84) && choice==2){
      fputc(i,f2);
      counter++;
    }else if ((i==78) && choice==2){      
	fputc(choose_char(),f2);
	counter++;
    }
  }while (i!=EOF);
  fputc(SEPERATOR,f2);
  fclose(f1);
  fclose(f2);
  cout<<"k:"<<k+1<<endl;
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