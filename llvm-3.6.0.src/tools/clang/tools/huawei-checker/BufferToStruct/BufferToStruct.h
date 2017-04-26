#include <sstream>
#include <iostream>
#include <string>
#include <string.h>
#include <list>
#include <queue>
#include <vector>

#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Frontend/ASTUnit.h"
#include "clang/Lex/Lexer.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "llvm/Support/raw_ostream.h"
#include "clang/Analysis/CFG.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "../framework/Common.h"
#include "../framework/BasicChecker.h"
#include "../framework/CallGraph.h"
#include "../taint-check/TaintChecker.h"
#include "../library/xml/pugixml/pugixml.hpp"

using namespace clang;
using namespace llvm;
using namespace clang::driver;
using namespace clang::tooling;
using namespace std;

class BufferToStruct {
public:
  void check(ASTFunction *F, FunctionDecl *FD, CFG *function_cfg);
  BufferToStruct();
  ~BufferToStruct();

  pugi::xml_document doc;
  int WarningNum;
  clock_t get_time();

private:
  clock_t run_time;
  int CheckLevel;

  void visitStmt(ASTFunction *F, FunctionDecl *FD, CFG *function_cfg);
  void writingToXML(std::string fileName, std::string funName,
                    std::string descr, std::string locLine);
  void readConfig();
  string getString_of_Expr(const Expr *expr);
  string printStmt(const Stmt *stmt);
};
