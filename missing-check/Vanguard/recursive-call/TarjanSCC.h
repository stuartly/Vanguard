#ifndef _TARJAN_SCC_H
#define _TARJAN_SCC_H

#include <stack>
#include <vector>

#include "Digraph.h"

class TarjanSCC {
  public:
    TarjanSCC(Digraph G);

    int count();

    bool stronglyConnected(int v, int w);
    
    int id(int v);

  private:
    std::vector<bool> marked;
    std::vector<int> idList;
    std::vector<int> low;
    int index;
    int scc_count;
    std::vector<bool> stacked;
    std::stack<int> sccStack;

    void dfs(Digraph G, int v);

};

#endif
