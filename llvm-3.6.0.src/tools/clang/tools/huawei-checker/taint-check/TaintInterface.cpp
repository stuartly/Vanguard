
#include "framework/Common.h"

#include "taint-check/TaintEnv.h"
#include "taint-check/TaintChecker.h"
#include "taint-check/TaintInterface.h"

using namespace std;

namespace taint {

TaintValueV getExprTaintValue(FunctionDecl *FD, CFGBlock *B, Stmt *S, Expr *E) {

  if (FD == nullptr || B == nullptr || S == nullptr || E == nullptr)
    return TaintValueV();

  ASTFunction *F = common::call_graph->getFunction(FD);

  if (F == nullptr ||
      B->getBlockID() >= TaintChecker::functions[F->getID()]->blocks.size())
    return TaintValueV();

  if (TaintChecker::functions[F->getID()]->is_visited == false) {
    return TaintValueV();
  }

  Environment env =
      TaintChecker::functions[F->getID()]->blocks[B->getBlockID()]->environment;

  for (auto &it : *B) {
    const Stmt *stmt = it.castAs<CFGStmt>().getStmt();
    visitStmt(const_cast<Stmt *>(stmt), &env);
    if (S == stmt)
      break;
  }

  return visitStmt(E, &env);
}
void printTaintEnvironment() {
  for (auto &f_it : TaintChecker::functions) {
    cout << "[CHECK] [FunctionEnv] " << f_it->environment << endl;
    for (auto &b_it : f_it->blocks) {
      cout << "  [CHECK] [BlockEnv] " << b_it->environment << endl;
    }
  }
}
}
