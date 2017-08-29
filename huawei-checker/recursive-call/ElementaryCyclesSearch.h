#ifndef _ELEMENTARY_CYCLES_SEARCH_H
#define _ELEMENTARY_CYCLES_SEARCH_H

#include <algorithm>
#include <iostream>
#include <set>
#include <vector>

#include "Digraph.h"

class SCCResult {
public:
  SCCResult();
  SCCResult(std::vector<std::vector<int>> adjList, int lowestNodeId);
  std::vector<std::vector<int>> getAdjList();
  int getLowestNodeId();
  bool empty();
private:  
  std::set<int> nodeIDsOfSCC;
  std::vector<std::vector<int>> adjList;
  int lowestNodeId;
};



class StrongConnectedComponents {
public:
  StrongConnectedComponents(std::vector<std::vector<int>> adjList) : adjListOrjginal(adjList), sccCounter(0) {}

  
  SCCResult getAdjacencyList(int node);
private:  
  
  
  void makeAdjListSubgraph(int node);

  std::vector<int> getLowestIdComponent();

  std::vector<std::vector<int>> getAdjList(std::vector<int> nodes);

  void getStrongConnectedComponents(int root);

  /** Adjacency-list of original graph */
  std::vector<std::vector<int>> adjListOrjginal;

  /** Adjacency-list of currently viewed subgraph */
  std::vector<std::vector<int>> adjList;

  /** Help attribute for finding scc's */
  std::vector<bool> visited;
  std::vector<int> instack;
  std::vector<int> lowlink;
  std::vector<int> number;
  int sccCounter;
  std::vector<std::vector<int>> currentSCCs;

};


class ElementaryCyclesSearch {
public:
  ElementaryCyclesSearch(Digraph G) : G(G) {}
  std::vector<std::vector<int>> getElementaryCycles();
  std::vector<std::vector<int>> getElementaryCycles(std::vector<std::vector<int>> adjList);

private:
  /** Directed graph */
  Digraph G;

  /** List of cycles */
  std::vector<std::vector<int>> cycles; 

  /** Adjacency-list of graph */
  std::vector<std::vector<int>> adjList;

  /** Blocked nodes, used by the algorithm of Johnson */
  std::vector<bool> blocked;

  /** B-Lists, used by the algorithm of Johnson */
  std::vector<std::vector<int>> B;

  /** Stack for nodes, used by the algorithm of Johnson */
  std::vector<int> stack;

  bool findCycles(int v, int s, std::vector<std::vector<int>> adjList);

  void unblock(int node);
};

//int main() {
//  std::vector<std::vector<int>> adjList;
//  std::vector<int> adj;
//  
//  adj.push_back(1);
//  adj.push_back(2);
//  adj.push_back(3);
//  adjList.push_back(adj);
//  adj.clear();
//
//  adj.push_back(0);
//  adj.push_back(2);
//  adj.push_back(3);
//  adjList.push_back(adj);
//  adj.clear();
//
//  adj.push_back(0);
//  adj.push_back(1);
//  adj.push_back(3);
//  adjList.push_back(adj);
//  adj.clear();
//  
//  adj.push_back(0);
//  adj.push_back(1);
//  adj.push_back(2);
//  adjList.push_back(adj);
//  adj.clear();
//
//  adj.push_back(5);
//  adj.push_back(6);
//  adjList.push_back(adj);
//  adj.clear();
//
//  adj.push_back(3);
//  adjList.push_back(adj);
//  adj.clear();
//
//  adj.push_back(1);
//  adj.push_back(7);
//  adjList.push_back(adj);
//  adj.clear();
//
//  adj.push_back(8);
//  adjList.push_back(adj);
//  adj.clear();
//
//  adj.push_back(6);
//  adjList.push_back(adj);
//  adj.clear();
//
//  ElementaryCyclesSearch ecs(adjList);
//  std::vector<std::vector<int>> cycles = ecs.getElementaryCycles();
//  for (int i = 0; i < cycles.size(); ++i) {
//    for (int j = 0; j < cycles[i].size(); ++j) {
//      std::cerr << cycles[i][j] << " ";
//    }
//    std::cerr << "\n";
//  }
//  
//  StrongConnectedComponents scc(adjList);
//  for (int i = 0; i < adjList.size(); ++i) {
//    std::cerr << "i: " << i << "\n";
//    for (int j : adjList[i]) {
//      std::cerr << j << " ";
//    }
//    std::cerr << "\n";
//    SCCResult r = scc.getAdjacencyList(i);
//    if (!r.empty()) {
//      std::vector<std::vector<int>> al = r.getAdjList();
//      for (int j = i; j < al.size(); ++j) {
//        if (al[j].size() > 0) {
//          std::cerr << "j: " << j;
//          for (int k = 0; k < al[j].size(); ++k) {
//            std::cerr << " _" << al[j].at(k);
//          }
//          std::cerr << "\n";
//        }
//      }
//      std::cerr << "\n";
//    }
//  }
//
//}

#endif 
