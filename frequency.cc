#include "frequency.h"
#include <iostream>
#include <algorithm>

#define rep(i, a, b) for(unsigned int i = (a); i < (unsigned int)(b); ++i)
#define trav(it, v) for(typeof((v).begin()) it = (v).begin(); \
    it != (v).end(); ++it)

using namespace std;

FenwickTree::FenwickTree(uint32_t _n) : n(_n), bitmask(1) {
  while((bitmask<<1) < n) bitmask <<= 1;
  s.assign(n, 0);
}
void FenwickTree::update(uint32_t pos, int dif) {
  for (; pos < n; pos |= pos + 1)
    s[pos] += dif;
}
int FenwickTree::query(int val) {
  int count = 0;
  for (; val >= 0; val = (val & (val + 1)) - 1)
    count += s[val];
  return count;
}
int FenwickTree::find(uint32_t val){
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
void FenwickTree::scale(uint32_t c){
  trav(it, s) *it = *it/c;
}

// Model updates with fenwick tree.
ModelFenwick::ModelFenwick(): last_seen(0){
  rep(i, 0, alphabet_size){
    sizes[i] = alphabet_size;
    trees.push_back(FenwickTree(alphabet_size));
    rep(j, 0, alphabet_size){
      trees[i].update(j, 1);
    }
  }
}
uint64_t ModelFenwick::get_total(){
  return trees[last_seen].query(alphabet_size-1);
}
uint64_t ModelFenwick::get_l(){
  if((int)symbol - 1 == -1) return 0;
  return trees[last_seen].query(symbol-1);
}
uint64_t ModelFenwick::get_u(){
  if((int)symbol == -1) return 0;
  return trees[last_seen].query(symbol);
}
void ModelFenwick::update(){
  trees[last_seen].update(symbol, 1);
  if(++sizes[last_seen] > 1000000000){
    trees[last_seen].scale(10);
    rep(i, 0, alphabet_size){
      trees[last_seen].update(i, 1);
    }
    sizes[last_seen] = trees[last_seen].query(alphabet_size-1);
  }
  last_seen = symbol;
}
uint32_t ModelFenwick::get_symbol(uint64_t F){
  symbol = trees[last_seen].find(F);
  return symbol;
}
void ModelFenwick::set_symbol(uint32_t s){
  symbol = s;
}


// PPM model.
ModelPPM::ModelPPM(uint32_t ppm_type, bool decoder) : root(0), prev_context(0), l(0), u(1), total(1), ppm_type(ppm_type), need_new_symbol(true), need_new_symbol_intern(false), decoder(decoder){
  cur_context = &root;
  cur_symbol = root.first;
}
uint64_t ModelPPM::get_total(){
  return total;
}
uint64_t ModelPPM::get_l(){
  return l;
}
uint64_t ModelPPM::get_u(){
  return u;
}

void ModelPPM::find_in_no_context(){
  l = 0;
  u = 1;
  rep(i, 0, 256){
    if(s == i) break;
    if(!seen[i]){
      ++l;
      ++u;
    }
  }
  total = 256 - seen.count();
}


void ModelPPM::find_in_context(){
  if(!cur_context){
    found_symbol = true;
    find_in_no_context();
    return;
  }
  Symbol *it = cur_context->first;
  cur_symbol = it;
  uint32_t sum = 0;
  l = 0;
  u = 1;
  uint32_t no_symbols = 0;
  while(it){
    if(it->symbol > 256){
      cerr << "Error saw symbol: " << it->symbol << endl;
      return;
    }
    ++no_symbols;
    if(seen[it->symbol]){
      it = it->next;
      continue;
    }
    if(it->symbol == s){
      l = sum;
      u = sum + it->count;
      cur_symbol = it;
    }
    else if(!decoder && it->symbol != NOT_SEEN) seen.set(it->symbol);
    sum += it->count;
    it = it->next;
  }
  if(cur_symbol->symbol == NOT_SEEN){ // Not found, use the first which is NOT_SEEN.
    u = cur_context->first->count;

    switch(ppm_type){
      case PPMA:
        break;
      case PPMD:
        ++cur_context->first->count; // ppmd
        ++cur_context->total;
        break;
      case PPMC:
        cur_context->total += no_symbols - cur_context->first->count; // ppmc
        cur_context->first->count = no_symbols;
        break;
    }
  }
  else{
    found_symbol = true;
    ++cur_symbol->count;
    ++cur_context->total;
  }
  total = sum;
}

void ModelPPM::find_context(){
  Symbol *it = cur_context->first;
  while(it){
    if(it->symbol == s){
      cur_context = it->context;
      break;
    }
    it = it->next;
  }
  if(!it) cerr << "Error find_context() didn't find context." << endl;

  total = cur_context->total;
}

void ModelPPM::update(){
  if(found_symbol){
    if(cur_context && cur_symbol->context) cur_context = cur_symbol->context;
    else if(cur_context){
      cur_context = cur_context->parent;
      find_context();
      cur_symbol->context = cur_context; // Slight optimization. As we don't need to find the parent context next time.
      cur_symbol->do_not_delete_context = true;
    }
    while(!context_stack.empty()){
      Context *context = context_stack.top();
      context_stack.pop();

      if(context->depth < MAX_DEPTH){
        context->last->next = new Symbol(s, new Context(cur_context?cur_context:&root));
        context->last = context->last->next;
        ++context->total;
        cur_context = context->last->context;
      }
      else{
        context->last->next = new Symbol(s, 0);
        context->last = context->last->next;
        cur_context = context->parent;
        ++context->total;
        find_context();
      }
    }
    need_new_symbol = true;
    seen.reset();
    if(!cur_context) total = 256;
    else{
      total = cur_context->total;
    }
  }
  else{
    context_stack.push(cur_context);
    cur_context = cur_context->parent;
    if(decoder){
      if(!cur_context) find_in_no_context();
      else{
        Symbol *it = cur_context->first;
        total = 0;
        while(it){
          if(!seen[it->symbol]) total += it->count;
          it = it->next;
        }
      }
    }
    else find_in_context();
  }
}
uint32_t ModelPPM::get_symbol(uint64_t F){
  found_symbol = false;
  if(!cur_context){
    found_symbol = true;
    l = 0;
    u = 1;
    rep(i, 0, 256){
      if(!seen[i]){
        if(l == F){
          s = i;
          break;
        }
        ++l;
        ++u;
      }
    }
    total = 256 - seen.count();
    return s;
  }
  Symbol *it = cur_context->first;
  uint32_t sum = 0;
  s = it->symbol;
  l = 0;
  u = 1;
  cur_symbol = it;
  uint32_t no_symbols = 0;
  while(it){
    if(it->symbol > 256){
      cerr << "Error saw symbol: " << it->symbol << endl;
      return 0;
    }
    ++no_symbols;
    if(seen[it->symbol]){
      it = it->next;
      continue;
    }
    if(sum <= F){
      s = it->symbol;
      l = sum;
      u = sum + it->count;
      cur_symbol = it;
    }
    if(it->symbol != NOT_SEEN) seen.set(it->symbol);
    sum += it->count;
    it = it->next;
  }
  seen.reset(cur_symbol->symbol);
  if(cur_symbol->symbol == NOT_SEEN){ // Not found, use the first which is NOT_SEEN.
    u = cur_context->first->count;
    s = NOT_SEEN;

    switch(ppm_type){
      case PPMA:
        break;
      case PPMD:
        ++cur_context->first->count; // ppmd
        ++cur_context->total;
        break;
      case PPMC:
        cur_context->total += no_symbols - cur_context->first->count; // ppmc
        cur_context->first->count = no_symbols;
        break;
    }
  }
  else {
    found_symbol = true;
    ++cur_symbol->count;
    ++cur_context->total;
  }
  total = sum;
  return s;
}
void ModelPPM::set_symbol(uint32_t s){
  found_symbol = false;
  need_new_symbol = false;
  this->s = s;
  seen.reset();
  this->find_in_context();
}
bool ModelPPM::need_symbol(){
  return need_new_symbol;
}
