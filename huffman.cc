#include <fstream>
#include <iostream>
#include <queue>
#include <bitset>

#define rep(i, a, b) for(int i = (a); i < int(b); ++i)
#define trav(it, v) for(typeof((v).begin()) it = (v).begin(); \
    it != (v).end(); ++it)

using namespace std;

int const alphabet_size = 256;
int const NOSYMBOL = (int)1e9;

struct node;

typedef node* Tree;
typedef pair<int, int> Codeword; // first is codeword, second is codeword length.
Codeword make_codeword(int first = 0, int second = 0){
  return make_pair(first, second);
}

struct node{
  node *parent, *left, *right;
  int weight, symbol;
  static int next_symbol;
  static node *symbol_to_node[alphabet_size];
  node(node* parent = 0, int number_of_symbols = alphabet_size):parent(parent), left(0), right(0), weight(number_of_symbols), symbol(NOSYMBOL){
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
      parent->get_codeword(codeword, this);
      return;
    }
    if(child == right) 
      codeword.first += 1 << codeword.second;
    ++codeword.second;
    if(parent == 0) return;
    else{
      parent->get_codeword(codeword, this);
      return;
    }
  }
};
int node::next_symbol = 0;
node *node::symbol_to_node[alphabet_size];

void switch_nodes(node *n1, node*n2){
  node *n1_parent = n1->parent, *n2_parent = n2->parent;
  if(n1_parent->right == n1) n1_parent->right = n2;
  else n1_parent->left = n2;
  if(n2_parent->right == n2) n2_parent->right = n1;
  else n2_parent->left = n1;
  n1->parent = n2_parent;
  n2->parent = n1_parent;
}

void update_tree(Tree tree, node *update_node){
  node *best_node = 0;
  queue<node*> bfs_queue;

  // Use a bfs to find a better place for the node.
  // This means the first node with the same weight as the node we're updating is
  // either the that node or a node higher up in the tree.
  bfs_queue.push(tree);
  //int i = 0, pos = 1;
  while(!bfs_queue.empty()){
    node *cur_node = bfs_queue.front();
    bfs_queue.pop();
    //if(i == pos) cerr << '\n', pos = pos << 1;
    //++i;
    //cerr << cur_node->symbol << ": " << cur_node->weight << " | ";
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
  if(best_node != update_node) // Not necessary but probably avoids some operations.
    switch_nodes(best_node, update_node);
  ++update_node->weight;
  if(update_node->parent) update_tree(tree, update_node->parent);
}
  
vector<int> v(256);
void print_tree(Tree tree, int depth = 0){
  if(tree->left) print_tree(tree->left, depth+1);
  rep(i, 0, depth) cerr << "    ";
  cerr << "s: " << (char)tree->symbol << " w:" << tree->weight << " d: " << depth << endl;
  if(tree->right) print_tree(tree->right, depth+1);
  if(tree->symbol != NOSYMBOL) v[tree->symbol] = tree->weight;
  if(depth == 0){
    trav(it, v) cout << (char)(it - v.begin()) << ": " << *it << endl;
  }
}


Codeword get_codeword(Tree tree, int symbol){
  node *leaf = tree->symbol_to_node[symbol];
  Codeword codeword = make_codeword();
  leaf->get_codeword(codeword);
  return codeword;
}

void compress(fstream &infile, fstream &outfile){
  Tree huffman_tree = new node();
  int buffer = 0, pos = 0;
  while(1){
    int c = infile.get();
    if(!infile) break;
    Codeword codeword = get_codeword(huffman_tree, c);
    buffer = (buffer << codeword.second) | codeword.first;
    //bitset<32> bb(codeword.second);
    //cerr << bb << " " << codeword.second << endl;
    pos += codeword.second;
    while(pos > 7){
      outfile.put((buffer >> (pos-8)) & 0xff);
      //cerr << ((buffer >> (pos-8)) & 0xff) << " " << pos << endl;
      //buffer = buffer  8;
      pos = pos - 8;
    }
    update_tree(huffman_tree, huffman_tree->symbol_to_node[c]);
  }
  if(pos){
    // One more symbol to code, find a codeword long enough to to not fit in the  last byte.
    Codeword codeword;
    rep(i, 0, 256){
      codeword = get_codeword(huffman_tree, i);
      if(codeword.second > 8-pos) break;
    }
    outfile.put(((buffer<<(8-pos)) & 0xff) | (codeword.first>>(codeword.second-8+pos)));
  }
  //print_tree(huffman_tree);
}

void decompress(fstream &infile, fstream &outfile){
  Tree huffman_tree = new node();
  node *cur = huffman_tree;;
  while(1){
    int c = infile.get();
    if(!infile) break;
    rep(i, 0, 8){
      if((c >> (7-i)) & 1) cur = cur->right;
      else cur = cur->left;
      if(cur->symbol != NOSYMBOL){
        outfile.put(cur->symbol);
        update_tree(huffman_tree, huffman_tree->symbol_to_node[cur->symbol]);
        cur = huffman_tree;
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
