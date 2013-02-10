#include <fstream>
#include <iostream>
#include <queue>
#include <bitset>
#include <map>
#include <inttypes.h>
#include <cstdlib>

#define rep(i, a, b) for(int i = (a); i < int(b); ++i)
#define trav(it, v) for(typeof((v).begin()) it = (v).begin(); \
    it != (v).end(); ++it)

using namespace std;

int const alphabet_size = 256;
int const NOSYMBOL = (int)1e9;

struct FenwickTree{
  int n;
  vector<int> s;
  FenwickTree(int _n) : n(_n) {
    s.assign(n, 0);
  }
  void update(int pos, int dif) {
    for (; pos < n; pos |= pos + 1)
      s[pos] += dif;
  }
  int query(int val) {
    int count = 0;
    for (; val >= 0; val = (val & (val + 1)) - 1)
      count += s[val];
    return count;
  }
};


typedef pair<int, int> Codeword; // first is codeword, second is codeword length.
Codeword make_codeword(int first = 0, int second = 0){
  return make_pair(first, second);
}

class Outfile{
  private:
    fstream &outfile;
    int pos, buffer;
  public:
    Outfile(fstream &outfile): outfile(outfile), pos(0), buffer(0){};
    void put_codeword(Codeword &codeword){
      buffer = (buffer << codeword.second) | codeword.first;
      pos += codeword.second;
      while(pos > 7){
        outfile.put((buffer >> (pos-8)) & 0xff);
        pos = pos - 8;
      }
    }
};

class Frequency{
  private:
    FenwickTree tree;
  public:
    Frequency(): tree(alphabet_size){
      rep(i, 0, alphabet_size){
        tree.update(i, 1);
      }
    }
    uint64_t get_total(){
      return tree.query(alphabet_size-1);
    }
    uint64_t get_symbol_frequency(uint64_t symbol){
      if(symbol == 0) return tree.query(0);
      else return tree.query(symbol) - tree.query(symbol-1); // Can be made faster by writing a specialised function for the fenwick tree.
    }
    uint64_t get_F(char symbol){
      if(symbol == -1) return 0;
      return tree.query(symbol);
    }
    void update_symbol(uint64_t symbol){
      tree.update(symbol, 1);
    }
};


void compress(fstream &infile, fstream &outfile){
  Outfile out(outfile);
  Frequency F;
  uint64_t l = 0, u = (uint32_t)-1;
  int c = 0;
  cerr << hex << u << endl;
  while(1){
    uint64_t l_old = l, u_old = u, t;
    c = infile.get();
    if(!infile) break;
    Codeword codeword = make_codeword();
    l = l_old + (u_old - l_old + 1)*F.get_F(c-1)/F.get_total();
    u = l_old + (u_old - l_old + 1)*F.get_F(c)/F.get_total() - 1;
    //cerr << "F(c-1): " << F.get_F(c-1) << " F(c): " << F.get_F(c) << endl;
    //cerr << "l: " << hex << l << " u: " << hex << u << endl;
    t = l ^ u;
    for(int i = 1; !(t & uint64_t(1) << 63); ++i){
      codeword.first = (codeword.first << 1) + (u >> 63);
      codeword.second = i;
      l = l << 1;
      u = u << 1;
      t = t << 1;
    }
    out.put_codeword(codeword);
  }
}

/*void decompress(fstream &infile, fstream &outfile){
  TreeMap trees;
  trees[0] = new Tree();
//vector<Tree*> trees(alphabet_size*alphabet_size);
//trav(it, trees){
//  *it = new Tree();
//}
Node *cur = trees[0]->root;
uint64_t c1 = 0, c2 = 0, c3 = 0, c4 = 0;
while(1){
//cerr << 0;
int c = infile.get();
//cerr << c;
if(!infile) break;
rep(i, 0, 8){
//cerr << 1;
if((c >> (7-i)) & 1) cur = cur->right;
else cur = cur->left;
if(cur->symbol != NOSYMBOL){
//outfile.put(cur->symbol >> 8);
int t = cur->symbol;
outfile.put(t);
//cerr << (char)t;
update_tree(cur->tree->root, cur->tree->symbol_to_node[t]);
cur = get_tree(trees, (c4 << 32) + (c3 << 24) + (c2 << 16) + (c1 << 8) + t);
c4 = c3;
c3 = c2;
c2 = c1;
c1 = t;
//cerr << 2 << endl;
}
}
}
}*/

int main(int argc, char *argv[]){
  if(argc < 2){
    cout << "Wrong input argument, use " << argv[0] << " [-d] infile.\n";
    return 0;
  }
  fstream infile;
  fstream outfile;
  bool infile_read = false, outfile_read = false, will_decompress = false;
  rep(i, 1, argc){
    if(argv[i][0] == '-'){
      if(argv[i][1] == 'd'){
        will_decompress = true;
      }
      if(argv[i][1] == 'm'){
        //memory = atoi(argv[i+1]);
        ++i;
      }
    }
    else{
      if(!infile_read){
        infile.open(argv[i], fstream::in | fstream::binary);
        if(!infile){
          cerr << "Could not open file " << argv[i] << '\n';
          return 1;
        }
        infile_read = true;
      }
      else if(!outfile_read){
        outfile.open(argv[i], fstream::out | fstream::binary);
        if(!outfile){
          cerr << "Could not open file " << argv[3] << '\n';
          return 1;
        }
      }
    }
  }
  //if(will_decompress)
  //  decompress(infile, outfile);
  //else
    compress(infile, outfile);

  infile.close();
  outfile.close();
  return 0;
}
