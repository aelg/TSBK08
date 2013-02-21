#ifndef FREQUENCY_H
#define FREQUENCY_H
#include <vector>
#include <inttypes.h>
#include <bitset>
#include <iostream>
#include <stack>

using namespace std;

uint32_t const alphabet_size = 256;
uint32_t const NOT_SEEN = 256;
uint32_t const NO_SYMBOL = 257;

class Model{
  public:
    Model(){};
    virtual uint64_t get_total() = 0;
    virtual uint64_t get_l() = 0;
    virtual uint64_t get_u() = 0;
    virtual void update() = 0;
    virtual uint32_t get_symbol(uint64_t F) = 0;
    virtual void set_symbol(uint32_t symbol) = 0;
    virtual bool need_symbol(){return true;}
};

struct FenwickTree{
  uint32_t n, bitmask;
  std::vector<uint32_t> s;
  FenwickTree(uint32_t _n);
  void update(uint32_t pos, int dif);
  int query(int val);
  int find(uint32_t val);
  void scale(uint32_t c);
};

class ModelFenwick : public Model {
  private:
    std::vector<FenwickTree> trees;
    uint32_t sizes[alphabet_size], last_seen;
    uint32_t symbol;
  public:
    ModelFenwick();
    uint64_t get_total();
    uint64_t get_l();
    uint64_t get_u();
    void update();
    uint32_t get_symbol(uint64_t F);
    void set_symbol(uint32_t symbol);
};


class ModelPPM : public Model {
  private:
    void find_in_context();
    void addContext();
    void find_in_no_context();
    void find_context_if_max_depth();
    struct Context;
    struct Symbol{
      uint32_t symbol, count;
      Context *context;
      Symbol *next;
      Symbol(uint32_t symbol, Context *context):symbol(symbol), count(1), context(context), next(0){
        //cerr << "Added symbol: " << symbol << endl;
      }
    } *cur_symbol;
    struct Context{
      Symbol *first, *last;
      Context *parent;
      uint32_t depth;
      static uint32_t size;
      Context(Context *parent):parent(parent){
        first = new Symbol(NOT_SEEN, parent);
        last = first;
        if(parent) depth = parent->depth+1;
        else depth = 0;
        //cerr << depth << endl;
      }
    }root, *cur_context, *prev_context;
    std::bitset<257> seen;
    uint32_t l, u, total, s;
    bool need_new_symbol, found_symbol, need_new_symbol_intern, decoder;
    //ModelFenwick no_context;
    stack<Context*> context_stack;
    static uint32_t const MAX_DEPTH = 4;
  public:
    ModelPPM(bool decoder = false);
    uint64_t get_total();
    uint64_t get_l();
    uint64_t get_u();
    //uint64_t get_F(int symbol);
    void update();
    uint32_t get_symbol(uint64_t F);
    bool need_symbol();
    void set_symbol(uint32_t symbol);
};
#endif
