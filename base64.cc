#include <fstream>
#include <iostream>

#define rep(i, a, b) for(int i = (a); i < int(b); ++i)
#define trav(it, v) for(typeof((v).begin()) it = (v).begin(); \
    it != (v).end(); ++it)

using namespace std;

char const base64[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void compress(fstream &infile, ostream &outfile){
  int j = 0;
  while(1){
    int n = 0, filled_bytes_at_eof = 3, c;
    rep(i, 0, 3){
      c = infile.get();
      if(c == EOF){
        filled_bytes_at_eof = i;
        break;
      }
      else{
        n = n << 8;
        n += c;
      }
    }
    switch(filled_bytes_at_eof){
      case 3:
        outfile.put(base64[(n>>18) & 0x3f]);
        outfile.put(base64[(n>>12) & 0x3f]);
        outfile.put(base64[(n>>6) & 0x3f]);
        outfile.put(base64[n & 0x3f]);
        break;
      case 2:
        outfile.put(base64[(n>>12) & 0x3f]);
        outfile.put(base64[(n>>6) & 0x3f]);
        outfile.put(base64[n & 0x3f]);
        outfile.put('=');
        break;
      case 1:
        outfile.put(base64[(n>>6) & 0x3f]);
        outfile.put(base64[n & 0x3f]);
        outfile.put('=');
        outfile.put('=');
        break;
    }
    if(++j%19 == 0) outfile.put('\n');
    if(c == EOF) break;
  }
  outfile.put('\n');
}

void decompress(fstream &infile, ostream &outfile){
  int decode[256];
  rep(i, 0, 64){
    decode[(int)base64[i]] = i;
  }
  while(1){
    int n = 0, c, stopped_at = 4;
    rep(i, 0, 4){
      c = infile.get();
      while(!((c >= 'A' && c <= 'Z') || 
              (c >= 'a' && c <= 'z') || 
              (c >= '0' && c <= '9') || 
               c == '+' || 
               c == '/' || 
               c == '=' ||
               c == EOF)){
        c = infile.get();
      }
      if(c == '=' || c == EOF){
        stopped_at = i;
        break;
      }
      n = n << 6;
      n += decode[c];
    }
    switch(stopped_at){
      case 4:
        outfile.put((n>>16) & 0xff);
        outfile.put((n>>8) & 0xff);
        outfile.put(n & 0xff);
        break;
      case 3:
        outfile.put((n>>8) & 0xff);
        outfile.put(n & 0xff);
        break;
      case 2:
        outfile.put(n & 0xff);
        break;
    }
    if(c == EOF) break;
  }
}

int main(int argc, char *argv[]){
  if(argc < 2){
    cout << "Wrong input argument, use " << argv[0] << " [-d] infile.\n";
    return 0;
  }
  fstream infile;
  if(argv[1][0] == '-' && argv[1][1] == 'd')
    infile.open(argv[2], fstream::in | fstream::binary);
  else
    infile.open(argv[1], fstream::in | fstream::binary);
  //fstream outfile(argv[2], fstream::out | fstream::binary);

  if(argv[1][0] == '-' && argv[1][1] == 'd')
    decompress(infile, cout);
  else
    compress(infile, cout);

  infile.close();
  //outfile.close();
  return 0;
}
