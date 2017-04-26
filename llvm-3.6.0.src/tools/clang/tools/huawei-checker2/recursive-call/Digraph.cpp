#include "Digraph.h"

#include <algorithm>
#include <iomanip>
#include <string>

Digraph::Digraph(int V) {
  this->V = V;
  this->E = 0;
  adj.resize(V, std::vector<int>());
}

void Digraph::addEdge(int v, int w) {
  validateVertex(v);
  validateVertex(w);
  adj[v].push_back(w); 
  E++;
}

void Digraph::removeEdge(int v, int w) {
  validateVertex(v);
  validateVertex(w);
  auto pos = std::find(adj[v].begin(), adj[v].end(), w);
  if (pos != adj[v].end()) {
    adj[v].erase(pos);
  }
}

bool Digraph::hasEdge(int v, int w) {
  validateVertex(v);
  validateVertex(w);
  return std::find(adj[v].begin(), adj[v].end(), w) != adj[v].end(); 
}

std::vector<int> Digraph::adjacentEdges(int v) {
  validateVertex(v);
  return adj[v];
}

std::vector<std::vector<int>> Digraph::getAdjList() {
  return this->adj;
}
  
int Digraph::numOfVertices() {
  return this->V;
}

int Digraph::numOfEdges() {
  return this->E;
}

Digraph Digraph::reverse() {
  Digraph R(V);
  for (int v = 0; v < V; ++v) {
    for (int w : adj[v]) {
      R.addEdge(w, v);
    }
  }
  return R;
}

void Digraph::validateVertex(int v) {
  if (v < 0 || v > V) {
    std::string msg;
    msg += "Vertex index ";
    msg += std::to_string(v);
    msg += " out of range!";
    std::cerr << msg << std::endl;
  }
  
}


//std::vector<std::pair<int, int>> Digraph::getEdges(std::vector<int> vertices) {
//  std::vector<std::pair<int, int>> edges; 
//  for (auto i = vertices.begin(); i != vertices.end(); ++i) {
//    std::set<int> edgesOfVertice = adj[*i];
//    for (auto j = edgesOfVertice.begin(); j != edgesOfVertice.end(); ++j) {
//      if (std::find(vertices.begin(), vertices.end(), *i) != vertices.end()) {
//        edges.push_back(std::pair<int, int>(*i, *j));
//      }
//    }
//  }
//  return edges;
//}

std::ostream& operator<<(std::ostream& out, const Digraph& G) {
  out << G.V << " vertices, " << G.E << " edges" << std::endl;
  for (int v = 0; v < G.V; ++v) {
    out << std::right << std::setw(5) << v << ": ";
    for (int w : G.adj[v]) {
      out << std::right << std::setw(5) << w;
    }
    out << std::endl;
  }
  return out;    
}
