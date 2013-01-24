#include <fstream>
#include <iostream>

using namespace std;

void compress(fstream &infile, fstream &outfile){
  while(1){
    char c = infile.get();
    if(c == EOF) break;
    outfile.put(c);
  }
}


int main(int argc, char *argv[]){
  if(argc != 3){
    cout << "Wrong input argument, use " << argv[0] << " infile outfile.\n";
    return 0;
  }
  fstream infile(argv[1], fstream::in);
  fstream outfile(argv[2], fstream::out);

  compress(infile, outfile);

  infile.close();
  outfile.close();
  return 0;
}
