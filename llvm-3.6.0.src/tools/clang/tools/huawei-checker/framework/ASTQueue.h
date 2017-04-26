
#ifndef AST_QUEUE_H
#define AST_QUEUE_H

#include <queue>
#include <unordered_set>
#include <unordered_map>

#include "framework/ASTElement.h"

#include "clang/Analysis/CFG.h"
#include "clang/Frontend/ASTUnit.h"

using namespace clang;

class ASTMap {

public:
  void buildMap(
      ASTUnit *AU,
      std::unordered_map<std::string, std::set<unsigned>> &filter_functions,
      bool build_variable = false);

  FunctionDecl *getFunction(ASTFunction *F) {
    auto it = functionMap.find(F);
    if (it == functionMap.end()) {
      return nullptr;
    }
    return it->second;
  }

  ASTVariable *getVariable(VarDecl *VD) {
    auto it = variableMap.find(VD);
    if (it == variableMap.end()) {
      return nullptr;
    }
    return it->second;
  }

  CFG *getCFG(ASTFunction *F, FunctionDecl *FD) {

    if (CFGs.count(F) == 0) {

      std::unique_ptr<CFG> functionCFG = CFG::buildCFG(
          FD, FD->getBody(), &FD->getASTContext(), CFG::BuildOptions());
      CFGs[F] = std::move(functionCFG);
    }
    return CFGs[F].get();
  }

  friend class ASTQueue;
  friend class ASTManager;

private:
  std::unordered_map<ASTFunction *, FunctionDecl *> functionMap;
  std::unordered_map<VarDecl *, ASTVariable *> variableMap;

  std::unordered_map<ASTFunction *, std::unique_ptr<CFG>> CFGs;

  void addFunction(ASTFunction *F, FunctionDecl *FD) {
    assert(F != nullptr);
    assert(FD != nullptr);
    functionMap[F] = FD;
  }
  void addVariable(ASTVariable *V, VarDecl *VD) { variableMap[VD] = V; }

  std::vector<FunctionDecl *> getFunctions() {
    std::vector<FunctionDecl *> functions;
    for (auto &content : functionMap) {
      functions.push_back(content.second);
    }
    return functions;
  }
};

class ASTUnitT {

public:
  ASTUnitT(ASTUnit *AU, clock_t load_time) : AU(AU), load_time(load_time) {}

  ASTUnit *get() { return AU; }

  bool operator<(const ASTUnitT &AUT) const {
    return load_time > AUT.load_time;
  }

private:
  ASTUnit *AU;
  clock_t load_time;
};

class ASTQueue {

public:
  ASTQueue();
  ASTUnit *push_back(
      std::string AST, std::unordered_set<std::string> &unique_functions,
      std::unordered_map<std::string, std::set<unsigned>> &filter_functions,
      std::vector<FunctionDecl *> &return_functions,
      std::vector<ASTFunction *> &return_ast_functions);

  void
  load(std::string AST,
       std::unordered_map<std::string, std::set<unsigned>> &filter_functions,
       std::vector<ASTFunction *> &return_ast_functions,
       std::vector<FunctionDecl *> &return_functions);

  FunctionDecl *
  load(ASTFunction *F,
       std::unordered_map<std::string, std::set<unsigned>> &filter_functions);

  friend class ASTManager;

private:
  void pop_front();

  std::unique_ptr<ASTUnit> tempASTUnit;

  unsigned max_size;
  std::priority_queue<ASTUnitT> priorityQueue;
  std::unordered_map<std::string, std::unique_ptr<ASTUnit>> collection;

  std::unordered_map<std::string, std::unique_ptr<ASTMap>> maps;
};

#endif
