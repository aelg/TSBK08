#include <fstream>
#include <iostream>
#include <queue>
#include <bitset>
#include <map>
#include <inttypes.h>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define rep(i, a, b) for(int i = (a); i < int(b); ++i)
#define trav(it, v) for(typeof((v).begin()) it = (v).begin(); \
    it != (v).end(); ++it)

using namespace std;

int const alphabet_size = 256;
int const NOSYMBOL = (int)1e9;

struct FenwickTree{
  uint32_t n, bitmask;
  vector<uint32_t> s;
  FenwickTree(uint32_t _n) : n(_n), bitmask(1) {
    while((bitmask<<1) < n) bitmask <<= 1;
    //n = bitmask;
    //s.assign(bitmask, 0);
    s.assign(n, 0);
  }
  void update(uint32_t pos, int dif) {
    for (; pos < n; pos |= pos + 1)
      s[pos] += dif;
  }
  int query(int val) {
    int count = 0;
    for (; val >= 0; val = (val & (val + 1)) - 1)
      count += s[val];
    return count;
  }
  int find(uint32_t val){
    if(val == 0) return 0;
    uint32_t pos = 0;
    uint32_t bm = bitmask;
    while(pos < n && bm != 0){
      uint32_t tpos = pos | (bm-1);
      if(tpos < n && s[tpos] <= val){
        pos = tpos + 1;
        val -= s[tpos];
      }
      bm >>= 1;
    }
    if(pos >= n) return pos = n-1;
    else return pos ;
  }
  void scale(uint32_t c){
    trav(it, s) *it = *it/c;
  }
};


typedef pair<uint64_t, uint64_t> Codeword; // first is codeword, second is codeword length.
Codeword make_codeword(uint64_t first = 0, uint64_t second = 0){
  return make_pair(first, second);
}

class Outfile{
  private:
    fstream &outfile;
    uint64_t pos, buffer;
  public:
    Outfile(fstream &outfile): outfile(outfile), pos(0), buffer(0){};
    void put_codeword(Codeword &codeword){
      if(codeword.second > 58) cerr << "codeword.put() warning" << endl;
      buffer = (buffer << codeword.second) | codeword.first;
      pos += codeword.second;
      while(pos > 7){
        outfile.put((buffer >> (pos-8)) & 0xff);
        //cerr << "put: " << bitset<8>((buffer >> (pos-8)) & 0xff) << endl;
        //buffer = buffer >> 8;
        pos = pos - 8;
      }
    }
};

class Frequency{
  private:
    FenwickTree tree;
    vector<uint32_t> freq;
    uint32_t size;
  public:
    Frequency(): tree(alphabet_size), freq(alphabet_size, 1), size(alphabet_size){
      rep(i, 0, alphabet_size){
        tree.update(i, 1);
      }
    }
    uint64_t get_total(){
      return tree.query(alphabet_size-1);
    }
    uint64_t get_symbol_frequency(uint64_t symbol){
      return freq[symbol];
      //if(symbol == 0) return tree.query(0);
      //else return tree.query(symbol) - tree.query(symbol-1); // Can be made faster by writing a specialised function for the fenwick tree. Or simply save them.
    }
    uint64_t get_F(int symbol){
      if(symbol == -1) return 0;
      return tree.query(symbol);
    }
    void update_symbol(uint64_t symbol){
      tree.update(symbol, 1);
      if(++size > 1000000000){
        tree.scale(5);
        rep(i, 0, alphabet_size){
          tree.update(i, 1);
        }
        size = tree.query(alphabet_size-1);
      }
    }
    int get_symbol(uint64_t F){
      return tree.find(F);
    }
};


void compress(fstream &infile, fstream &outfile){
  infile.seekg(0, fstream::end);
  uint32_t size = infile.tellg();
  infile.seekg(0, fstream::beg);
  outfile.put(size>>24);
  outfile.put(size>>16);
  outfile.put(size>>8);
  outfile.put(size);
  Outfile out(outfile);
  //cerr << size << endl;

  Frequency F;
  uint32_t l = 0, u = -1;
  uint32_t c = 0;
  //cerr << hex << u << dec << endl;
  int extra_shifts = 0;
  while(1){
    uint64_t l_old = l, u_old = u, t;
    c = infile.get();
    if(!infile) break;
    Codeword codeword = make_codeword();
    l = l_old + ((u_old - l_old + 1)*F.get_F(c-1))/F.get_total();
    u = l_old + ((u_old - l_old + 1)*F.get_F(c))/F.get_total() - 1;
    if(u < l){
      cerr << "Error: comp u < l symbol: " << (char) c << " n: " << F.get_F(c) - F.get_F(c-1) << " total: " << F.get_total() << endl;
      cerr << "l   : " << bitset<32>(l) << endl << "u   : " << bitset<32>(u) << endl;
      cerr << "lold: " << bitset<32>(l_old) << endl << "uold: " << bitset<32>(u_old) << endl;
    }
    //cerr << "F(c-1): " << F.get_F(c-1) << " F(c): " << F.get_F(c) << endl;
    //cerr << c << " : " << (char) c << endl;
    //cerr << "l: " << hex << l << " u: " << hex << u << endl;
    t = l ^ u;
    while(!(t >> 31)){
      codeword.first = (codeword.first << 1) + (u >> 31);
      ++codeword.second;
      uint32_t outshifted = (~u) >> 31;
      l <<= 1;
      u <<= 1;
      ++u;
      t <<= 1;
      ++t;
      if(extra_shifts){
        do{
          codeword.first = (codeword.first << 1) + outshifted;
          ++codeword.second;
        }while(--extra_shifts);
      }
    }
    // Case 3.
    while(((l >> 30) & 3) == 1 && ((u >> 30) & 3) == 2){
      ++extra_shifts;
      l <<= 1;
      u <<= 1;
      ++u;
      l ^= 0x80000000;
      u ^= 0x80000000;
    }
    F.update_symbol(c);
    if(codeword.second) out.put_codeword(codeword);
  }

  // Awkward finishing code.
  Codeword end = make_codeword(l>>31, 1);
  uint32_t outshifted = (~l) >> 31;
  if(extra_shifts){
    do{
      end.first = (end.first << 1) + outshifted;
      ++end.second;
    }while(--extra_shifts);
  }
  end.first = (end.first << 31) | (l&0x7fffffff);
  end.second += 31;
  out.put_codeword(end);
}

void decompress(fstream &infile, fstream &outfile){
  uint32_t size = 0;
  size += infile.get() << 24;
  size += infile.get() << 16;
  size += infile.get() << 8;
  size += infile.get();
  //cerr << size << endl;

  Frequency F;

  uint64_t length = 0;
  // Init.
  uint32_t t = 0;
  uint32_t l = 0, u = (uint32_t)-1;
  rep(i, 0, 4){
    int c = infile.get();
    t <<= 8;
    t += c;
  }

  uint32_t i = 0, c = infile.get();
  while(1){
    if (++length > size) return;
    //if(length%1000 == 0) cerr << length << " " << size << endl;
    uint64_t f = (((uint64_t)t-l+1)*F.get_total()-1)/((uint64_t)u-l+1);

    int symbol = F.get_symbol(f);
    outfile.put(symbol);
    //cerr << bitset<32>(t) << " : " << f << " : " << symbol << " : " << (char) symbol << endl;

    uint64_t l_old = l, u_old = u;

    l = l_old + ((u_old - l_old + 1)*F.get_F(symbol-1))/F.get_total();
    u = l_old + ((u_old - l_old + 1)*F.get_F(symbol))/F.get_total() - 1;
    if(u < l) cerr << "Error: decomp u < l symbol: " << (char)symbol << endl;
    F.update_symbol(symbol);
    //cerr << "F(c-1): " << F.get_F(symbol-1) << " F(c): " << F.get_F(symbol) << endl;
    //cerr << bitset<32>(l) << endl << bitset<32>(u) << endl;

    uint32_t tt = l^u;
    while(!(tt >> 31)){
      tt <<= 1;
      ++tt;
      u <<= 1;
      ++u;
      l <<= 1;
      t <<= 1;
      if((c >> (7-i)) & 1) t += 1;
      ++i;
      if(i > 7){
        i = 0;
        c = infile.get();
      }
    }
    // Case 3.
    while((l >> 30) == 1 && (u >> 30) == 2){
      //cerr << "l   : " << bitset<32>(l >> 30) << endl << "u   : " << bitset<32>(u >> 30) << endl;
      l <<= 1;
      u <<= 1;
      ++u;
      l ^= 0x80000000;
      u ^= 0x80000000;
      t <<= 1;
      if((c >> (7-i)) & 1) ++t;
      t ^= 0x80000000;
      ++i;
      if(i > 7){
        i = 0;
        c = infile.get();
      }
    }
  }
}

int main(int argc, char *argv[]){

  /*FenwickTree t(100);
  rep(i, 0, 100){
    t.update(i, 2);
  }
  rep(i, 0, 100){
    cerr << i << " " << t.find(i) << endl;
  }
  return 0;*/


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
  if(will_decompress)
    decompress(infile, outfile);
  else
    compress(infile, outfile);

  infile.close();
  outfile.close();
  return 0;
}
