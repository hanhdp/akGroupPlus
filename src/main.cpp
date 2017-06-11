#include <string.h>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <tuple>
#include <cassert>
#include <iostream>
#include <algorithm>

#include "biggraph.hpp"
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
using namespace std;

vector<tuple<char, int, int> > batch;

inline bool readeof() {
  for (;;) {
    int c = getc_unlocked(stdin);
    if (c == EOF) {
      return true;
    } else if (isspace(c)) {
      continue;
    } else {
      ungetc(c, stdin);
      return false;
    }
  }
  assert(false);
}

inline int readuint() {
  int c;
  int x;
  while (!isdigit(c = getc_unlocked(stdin)));
  x = c - '0';
  while (isdigit(c = getc_unlocked(stdin))) {
    x = (x * 10 + (c - '0'));
  }
  return x;
}

int main(int argc, char *argv[])
{

  __cilkrts_set_param("nworkers","32");

  BigGraph* pGD = new BigGraph();

  pGD->Build();
  
  // cerr << "Num of nodes " << pGD->Node_Num << endl;
  //   //print graph
  //   for (uint32_t v = 0; v < pGD->Node_Num; v++){
  //     for (uint32_t w : pGD->Edges[0][v]){
  //         cerr << v << " "<< pGD->GetID(w) << endl;
  //     }
  //   }
  //cerr << sizeof(pGD->Ds[0][0]) << endl;
  batch.reserve(100000);
  
  //warm memory
  for (int i = 0; i < 0; ++i) {
      batch.emplace_back('Q', rand() % pGD->Node_Num, rand() % pGD->Node_Num);
  }
  pGD->ProcessBatch(batch);
  batch.clear();
  
  //fprintf(stderr, "Finished cache warming !\n"); 
  puts("R");
  cout.flush();
  // sleep(1);
  //FILE *fp = stdin;
  //char out_buff[100000];memset(out_buff, 0, 100000);
  //setvbuf(stdout, out_buff, _IOFBF, 100000);
  //char *line = NULL;
  //size_t linesize=0;
  //int res;
  vector<int> dists;
  char cmd; int u, v;
  char num[6];
      
  while (true){
    if(readeof()) break;    
    
    while ((cmd = getc_unlocked(stdin)) && cmd != 'F'){
      u = readuint();
      v = readuint();
      batch.push_back(make_tuple(cmd, u, v));
      //renamed.emplace_back(false, false);
    }
    
    //mapping();
    dists = pGD->ProcessBatch(batch);
    string out;
    for (auto d : dists) {        
        sprintf(num, "%d\n",  d);
        out += num;
    }
    fputs_unlocked(out.c_str(), stdout);
    fflush_unlocked(stdout);
    if (batch.empty()) break;
    batch.clear();
    //renamed.clear();
  }
  
  delete pGD;
  
  return 0;
}
