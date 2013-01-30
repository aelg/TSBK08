#include <fstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <deque>
#include <cmath>

#define rep(i, a, b) for(int i = (a); i < int(b); ++i)
#define trav(it, v) for(typeof((v).begin()) it = (v).begin(); \
    it != (v).end(); ++it)

using namespace std;

//double log2(double n){
//  return log(n)/log(2);
//}

typedef map<deque<unsigned char>, int> State_map;
typedef vector<State_map> Freq_vector;
typedef vector<deque<unsigned char> > State_vector;
typedef pair<int, vector<double> > Entropy_pair;

unsigned int const max_size = 3000000; // ~700 MB apparently

Entropy_pair entropy(fstream &infile, int max_memory){
  int depth = max_memory+1;
  unsigned int size = 0;
  Freq_vector freq(depth);
  State_vector state(depth);
  unsigned int length = 0;
  //while(1){
  while(length < 10000000){ // for testing with /dev/urandom.
    int c = infile.get();
    if(c == EOF) break;
    rep(i, 0, depth){
      state[i].push_front(c);
      if(state[i].size() > (unsigned int) i+1){
        state[i].pop_back();

        State_map::iterator it = freq[i].find(state[i]);
        if(it == freq[i].end()){
          freq[i][state[i]] = 1;
          ++size;
        }

        else ++it->second;
      }
    }
    ++length;
    if(length % 100000 == 0) cerr << "Length: " << length << '\n';
    if(size > max_size){
      --depth;
      size = size - freq[depth].size();
      freq[depth].clear();
      cerr << "New depth: " << depth << '\n';
    }
  }

  vector<double> H(depth);
  rep(i, 0, depth){
    trav(it, freq[i]){
      double p = (double)it->second/(length-i);
      if(it->second) H[i] += p * log2(p);
    }
    H[i] = -H[i];
  }
  return make_pair(length, H);
}


int main(int argc, char *argv[]){
  if(argc < 2){
    cout << "Wrong input argument, use " << argv[0] << " infile1 ... \n";
    return 0;
  }
  fstream infile;
  
  rep(i, 1, argc){
    infile.open(argv[i], fstream::in | fstream::binary);
    int memory = 3;

    cerr << "File: " << argv[i] << "\n";
    
    Entropy_pair res = entropy(infile, memory);
    cout << "File: " << argv[i] << "\n Length: " << res.first << "\n";
    cout << " " << left << setw(14) << "Memory (k)" << setw(24) << "Entropy H(Xn,...,Xn+k)"
         << setw(30) << "Entropy H(Xn+k|Xn,...Xn+k-1)" << setw(15) << "Max compression\n";
    rep(j, 0, res.second.size()){
      cout << " " << setw(14) << j << setw(24) << res.second[j] << setw(30) 
           << (j>0?res.second[j] - res.second[j-1]:res.second[0]) << setw(15) 
           << (j>0?res.second[j] - res.second[j-1]:res.second[0])/8 << "\n";
    }
    cout << '\n';

    infile.close();
  }
  return 0;
}
