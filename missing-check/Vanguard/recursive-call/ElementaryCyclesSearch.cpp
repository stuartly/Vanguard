#include "ElementaryCyclesSearch.h"
#include <map> 
#include "Bimap.h"
#include "TarjanSCC.h"

/**
 * ------------------------------------------------
 * class ElementaryCyclesSearch implementation
 * ------------------------------------------------
 */

/**
 * Returns List::List::int with the Lists of node indexes of all elementary
 * cycles in the graph.
 *
 * @return List::List::int with the Lists of the elementary cycles.
 */
std::vector<std::vector<int>> ElementaryCyclesSearch::getElementaryCycles() {
  std::vector<std::vector<int>> result;
  std::vector<int> cycle;
  // handle loops
  for (int i = 0; i < G.numOfVertices(); ++i) {
    if (G.hasEdge(i, i)) {
      cycle.clear();
      cycle.push_back(i);
      result.push_back(cycle);
      G.removeEdge(i, i);
    }
  } 

  TarjanSCC sccFinder(G);
  std::map<int, std::vector<int>> sccs;
  for (int i = 0; i < G.numOfVertices(); ++i) {
    std::map<int, std::vector<int>>::iterator itr = sccs.find(sccFinder.id(i));
    std::vector<int> v;
    if (itr != sccs.end()) {
      v = sccs[sccFinder.id(i)];
      v.push_back(i);
      sccs[sccFinder.id(i)] = v;
    }
    else{
      v.push_back(i);
      sccs[sccFinder.id(i)] = v;
    }
  }

  std::map<int, std::vector<int>>::iterator itr;
  itr = sccs.begin();
  while (itr != sccs.end()) {
    if (itr->second.size() == 1) {
      itr = sccs.erase(itr);
    }
    else {
      ++itr;
    }
  }

  itr = sccs.begin();
  while (itr != sccs.end()) {
    Bimap<int, int> vertexMap; 
    std::vector<int> vertexes = itr->second;
    Digraph subgraph(vertexes.size());
    
    for (unsigned i = 0; i < vertexes.size(); ++i) {
      vertexMap.add(i, vertexes[i]);
    }

    for (unsigned i = 0; i < vertexes.size(); ++i) {
      for (unsigned j = 0; j < vertexes.size(); ++j) {
        if (G.hasEdge(vertexes[i], vertexes[j])) {
          subgraph.addEdge(i, j);
        }
      }
    }
    
    std::vector<std::vector<int>> cyclesInSubgraph = getElementaryCycles(subgraph.getAdjList());
    for (unsigned i = 0; i < cyclesInSubgraph.size(); ++i) {
      std::vector<int> subc = cyclesInSubgraph[i]; 
      std::vector<int> oric;
      for (auto v : subc) {
        oric.push_back(vertexMap.getRight(v));
      }
      result.push_back(oric);
    }
    ++itr; 
  }
  
  return result;
}


/**
 * Returns List::List::int with the Lists of node indexes of all elementary
 * cycles in the given adjacency list.
 *
 * @return List::List::int with the Lists of the elementary cycles.
 */
std::vector<std::vector<int>> ElementaryCyclesSearch::getElementaryCycles(std::vector<std::vector<int>> list) {

  // Initialize variables
  this->cycles.clear();
  this->adjList = list;
  blocked.clear();
  B.clear();

  for (unsigned i = 0; i < this->adjList.size(); ++i) {
    this->blocked.push_back(false);
    this->B.push_back(std::vector<int>());
  }

  StrongConnectedComponents sccs(this->adjList);
  int s = 0;
  
  while(true) {
    SCCResult sccResult = sccs.getAdjacencyList(s);
    if (!sccResult.empty() && !sccResult.getAdjList().empty()) {
      std::vector<std::vector<int>> scc = sccResult.getAdjList();
      s = sccResult.getLowestNodeId();
      for (unsigned j = 0; j < scc.size(); ++j) {
        if (!scc.at(j).empty()) {
          this->blocked[j] = false;
        }
      }
      this->findCycles(s, s, scc);
      ++s;
    } else {
      break;
    }
  }
  return this->cycles;
}

/**
 * Calculates the cycles containing a given node in a strongly connected
 * component. The method calls itself recursivly.
 *
 * @param v
 * @param s
 * @param adjList adjacency-list with the subgraph of the strongly
 * connected component s is part of.
 * @return true, if cycle found; false otherwise
 */
bool ElementaryCyclesSearch::findCycles(int v, int s, std::vector<std::vector<int>> adjList) {

  bool f = false;
  this->stack.push_back(v);
  this->blocked[v] = true;

  for (unsigned i = 0; i < adjList[v].size(); ++i) {
    int w = adjList[v][i];
    // find cycle
    if (w == s) {
      std::vector<int> cycle;
      for (unsigned j = 0; j < this->stack.size(); ++j) {
        int index = this->stack[j];
        cycle.push_back(index);
      }
      this->cycles.push_back(cycle);
      f = true;
    } else if (!this->blocked[w]) {
      if (this->findCycles(w, s, adjList)) {
        f = true;
      }
    }
  }

  if (f) {
    this->unblock(v);
  } else {
    for (unsigned i = 0; i < adjList[v].size(); ++i) {
      int w = adjList[v][i];
      if (std::find(this->B[w].begin(), this->B[w].end(), v) == this->B[w].end()) {
        this->B[w].push_back(v);
      }
    }
  }

  auto pos = std::find(this->stack.begin(), this->stack.end(), v);
  if (pos != this->stack.end()) {
    this->stack.erase(pos);
  }

  return f;
}

/**
 * Unblocks recursivly all blocked nodes, starting with a given node.
 *
 * @param node node to unblock
 */
void ElementaryCyclesSearch::unblock(int node) {
  this->blocked[node] = false;
  std::vector<int> Bnode = this->B[node];
  while (Bnode.size() > 0) {
    int w = Bnode[0];
    Bnode.erase(Bnode.begin());
    if (this->blocked[w]) {
      this->unblock(w);
    } 
  }
}


/**
 * ------------------------------------------------
 * class SCCResult implementation
 * ------------------------------------------------
 */
SCCResult::SCCResult() {
  this->lowestNodeId = -1;
}

SCCResult::SCCResult(std::vector<std::vector<int>> adjList, int lowestNodeId) {
  this->adjList = adjList;
  this->lowestNodeId = lowestNodeId;
  for (unsigned i = this->lowestNodeId; i < this->adjList.size(); ++i) {
    if (this->adjList.at(i).size() > 0) {
      this->nodeIDsOfSCC.insert(i);
    }
  }
}

std::vector<std::vector<int>> SCCResult::getAdjList() {
  return this->adjList;
}

int SCCResult::getLowestNodeId() {
  return this->lowestNodeId;
}

bool SCCResult::empty() {
  return this->lowestNodeId == -1;
}


/**
 * ------------------------------------------------
 * class StrongConnectedComponents implementation
 * ------------------------------------------------
 */

/**
 * This method returns the adjacency-structure of the strong connected
 * component with the least vertex in a subgraph of the original graph
 * induced by the nodes {s, s + 1, ..., n}, where s is a given node. Note
 * that trivial strong connected components with just one node will not
 * be returned.
 *
 * @param node 
 * @return SCCResult with adjacency-structure of the strong
 * connected component; null, if no such component exists
 */
SCCResult StrongConnectedComponents::getAdjacencyList(int node) {
  visited.clear();
  lowlink.clear();
  number.clear();
  instack.clear();
  currentSCCs.clear();
  for (unsigned i = 0; i < this->adjListOrjginal.size(); ++i) {
    visited.push_back(false);
    lowlink.push_back(-1);
    number.push_back(-1);
  }

  this->makeAdjListSubgraph(node);

  for (unsigned i = node; i < this->adjListOrjginal.size(); ++i) {
    if (!this->visited[i]) {
      this->getStrongConnectedComponents(i);
      std::vector<int> nodes = this->getLowestIdComponent();
      if ((!nodes.empty()) && (std::find(nodes.begin(), nodes.end(), node) == nodes.end()) && (std::find(nodes.begin(), nodes.end(), node + 1) == nodes.end())) {
        return this->getAdjacencyList(node + 1);
      } else {
        std::vector<std::vector<int>> adjacencyList = this->getAdjList(nodes);
        if (!adjacencyList.empty()) {
          for (unsigned j = 0; j < this->adjListOrjginal.size(); ++j) {
            if (adjacencyList[j].size() > 0) {
              SCCResult result(adjacencyList, j);
              return result;
            }
          }
        }
      }
    }
  }
  SCCResult result;
  return result;
}

/**
 * Builds the adjacency-list for a subgraph containing just nodes
 * >= a given index.
 *
 * @param node Node with lowest index in the subgraph
 */
void StrongConnectedComponents::makeAdjListSubgraph(int node) {
  this->adjList.clear();
  for (unsigned i = 0; i < this->adjListOrjginal.size(); ++ i) {
    std::vector<int> adjs;
    this->adjList.push_back(adjs);
  }
  for (unsigned i = node; i < this->adjListOrjginal.size(); ++i) {
    std::vector<int> successors;
    for (unsigned j = 0; j < this->adjListOrjginal[i].size(); ++j) {
      if (this->adjListOrjginal[i][j] >= node) {
        successors.push_back(this->adjListOrjginal[i][j]);
      }
    }
    if (successors.size() > 0) {
      this->adjList[i] = successors;      
    }
  }
}

/**
 * Calculates the strong connected component out of a set of scc's, that
 * contains the node with the lowest index.
 *
 * @return Vector::Integer of the scc containing the lowest nodenumber
 */
std::vector<int> StrongConnectedComponents::getLowestIdComponent() {
  int min = this->adjList.size();
  std::vector<int> currScc;

  for (unsigned i = 0; i < this->currentSCCs.size(); ++i) {
    std::vector<int> scc = this->currentSCCs.at(i);
    for (unsigned j = 0; j < scc.size(); ++j) {
      int node = scc.at(j);
      if (node < min) {
        currScc = scc;
        min = node;
      }
    }
  }
  return currScc;
}

/**
 * @return Vector[]::Integer representing the adjacency-structure of the
 * strong connected component with least vertex in the currently viewed
 * subgraph
 */
std::vector<std::vector<int>> StrongConnectedComponents::getAdjList(std::vector<int> nodes) {
  std::vector<std::vector<int>> lowestIdAdjacencyList;
  
  if (!nodes.empty()) {
    for (unsigned i = 0; i < this->adjList.size(); ++i) {
      std::vector<int> adjs;
      lowestIdAdjacencyList.push_back(adjs);
    }
    for (unsigned i = 0; i < nodes.size(); ++i) {
      int node = nodes.at(i);
      for (unsigned j = 0; j < this->adjList[node].size(); ++j) {
        int succ = this->adjList[node][j];
        if (std::find(nodes.begin(), nodes.end(), succ) != nodes.end()) {
          lowestIdAdjacencyList[node].push_back(succ);
        }
      }
    }
  }
  return lowestIdAdjacencyList;
}

/**
 * Searchs for strong connected components reachable from a given node.
 *
 * @param root node to start from.
 */
void StrongConnectedComponents::getStrongConnectedComponents(int root) {
  this->sccCounter++;
  this->lowlink[root] = this->sccCounter;
  this->number[root] = this->sccCounter;
  this->visited[root] = true;
  this->instack.push_back(root);

  for (unsigned i = 0; i < this->adjList[root].size(); ++i) {
    int w = this->adjList[root][i];
    if (!this->visited[w]) {
      this->getStrongConnectedComponents(w);
      this->lowlink[root] = std::min(this->lowlink[root], this->lowlink[w]);
    } else if (this->number[w] < this->number[root]) {
      if (std::find(this->instack.begin(), this->instack.end(), w) != this->instack.end()) {
        lowlink[root] = std::min(this->lowlink[root], this->number[w]);
      } 
    }
  }

  // find scc
  if ((lowlink[root] == number[root]) && (instack.size() > 0)) {
    int next = -1;
    std::vector<int> scc;
    do {
      next = instack.back();
      instack.pop_back();
      scc.push_back(next);
    } while (this->number[next] > this->number[root]);

    // simple scc with just one node will not be added
    if (scc.size() > 1) {
      this->currentSCCs.push_back(scc);
    }
  }
}



