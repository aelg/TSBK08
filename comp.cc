#include <fstream>
#include <iostream>

using namespace std;

void compress(fstream &infile, fstream &outfile){
  while(1){
    char c = infile.get();
    if(!infile) break;
    outfile.put(c);
  }
}

void decompress(fstream &infile, fstream &outfile){
  while(1){
    char c = infile.get();
    if(!infile) break;
    outfile.put(c);
  }
}

int main(int argc, char *argv[]){
  if(argc < 2){
    cout << "Wrong input argument, use " << argv[0] << " [-d] infile.\n";
    return 0;
  }
  fstream infile;
  fstream outfile;
  if(argv[1][0] == '-'){
    if(argv[1][1] == 'd'){
      infile.open(argv[2], fstream::in | fstream::binary);
      if(!infile){
        cerr << "Could not open file " << argv[2] << '\n';
      }
      outfile.open(argv[3], fstream::out | fstream::binary);
      if(!outfile){
        cerr << "Could not open file " << argv[3] << '\n';
      }
    }
    else{
      cerr << "Unrecoginized parameter " << argv[1] << '\n';
      return 0;
    }
  }
  else{
    infile.open(argv[1], fstream::in | fstream::binary);
    if(!infile){
      cerr << "Could not open file " << argv[1] << '\n';
    }
    outfile.open(argv[2], fstream::out | fstream::binary);
    if(!outfile){
      cerr << "Could not open file " << argv[2] << '\n';
    }
  }

  if(argv[1][0] == '-' && argv[1][1] == 'd')
    decompress(infile, outfile);
  else
    compress(infile, outfile);

  infile.close();
  outfile.close();
  return 0;
}
