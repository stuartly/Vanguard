#include "TarjanSCC.h"

#include <algorithm>

#include "Digraph.h"

TarjanSCC::TarjanSCC(Digraph G) {

  index = 0;
  scc_count = 0;

  for (int i = 0; i < G.numOfVertices(); ++i) {
    marked.push_back(false);
    stacked.push_back(false);
    idList.push_back(-1);
    low.push_back(-1);
  }

  for (int v = 0; v < G.numOfVertices(); ++v) {
    if (!marked[v]) {
      dfs(G, v);
    }
  }

}

int TarjanSCC::count() {
  return scc_count;
}

bool TarjanSCC::stronglyConnected(int v, int w) {
  return idList[v] == idList[w];
}

int TarjanSCC::id(int v) {
  return idList[v];
}

void TarjanSCC::dfs(Digraph G, int v) {

  marked[v] = true;
  idList[v] = index; 
  low[v] = index;
  index++;
  sccStack.push(v);
  stacked[v] = true;


  for (int w : G.adjacentEdges(v)) {
    if (!marked[w]) {
      dfs(G, w);
      low[v] = std::min(low[v], low[w]);
    } else if (stacked[w]) {
      low[v] = std::min(low[v], idList[w]); 
    }
  }
  
  int w;
  if (low[v] == idList[v]) {
    scc_count++; 
    do {
      w = sccStack.top();
      sccStack.pop();
      stacked[w] = false;
      idList[w] = scc_count;
      low[w] = G.numOfVertices();
    } while(w != v);
  }

}


