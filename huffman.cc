#include <fstream>
#include <iostream>

#define rep(i, a, b) for(int i = (a); i < int(b); ++i)
#define trav(it, v) for(typeof((v).begin()) it = (v).begin(); \
    it != (v).end(); ++it)

using namespace std;

int const alphabet_size = 256;
int const inf = (int)1e9

struct node;

typedef Tree node*;
typedef Codeword pair<int, int>; // first is codeword, second is codeword length.
Codeword make_codeword(){
  return make_pair(0, 0);
}

struct node{
  node* parent, left, right;
  int weight, symbol;
  static node*[alphabet_size] next_symbol;
  static vector<node*> symbol_to_node;
  node(node* parent, int number_of_symbols = alphabet_size):parent(parent), left(0), right(0), weight(number_of_symbols){
    if(number_of_symbols == 1){
      symbol = next_symbol++;
      symbol_to_node[symbol] = this;
      return;
    }
    left = new node(this, number_of_symbols >> 1);
    right = new node(this, number_of_symbols - (number_of_symbols >> 1));
  }
  ~node(){
      delete left;
      delete right;
  }
  void get_codeword(Codeword &codeword, node* child = 0){
    if(child == 0){
      parent->get_codeword(this, codeword);
      return;
    }
    if(child == right) 
      codeword.first += 1 << codeword.second;
    ++codeword.second;
    if(parent == 0) return;
    else{
      parent->get_codeword(this, codeword);
      return;
    }
  }
};
int node::next_symbol = 0;

void switch_nodes(node *n1, node*n2){
  node *n1_parent = n1->parent, *n2_parent = n2->parent;
  if(n1_parent->right == n1) n1_parent->right = n2;
  else n1_parent->left = n2;
  if(n2_parent->right == n2) n2_parent->right = n1;
  else n1_parent->left = n1;
  n1->parent = n2_parent;
  n2->parent = n1_parent;
}

void update_tree(Tree tree, int new_symbol){
  node *update_node = tree->symbol_to_node[new_symbol];
  node *best_node = 0;
  queue<node*> bfs_queue;
  tree->depth = 0;

  // Use a bfs to find a better place for the node.
  // This means the first node with the same weight as the node we're updating is
  // either the that node or a node higher up in the tree.
  bfs_queue.push(tree);
  while(!bfs_queue.empty()){
    node *cur_node = bfs_queue.front();
    bfs_queue.pop();
    if(cur_node->weight == update_node->weight){
      // A node with the same weight is found. This is either the starting node or a better node.
      best_node = cur_node;
      break; // This is the best node since bfs is used.
    }
    // Push childs to queue.
    if(cur_node->left){
      bfs_queue.push(cur_node->left);
    }
    if(cur_node->right){
      bfs_queue.push(cur_node->right);
    }
  }
  if(best_node == update_node) return; // Not necessary but probably avoids some operations.
  switch_nodes(best_node, update_node);
}
  


Codeword get_codeword(Tree tree, int symbol){
  node *leaf = tree->symbol_to_node[symbol];
  Codeword codeword;
  leaf->get_codeword(codeword);
  return codeword;
}

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
