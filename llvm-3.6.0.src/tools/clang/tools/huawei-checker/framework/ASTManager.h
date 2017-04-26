
#ifndef AST_MANAGER_H
#define AST_MANAGER_H

#include "framework/ASTQueue.h"

class ASTManager {

public:
  ASTManager(std::vector<std::string> &ASTFiles);

  void push(std::string AST);

  void load(std::string AST, std::vector<ASTFunction *> &ast_functions,
            std::vector<FunctionDecl *> &functions);
  FunctionDecl *load(ASTFunction *F);

  void clear();

  FunctionDecl *getFunctionDecl(ASTFunction *F);
  ASTVariable *getASTVariable(VarDecl *VD, ASTFunction *F);

  CFG *getCFG(ASTFunction *F, FunctionDecl *FD);

private:
  ASTQueue priorityQueue;

  unsigned max_size;
  unsigned current_size;

  std::queue<std::unique_ptr<ASTUnit>> queue;
  std::unique_ptr<ASTUnit> tempASTUnit;

  std::unordered_map<std::string, std::unique_ptr<ASTMap>> maps;

  std::unordered_map<std::string, std::set<unsigned>> filter_functions;
};

#endif
