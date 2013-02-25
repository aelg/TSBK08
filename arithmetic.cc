#include <fstream>
#include <iostream>
#include <queue>
#include <bitset>
#include <map>
#include <inttypes.h>
#include <cstdlib>
#include "frequency.h"

#define rep(i, a, b) for(int i = (a); i < int(b); ++i)
#define trav(it, v) for(typeof((v).begin()) it = (v).begin(); \
    it != (v).end(); ++it)

using namespace std;

typedef pair<uint64_t, uint64_t> Codeword; // first is codeword, second is codeword length.
Codeword make_codeword(uint64_t first = 0, uint64_t second = 0){
  return make_pair(first, second);
}

class Outfile{
  private:
    fstream &outfile;
    uint64_t pos, buffer;
    uint32_t length;
  public:
    Outfile(fstream &outfile): outfile(outfile), pos(0), buffer(0), length(0){};
    void put_codeword(Codeword &codeword){
      if(codeword.second > 58) cerr << "Outfile.put_codeword() warning, large codeword" << endl;
      buffer = (buffer << codeword.second) | codeword.first;
      pos += codeword.second;
      while(pos > 7){
        outfile.put((buffer >> (pos-8)) & 0xff);
        //if(++length % 1000 == 0) cerr << endl << "Encoded: " << length << endl;
        //cerr << "put: " << bitset<8>((buffer >> (pos-8)) & 0xff) << endl;
        //buffer = buffer >> 8;
        pos = pos - 8;
      }
    }
};

void compress(fstream &infile, fstream &outfile){
  // Read filesize.
  infile.seekg(0, fstream::end);
  uint32_t size = infile.tellg();
  infile.seekg(0, fstream::beg);
  // Write filesize to first in file. (Max filesize ~4GB)
  outfile.put(size>>24);
  outfile.put(size>>16);
  outfile.put(size>>8);
  outfile.put(size);
  Outfile out(outfile);

  //ModelFenwick F;
  ModelPPM F(ModelPPM::PPMC);

  uint32_t l = 0, u = -1;
  uint32_t c = 0;
  int extra_shifts = 0;
  //uint32_t length = 0;
  while(1){
    uint64_t l_old = l, u_old = u, t;
    if(F.need_symbol()){
      //if(++length%1000 == 0) cerr << endl << "Read: " << length << endl;
      c = infile.get();
      if(!infile) break;
      F.set_symbol(c);
    }

    // Update u and l with precision magic.
    l = l_old + ((u_old - l_old + 1)*F.get_l())/F.get_total();
    u = l_old + ((u_old - l_old + 1)*F.get_u())/F.get_total() - 1;

    // Update symbol table.
    F.update();

    // Shift away to get codes.
    Codeword codeword = make_codeword();
    t = l ^ u;
    while(!(t >> 31)){
      codeword.first = (codeword.first << 1) + (u >> 31);
      ++codeword.second;
      uint32_t outshifted = (~u) >> 31;
      l <<= 1;
      u <<= 1;
      t <<= 1;
      ++u;
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
  // Read length.
  uint32_t size = 0;
  size += infile.get() << 24;
  size += infile.get() << 16;
  size += infile.get() << 8;
  size += infile.get();

  //ModelFenwick F;
  ModelPPM F(ModelPPM::PPMC, true);

  uint32_t length = 0;
  // Init.
  uint32_t t = 0, l = 0, u = -1;
  rep(i, 0, 4){
    int c = infile.get();
    t <<= 8;
    t += c;
  }

  uint32_t i = 0, c = infile.get();
  while(1){

    // Calculate F for next symbol
    uint32_t f = ((t-l+1)*F.get_total()-1)/((uint64_t)u-l+1);

    // Find symbol.
    int symbol = F.get_symbol(f);
    //cerr << symbol;
    if(symbol < 256){
      outfile.put(symbol);
      if (++length >= size) return;
    }

    uint64_t l_old = l, u_old = u;

    // Update u and l with fixed precision magic.
    l = l_old + ((u_old - l_old + 1)*F.get_l())/F.get_total();
    u = l_old + ((u_old - l_old + 1)*F.get_u())/F.get_total() - 1;

    // Update symbol table.
    F.update();

    uint32_t tt = l^u;
    while(!(tt >> 31)){
      tt <<= 1;
      u <<= 1;
      l <<= 1;
      t <<= 1;
      // Shift in 1 in tt and u.
      ++tt;
      ++u;
      if((c >> (7-i)) & 1) t += 1;
      // Update read buffer
      ++i;
      if(i > 7){
        i = 0;
        c = infile.get();
      }
    }
    // Case 3.
    while((l >> 30) == 1 && (u >> 30) == 2){
      t <<= 1;
      l <<= 1;
      u <<= 1;
      ++u; // Shift in 1 in u
      if((c >> (7-i)) & 1) ++t; // Read from buffer.
      t ^= 0x80000000;
      l ^= 0x80000000;
      u ^= 0x80000000;
      // Update read buffer.
      ++i;
      if(i > 7){
        i = 0;
        c = infile.get();
      }
    }
  }
}

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
  if(will_decompress)
    decompress(infile, outfile);
  else
    compress(infile, outfile);

  infile.close();
  outfile.close();
  return 0;
}
