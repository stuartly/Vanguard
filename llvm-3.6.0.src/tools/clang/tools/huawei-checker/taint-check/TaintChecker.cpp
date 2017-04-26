
#include "taint-check/TaintChecker.h"
#include "taint-check/TaintInterface.h"

namespace taint {

std::vector<FunctionEnvironment *> TaintChecker::functions;
std::set<std::string> TaintChecker::blackList;
std::set<std::string> TaintChecker::whiteList;

void TaintChecker::init(ASTFunction *F, FunctionDecl *FD, CFG *function_cfg) {

  assert(F->getName() == FD->getNameAsString());
  functions.push_back(new FunctionEnvironment(F, FD, function_cfg));
}

TaintChecker::~TaintChecker() {

  for (auto F : functions) {
    delete F;
  }
}

void TaintChecker::check(ASTFunction *F, FunctionDecl *FD, CFG *function_cfg) {

  clock_t start = clock();

  functions[F->getID()]->visitCFGBlocks(function_cfg);

  run_time += clock() - start;
}

void TaintChecker::build(std::vector<ASTFunction *> &topOrder) {

  const std::vector<ASTFunction *> &top =
      common::call_graph->getTopLevelFunctions();

  if (top.size() == 0) {
    functions[topOrder[0]->getID()]->param_values =
        (1 << topOrder[0]->getParamSize()) - 1;
  } else {
    for (auto F : top) {
      functions[F->getID()]->param_values = (1 << F->getParamSize()) - 1;
    }
  }

  for (auto F : topOrder) {
    functions[F->getID()]->emit_param_values();
  }
}

bool TaintChecker::is_tainted(FunctionDecl *F, CFGBlock *B, Stmt *S, Expr *E) {

  return getExprTaintValue(F, B, S, E).is_tainted();
}
}
