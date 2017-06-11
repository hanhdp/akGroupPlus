#include "biggraph.hpp"
#include <cassert>
#include <algorithm>
#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
using namespace std;

namespace {
  //check for timestamp
  bool inline Check(int v, int w, int time, uint32_t state,
    const vector<tuple<int, int, int, char>> &updates){

    auto iter = lower_bound(updates.begin(), updates.end(), make_tuple(v, w, time, 0));
    if (updates.empty() || iter == updates.begin()){
      return (state & 1) == 0;
    }
    
    auto last = *(--iter);
    if (v == get<0>(last) && w == get<1>(last)){
      return get<3>(last) == 'A';
    } else {
      return (state & 1) == 0;
    }
  }
}


int BigGraph::QueryDistance(int s, int t, int time,
                const vector<tuple<int, int, int, char> > &updates)
{

  if (s == t) return 0;
        
  int dist_ub =  1e2;

  int thread_num = __cilkrts_get_worker_number();    
  auto &Q = Qs[thread_num];
  auto &D = Ds[thread_num];
  int res = -1, dis[2] = {0, 0};
  int weight[2] = {0, 0}, v;
  
  for (int dir = 0; dir < 2; dir++){
    v = dir == 0 ? s : t;
    Q[dir].clear();
    Q[dir].push(v);
    Q[dir].next();

    //D[v / D_unit] |= 1 << (dir + v % D_unit * 2);
    D[v >> 4] |= 1 << (dir + ((v & 15) << 1 ) );
    weight[dir] += Edges[dir][v].size();
  }
  
  auto bfs_step = [&](int use) -> bool{
    dis[use]++;
    const auto & G_ = Edges[use];
    const auto & S_ = S[use];
    auto &       Q_ = Q[use];
    weight[use] = 0;
    
    while (!Q_.empty()) {
      int v = Q_.front();
      //cerr << endl << "Current check vertex " << v << "(" << Edges[use][v].size() << "): ";
      weight[use] += S_[v];
      Q_.pop();
      
      for (uint32_t w_ : G_[v]) {
        //counter[use]++;
        uint32_t w = GetID(w_);
        //cerr << w << " - ";
        auto & d = D[w >> 4];
        //auto & d = D[w / D_unit];
        const int bit_pos = ((w & 15) << 1 ) + use;
        //const int bit_pos = w % D_unit * 2 + use;
        if (d & (1 << bit_pos)) continue;
        uint32_t state = GetEdgeState(w_);
        if (state == ALIVE || ((state & UNKNOWN) &&
         (use == 0 ? Check(v, w, time, state, updates) :
              Check(w, v, time, state, updates) ) ) ){
          if (d & (1 << (1 - 2 * use + bit_pos))) {
            res = dis[0] + dis[1];
            //cerr << "One check" << endl;
            //connected_counter++;
            return true;
          } else {
            Q_.push(w);
            //weight[use] += S_[w];
            d |= 1 << bit_pos;
          }
        }
      }
    }
    Q_.next();    
    return false;
  };
  

    for (int use = 0; use < 2; ++use) {
      if(bfs_step(use)) goto LOOP_END;
    }

    while (!Q[0].empty() && !Q[1].empty()) {
      const int use = (weight[0] <= weight[1]) ? 0 : 1;
      if (dis[0] + dis[1] + 1 == dist_ub){
        res = dis[0] + dis[1] + 1;
        goto LOOP_END;
      }
      if(bfs_step(use)) goto LOOP_END;
    }

    LOOP_END:
    for (int dir = 0; dir < 2; dir++) {
      for (int v : Q[dir]) {
        D[v >> 4] = 0;
        //D[v / D_unit] = false;
      }
      Q[dir].clear();
    }

    return res;
  }

  void BigGraph::InsertNode(vector<uint32_t> &vs, uint32_t v, bool unknown){
    auto iter = lower_bound(vs.begin(), vs.end(), v);
    if (iter != vs.end() && GetID(*iter) == GetID(v)){//v exists on list vs
      if (unknown){
        *iter |= UNKNOWN;
      } else {
        *iter = ToEdge(GetID(v)) | ALIVE;
      }
    } else {// not exist
      if (unknown){
        vs.insert(iter, ToEdge(GetID(v))  | MASK);//ToEdge(GetID(v))
      } else {
        vs.insert(iter, ToEdge(GetID(v)) | ALIVE);
      }
    }
  }

  void BigGraph::DeleteNode(vector<uint32_t> &vs, uint32_t v, bool unknown){
    auto iter = lower_bound(vs.begin(), vs.end(), v);
    if (iter != vs.end() && GetID(*iter) == GetID(v)){//if found
      if (unknown){
        *iter |= UNKNOWN;
      } else {
        *iter = ToEdge(GetID(v)) | DEAD;
      }
    }
  }

  void BigGraph::Build()
  {
    //1.fetch edge set from stdin  
    FILE *fp = stdin; size_t bufsize=0;char *line = NULL;
    //vector<pair<int, int> > es;
    int res; uint32_t u, v;
    Edges[0].clear(); Edges[1].clear();
    Edges[0].resize(1e7); Edges[1].resize(1e7);   
    // vertex identified from 0 -> n
    while (true){
      res = getline(&line,&bufsize,fp);
      if (res == -1) break;
      if (line[0] == 'S') break;
    
      res = sscanf(line, "%u %u", &u, &v);
      if ( !res || res == EOF ) {
        continue;
      } 
      Node_Num = max({Node_Num, u + 1, v + 1});  
      if (Node_Num>1e7-1){
        Edges[0].resize(1e8); Edges[1].resize(1e8);  
      }
      Edges[0][u].push_back(ToEdge(v));      
      Edges[1][v].push_back(ToEdge(u));
    }
    cerr << "End of Read" << endl;

	  /*
	for (auto &e : es){
      Edges[0][e.fst].push_back(ToEdge(e.snd));
      Edges[1][e.snd].push_back(ToEdge(e.fst));
    }
*/
    //sort adjacent lists
    for (uint32_t v = 0; v < Node_Num; v++){
      sort(Edges[0][v].begin(), Edges[0][v].end());    
      sort(Edges[1][v].begin(), Edges[1][v].end());
      //for (int i=0; i<Edges[0][v].size(); i++)
      //  cerr << v << " " << GetID(Edges[0][v][i]) << endl;
    }

    Node_Num += 1e5;//add more nodes 
    //cerr << "End of init" << endl;
    Edges[0].resize(Node_Num);
    Edges[1].resize(Node_Num);
  

    //2. Init the graph
    int num_threads = __cilkrts_get_nworkers();
    cerr << num_threads << endl;
  
    Qs.clear();
    Ds.clear();
    
    for (int t = 0; t < num_threads; t++){
      Qs.emplace_back(2, TwoLevelQueue<int>(Node_Num));
      Ds.emplace_back(Node_Num / (sizeof(Ds[t][0]) * 4));
      //cerr << (Ds[t][0]) << " " << Node_Num / (sizeof(Ds[t][0]) * 4) << endl;
    }
        
    for (int dir = 0; dir < 2; dir++){
      S[dir].resize(Node_Num);
      for (uint32_t v = 0; v < Node_Num; v++){
        for (uint32_t w : Edges[dir][v]){
          S[dir][v] += Edges[dir][GetID(w)].size();
        }
      }
    }
    // cerr << "Num of nodes " << Node_Num << endl;
    cerr << "End of Build" << endl;
  }


  vector<int> BigGraph::ProcessBatch(const vector<query_t> &batch)
  {
    //#pragma omp parallel for
    cilk_for (int dir = 0; dir < 2; ++dir){
      for (size_t i = 0; i < batch.size(); i++){
        query_t query = batch[i];
        char cmd; int u, v;
        tie(cmd, u, v) = query;

        if(u == v && cmd != 'Q') continue;

        if (dir && int(Edges[0].size()) < max(u, v) + 1){
          int V = max(u, v) + 1;
          for (int dir = 0; dir < 2; dir++){
            Edges[dir].resize(V);
          }

          for (int t = 0; t < __cilkrts_get_nworkers(); t++){
            Ds[t].resize(V / (sizeof(Ds[t][0]) * 4) + 1);
            for (int dir = 0; dir < 2; dir++){
              Qs[t][dir].resize(V);
            }
          }
        }

        switch(cmd){
          case 'A':
          InsertNode(Edges[dir][dir ? v : u], ToEdge(dir ? u : v), true);
          if(dir) updates.push_back(make_tuple(u, v, i, 'A'));
          S[dir][dir ? v : u] += Edges[dir][(dir ? u : v)].size(); // add second level vertex
          break;
          case 'D':
          DeleteNode(Edges[dir][dir ? v : u], ToEdge(dir ? u : v), true);
          if(dir) updates.push_back(make_tuple(u, v, i, 'D'));
          S[dir][dir ? v : u] -= Edges[dir][(dir ? u : v)].size(); // delete second level vertex
          break;
          case 'Q':
          if(dir) queries.push_back(make_tuple(u, v, i));
          break;
          default:
          assert(false);
        }
      }
    }
    
    // cerr << "Num of nodes " << Edges[0].size() << endl;
    // //print graph
    // for (uint32_t v = 0; v < Node_Num; v++){
    //   for (uint32_t w : Edges[0][v]){
    //       cerr << v << " "<< GetID(w) << endl;
    //   }
    // }

    sort(updates.begin(), updates.end());
    
    vector<int> res(queries.size());

    cilk_for (size_t i = 0; i < queries.size(); i++){
      //double qstart = jlog_internal::get_current_time_sec();
      auto q = queries[i];
      //cerr << get<0>(q) << " "<< get<1>(q) << " " << get<2>(q) << endl;  
      res[i] = QueryDistance(get<0>(q), get<1>(q), get<2>(q), updates);
      //query_time_thread[__cilkrts_get_worker_number()] += jlog_internal::get_current_time_sec() - qstart;
    }

    for (auto upd : updates){
      char cmd; int u, v, t;
      tie(u, v, t, cmd) = upd;

      switch (cmd){
        case 'A':
        InsertNode(Edges[0][u], ToEdge(v), false);
        InsertNode(Edges[1][v], ToEdge(u), false);
        break;
        case 'D':
        DeleteNode(Edges[0][u], ToEdge(v), false);
        DeleteNode(Edges[1][v], ToEdge(u), false);
        break;
        //default:
        //assert(false);
      }
    }
    queries.clear();
    updates.clear();

    //process_batch_time += jlog_internal::get_current_time_sec() - process_start;
    return res;
  }
  
  BigGraph::~BigGraph(){
  
 }
