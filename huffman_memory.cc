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

struct Tree;
struct Node;

typedef pair<int, int> Codeword; // first is codeword, second is codeword length.
Codeword make_codeword(int first = 0, int second = 0){
  return make_pair(first, second);
}

struct Node{
  Tree *tree;
  Node *parent, *left, *right;
  int weight, symbol;
  Node(Tree* tree, Node* parent = 0, int number_of_symbols = alphabet_size);
  ~Node();
  void get_codeword(Codeword &codeword, Node* child);
};
struct Tree{
  int next_symbol;
  Node *symbol_to_node[alphabet_size];
  vector<int> weights;// Keeps tracks of weight to avoid searching the tree for weights that don't exist. Uses linear space.
  Node *root;
  Tree():next_symbol(0),weights(alphabet_size+1, 0){
    //cerr << "Tree()" << endl;
    root = new Node(this);
  }
  ~Tree(){
    delete root;
  }
};

Node::Node(Tree* tree, Node* parent, int number_of_symbols):tree(tree),parent(parent), left(0), right(0), weight(number_of_symbols), symbol(NOSYMBOL){
  //cerr << "Node()" << endl;
  ++tree->weights[weight];
  if(number_of_symbols == 1){
    symbol = tree->next_symbol++;
    tree->symbol_to_node[symbol] = this;
    return;
  }
  left = new Node(tree, this, number_of_symbols >> 1);
  right = new Node(tree, this, number_of_symbols - (number_of_symbols >> 1));
}

Node::~Node(){
  if(left) delete left;
  if(right) delete right;
}
void Node::get_codeword(Codeword &codeword, Node* child = 0){
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

void switch_nodes(Node *n1, Node *n2){
  //cerr << endl << n1 << " " << n2 << endl;
  Node *n1_parent = n1->parent, *n2_parent = n2->parent;
  if(n1_parent->right == n1) n1_parent->right = n2;
  else n1_parent->left = n2;
  if(n2_parent->right == n2) n2_parent->right = n1;
  else n2_parent->left = n1;
  n1->parent = n2_parent;
  n2->parent = n1_parent;
  //if(n1->tree->root == n1) n1->tree->root = n2;
  //else if(n1->tree->root == n2) n1->tree->root = n1;
}

void update_tree(Node *root, Node *update_node){
  //cerr << "update_tree" << endl;
  Node *best_node = 0;

  if(root->tree->weights[update_node->weight] < 2){
    --root->tree->weights[update_node->weight];
    ++update_node->weight;
    if((unsigned int)update_node->weight >= root->tree->weights.size()) root->tree->weights.resize(root->tree->weights.size()*2);
    ++root->tree->weights[update_node->weight];
    if(update_node->parent) update_tree(root, update_node->parent);
    return;
  }
  --root->tree->weights[update_node->weight];

  /*if(update_node == root){
    cerr << "Har kan man inte vara" << endl;
    ++update_node->weight;
    ++root->tree->weights[update_node->weight];
    return;
  }*/

  // Use a bfs to find a better place for the node.
  // This means the first node with the same weight as the node we're updating is
  // either the that node or a node higher up in the tree.
  queue<Node*> bfs_queue;
  bfs_queue.push(root);
  while(!bfs_queue.empty()){
    Node *cur_node = bfs_queue.front();
    bfs_queue.pop();
    //if(cur_node->weight == update_node->weight){
    //  // A node with the same weight is found. This is either the starting node or a better node.
    //  best_node = cur_node;
    //  break; // This is the best node since bfs is used.
    //}
    // Push childs to queue.
    if(cur_node->left){
      if(cur_node->left->weight == update_node->weight){
        best_node = cur_node->left;
        break; // This is the best node since bfs is used.
      }
      if(cur_node->left->weight >= update_node->weight)
        bfs_queue.push(cur_node->left);
    }
    if(cur_node->right){
      if(cur_node->right->weight == update_node->weight){
        best_node = cur_node->right;
        break; // This is the best node since bfs is used.
      }

      if(cur_node->right->weight >= update_node->weight)
        bfs_queue.push(cur_node->right);
    }
  }
  if(best_node != update_node) // Not necessary but probably avoids some operations.
    switch_nodes(best_node, update_node);
  ++update_node->weight;
  ++root->tree->weights[update_node->weight];
  if(update_node->parent) update_tree(root, update_node->parent);
}

vector<int> v(alphabet_size);
void print_tree(Node *tree, int depth = 0){
  if(tree->left) print_tree(tree->left, depth+1);
  rep(i, 0, depth) cerr << "    ";
  cerr << "s: " << (char)tree->symbol << " w:" << tree->weight << " d: " << depth << endl;
  if(tree->right) print_tree(tree->right, depth+1);
  if(tree->symbol != NOSYMBOL) v[tree->symbol] = tree->weight;
  if(depth == 0){
    trav(it, v) cout << (char)(it - v.begin()) << ": " << *it << endl;
  }
}


Codeword get_codeword(Node *root, int symbol){
  Node *leaf = root->tree->symbol_to_node[symbol];
  Codeword codeword = make_codeword();
  leaf->get_codeword(codeword);
  return codeword;
}

int memory = 2;
typedef map<uint64_t, Tree*> TreeMap;
Node *get_tree(TreeMap &trees, uint64_t pos){
  uint64_t mask;
  switch(memory){
    case 4:
      mask = 0xffffffff; // Memory 4
      break;
    case 3:
      mask = 0x00ffffff; // Memory 3
      break;
    case 2:
      mask = 0x0000ffff; // Memory 2
      break;
    case 1:
      mask = 0x000000ff; // Memory 1
      break;
    default:
      cerr << "Illegal memory value" << endl;
      mask = 0x0000ffff; // Memory 2
      break;
  }
  TreeMap::iterator it = trees.find(pos & mask);
  Tree *tree;
  if(it == trees.end()){
    tree = new Tree();
    trees[pos & mask] = tree;
    return tree->root;
  }
  else{
    return it->second->root;
  }
}

void compress(fstream &infile, fstream &outfile){
  TreeMap trees;
  //trav(it, trees){
  //  *it = new Tree();
  //}
  trees[0] = new Tree();
  Node *cur_tree = trees[0]->root;
  int buffer = 0, pos = 0;
  uint64_t c1 = 0, c2 = 0, c3 = 0, c4 = 0, c = 0;
  while(1){
    c4 = c3;
    c3 = c2;
    c2 = c1;
    c1 = c;
    c = infile.get();
    //cc = infile.get();
    //c += cc << 8;
    if(!infile) break;
    Codeword codeword = get_codeword(cur_tree, c);
    buffer = (buffer << codeword.second) | codeword.first;
    pos += codeword.second;
    while(pos > 7){
      outfile.put((buffer >> (pos-8)) & 0xff);
      pos = pos - 8;
    }
    update_tree(cur_tree, cur_tree->tree->symbol_to_node[c]);
    cur_tree = get_tree(trees, (c4 << 32) + (c3 << 24) + (c2 << 16) + (c1<<8) + c);
  }
  if(pos){
    // One more symbol to code, find a codeword long enough to to NOT fit in the last byte.
    Codeword codeword;
    rep(i, 0, alphabet_size){
      codeword = get_codeword(cur_tree, i);
      if(codeword.second > 8-pos) break;
    }
    outfile.put(((buffer<<(8-pos)) & 0xff) | (codeword.first>>(codeword.second-8+pos)));
  }
}

void decompress(fstream &infile, fstream &outfile){
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
        memory = atoi(argv[i+1]);
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
