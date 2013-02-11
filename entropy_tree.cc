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

unsigned int const max_size = 1000000; // ~70 MB apparently

struct Node{
  int freq;
  map<int, Node> childs;
  Node():freq(1){}
};

int calc_H(map<int, Node>::iterator &it, vector<double> &H, int max_depth, int depth = 0){
  if(depth == max_depth) return it->freq;
  double h = 0;
  trav(iter, it->childs){


Entropy_pair entropy(fstream &infile, int max_memory){
  int depth = max_memory+1;
  vector<unsigned int> size(depth);
  unsigned int length = 0;
  map<int, Node> m;
  m[0] = Node();
  deque<int> d;
  while(1){
  //while(length < 1000000){ // for testing with /dev/urandom.
    int c = infile.get();
    if(c == EOF && d.empty()) break;
    if(c != EOF) d.push_back(c);
    while(d.size() > (unsigned int) depth+1 || c == EOF){
      map<int, Node>::iterator iter, last_iter = m.begin();
      int i = 0;
      trav(it, d){
        iter = last_iter->second.childs.find(*it);
        if(iter == last_iter->second.childs.end()){
          iter = last_iter->second.childs.insert(last_iter->second.childs.begin(), make_pair(*it, Node()));
          ++size[i];
          if(size[i] > max_size) --depth;
        }
        else ++iter->second.freq;
        last_iter = iter;
      }
      d.pop_front();
    }
    ++length;
    if(length % 100000 == 0) cerr << "Length: " << length << '\n';
    if(d.empty()) break;
  }

  vector<double> H(depth);
  calc_H(m.begin(), H, depth);
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
    int memory = 10;

    cerr << "File: " << argv[i] <<  "\n";
    
    Entropy_pair res = entropy(infile, memory);
    cout << "File: " << argv[i] << "\n Length: " << res.first << "\n";
    cout << " " << left << setw(7) << "Memory" << setw(10) << "Entropy" << setw(15) << "Max compression\n";
    rep(j, 0, res.second.size()){
      cout << " " << setw(7) << j << setw(10) << res.second[j] << setw(15) << res.second[j]/8 << "\n";
    }
    cout << '\n';

    infile.close();
  }
  return 0;
}
