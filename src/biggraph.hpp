#ifndef BIGGRAPH
#define BIGGRAPH

#include <vector>
#include <tuple>
#include <set>
#include <map>
#include <array>
#include <stdint.h>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <unordered_map>

#define fst first
#define snd second
template <typename T> class TwoLevelQueue {
  std::vector<T> data;
  size_t V;
  size_t curr_;
  size_t next_;
  size_t end_;
    
public:
  explicit TwoLevelQueue(size_t V) : data(V), V(V), curr_(0), next_(0), end_(0){
  }
  // virtual ~TwoLevelQueue(){ delete [] data; }
  inline bool empty() const { return curr_ == next_; }
  inline bool full() const { return end_ == V; }
  inline T &front() { return data[curr_];}
  inline size_t size() const { return end_; }
  inline void pop() { ++curr_; assert(curr_ <= end_);}
  inline void push(const T &val){ data[end_++] = val; assert(end_ <= V);}
  inline void next() { assert(curr_ == next_); next_ = end_; }
  inline void clear() { curr_ = next_ = end_ = 0; }
  inline void resize(size_t V_){
    if (V_ > V){ V = V_; data.resize(V); }
  }
  
  inline typename std::vector<T>::iterator begin() { return data.begin();}
  inline typename std::vector<T>::iterator end() { return data.begin() + end_;}
};

class BigGraph {
  typedef std::tuple<char, int, int> query_t;
  const uint32_t ALIVE   = 0;
  const uint32_t DEAD    = 1;
  const uint32_t UNKNOWN = 2;
  const uint32_t MASK    = 3;
public:  
  inline uint32_t GetID(uint32_t v) const { return v >> 2; }
  inline uint32_t GetEdgeState(uint32_t v) const { return v & MASK; }
  inline uint32_t ToEdge(uint32_t v) const { assert(v >= 0); return (v << 2) | ALIVE;}


  std::vector<std::tuple<int, int, int, char> > updates; // <source, target, timestamp, type>
  std::vector<std::tuple<int, int, int> > queries; // <source, target, timestamp>
  std::vector<std::vector<uint32_t> > Edges[2];
  std::vector<std::vector<uint32_t> > Ds;
  std::vector<std::vector<TwoLevelQueue<int> > > Qs;
  std::vector<uint32_t> S[2];
  std::vector<int> W[2];
  
  void InsertNode(std::vector<uint32_t> &vs, uint32_t v, bool);
  void DeleteNode(std::vector<uint32_t> &vs, uint32_t v, bool);
  int inline QueryDistance(int u, int v, int time,
                           const std::vector<std::tuple<int, int, int, char> > &updates);
  std::vector<int> ProcessBatch(const std::vector<query_t> &batch);
  void Build();    
  uint32_t  Node_Num = 0;
  ~BigGraph();
  
};

#endif /* BIGGRAPH */
