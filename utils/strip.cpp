/*
* Felipe A. Louza
* 03 dez 2015
*
*/
#include <iostream>
#include <fstream>

using namespace std;

const int buflen = (1<<28); 
char buf[buflen];

int main(int argc, char *argv[]){

	int k=0, l = 0;

	if(argc!=3)
		return 1;

	ifstream f_input(argv[1]); //input file
	if(!f_input.is_open()) return 1;

	int n_strings;
	sscanf(argv[2], "%u", &n_strings);

	while( f_input.getline(buf, buflen-1) ){
		if(buf[0]=='>'){
			if(k>=n_strings)
				break;
			l=0;
			cout<<buf<<endl;
		}	
		else if(buf[0]>='A' and buf[0]<='Z'){
			if( l==0 )
				++k;
			int i=0, j=0;
			while( buf[i]!='\0' ){
				// strip the N's
				if(buf[i]=='A' || buf[i]=='C' || buf[i]=='G' || buf[i]=='T'){
					cout<<buf[i];
					++j;
				}
				++i;
			}
				cout<<endl;
			l += j;
		}
		else cout<<endl;
	}
	f_input.close();

return 0;
}
