#include <iostream>
#include <limits.h>
#include <bitset>


using namespace std;

struct bits {
    unsigned int a:1;
    unsigned int b:1;

};

struct bits *mybits; 




int main(){
  
  long i=0;

  cout <<sizeof(i)<<endl;
  i |= 1 <<2;
  cout << (int)i <<endl;
  i |= 1 <<8;
  cout << (int)i <<endl;

  i &= ~(1<<3);
  cout << (int)i <<endl;

  int u = i & (1 << 2);
  cout << u <<endl;
  i |= (1 <<8);
  cout << (int)i <<endl;
  if (i & (1 << 2))
    cout <<"second is set\n";
  
   
if (i & (1 << 30))
    cout <<"thirty is set\n";


 
  return 0;
}
