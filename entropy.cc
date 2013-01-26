#include <fstream>
#include <iostream>
#include <vector>
#include <cmath>

#define rep(i, a, b) for(int i = (a); i < int(b); ++i)
#define trav(it, v) for(typeof((v).begin()) it = (v).begin(); \
    it != (v).end(); ++it)

using namespace std;

//double log2(double n){
//  return log(n)/log(2);
//}

pair<double, unsigned int> entropy(fstream &infile){
  vector<int> v(256);
  unsigned int length = 0;
  while(length < 200000000){
    int c = infile.get();
    if(c == EOF) break;
    ++v[(int)c];
    ++length;
  }

  double H = 0;
  trav(it, v){
    double p = (double)*it/length;
    if(*it) H += (*it) * log2(p);
  }
  H = -H/length;
  return make_pair(H, length);
}


int main(int argc, char *argv[]){
  if(argc < 2){
    cout << "Wrong input argument, use " << argv[0] << " infile1 ... \n";
    return 0;
  }
  fstream infile;
  
  rep(i, 1, argc){
    infile.open(argv[i], fstream::in | fstream::binary);
    pair<double, unsigned int> H = entropy(infile);

    cout << "File: " << argv[i] << "\n Entropy: " << H.first << "\n Length: " << H.second << "\n\n";

    infile.close();
  }
  return 0;
}
