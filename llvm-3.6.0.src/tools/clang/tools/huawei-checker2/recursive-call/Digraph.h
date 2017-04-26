#ifndef _DIGRAPH_H
#define _DIGRAPH_H

#include <iostream>
#include <set>
#include <stdexcept>
#include <vector>

class Digraph {
private:
  int V;
  int E;
  std::vector<std::vector<int>> adj;

public:
  Digraph(int V);

  bool hasEdge(int v, int w);

  void addEdge(int v, int w);
  void removeEdge(int v, int w);
  void validateVertex(int v);
  
  int numOfVertices();
  int numOfEdges();
  
  //std::vector<std::pair<int, int>> getEdges(std::vector<int> vertices);

  std::vector<int> adjacentEdges(int v);
  std::vector<std::vector<int>> getAdjList();

  Digraph reverse();

  friend std::ostream& operator<<(std::ostream& out, const Digraph& G);
};

#endif
