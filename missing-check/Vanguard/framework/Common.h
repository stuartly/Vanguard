
#ifndef BASE_COMMON_H
#define BASE_COMMON_H

#include <vector>
#include <iostream>

#include "clang/Frontend/ASTUnit.h"
#include "clang/AST/RecursiveASTVisitor.h"

#include "framework/Logger.h"
#include "framework/ASTManager.h"
#include "framework/CallGraph.h"

using namespace clang;

std::vector<std::string> initializeASTList(std::string astList);
Json::Value initializeConfigure(std::string config);
void initializePathReport(int argc, const char *argv[]);
void initializeBlackWhite(int argc, const char *argv[]);

void process_bar(float progress);


namespace common {

extern Config *configure;
extern ASTResource *resource;
extern CallGraph *call_graph;
extern ASTManager *manager;

std::unique_ptr<ASTUnit> loadFromASTFile(std::string AST);

std::vector<FunctionDecl *> getFunctions(ASTContext &Context);
std::vector<VarDecl *> getVariables(FunctionDecl *FD);

std::vector<FunctionDecl *> getCalledFunctions(FunctionDecl *FD);
std::vector<CallExpr *> getCallExpr(FunctionDecl *FD);

std::string getFullName(FunctionDecl *FD);

std::vector<ASTFunction *> getTopoOrder();

std::string getStringStmt(const Stmt* stmt,const LangOptions &LO);

} // end of namespace common

#endif

