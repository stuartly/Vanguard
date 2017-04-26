
#include "framework/Common.h"

#include "taint-check/TaintEnv.h"
#include "taint-check/TaintChecker.h"

namespace taint {

Environment operator+(const Environment &env1, const Environment &env2) {

  assert(env1.values.size() == env2.values.size() && env1.F == env2.F);

  Environment result = env1;

  for (unsigned i = 0; i < env1.values.size(); i++) {
    result.values[i].update(env1.values[i] + env2.values[i]);
  }
  result.return_value.update(env1.return_value + env2.return_value);
  return result;
}

bool operator==(const Environment &env1, const Environment &env2) {

  assert(env1.values.size() == env2.values.size() && env1.F == env2.F);

  for (unsigned i = 0; i < env1.values.size(); i++) {
    if (env1.values[i] != env2.values[i])
      return false;
  }

  return (env1.return_value == env2.return_value);
}

bool operator!=(const Environment &env1, const Environment &env2) {
  return !(env1 == env2);
}

TaintValueV visitCallExpr(CallExpr *E, Environment *env) {

  ASTFunction *call_function =
      common::call_graph->getFunction(E->getDirectCallee());
  if (call_function == nullptr) {
    return TaintValueV(TaintKind::Tainted);
  }
  if (TaintChecker::blackList.count(call_function->getFullName()) != 0) {
    return TaintValueV(TaintKind::Tainted);
  }
  if (TaintChecker::whiteList.count(call_function->getFullName()) != 0) {
    return TaintValueV(TaintKind::Untainted);
  }

  std::vector<TaintValueV> values;
  if (CXXMemberCallExpr *CXXCE = dyn_cast<CXXMemberCallExpr>(E)) {
    values.push_back(visitStmt(CXXCE->getImplicitObjectArgument(), env));
  }

  for (auto arg : E->arguments()) {
    values.push_back(visitStmt(arg, env));
  }

  TaintChecker::functions[call_function->getID()]->add_arg_values(call_function,
                                                                  values);

  TaintValueV returnValue =
      TaintChecker::functions[call_function->getID()]->environment.return_value;

  if (!TaintChecker::functions[call_function->getID()]->is_visited) {
    returnValue = TaintValueV((1 << call_function->getParamSize()) - 1);
  } else {
    for (TaintValueV tv : values) {
      if (tv.V != nullptr && tv.V->isPointerOrReferenceType()) {
        tv.update(combineValues(tv, values), true);
      }
    }
  }
  return combineValues(returnValue, values);
}

TaintValueV visitCXXConstructExpr(CXXConstructExpr *CXXE, Environment *env) {

  std::vector<TaintValueV> values;

  for (auto arg : CXXE->arguments()) {
    values.push_back(visitStmt(arg, env));
  }

  TaintValueV returnValue = TaintValueV((1 << CXXE->getNumArgs()) - 1);

  returnValue = combineValues(returnValue, values);

  ASTFunction *call_function =
      common::call_graph->getFunction(CXXE->getConstructor());
  if (call_function != nullptr) {
    values.insert(values.begin(), TaintValueV());
    TaintChecker::functions[call_function->getID()]->add_arg_values(
        call_function, values);
  }

  return returnValue;
}

TaintValueV visitStmt(Stmt *S, Environment *env) {

  if (S == nullptr || env == nullptr) {
    return TaintValueV();
  }

  if (S->getStmtClass() == Stmt::IntegerLiteralClass ||
      S->getStmtClass() == Stmt::FloatingLiteralClass) {
    return TaintValueV();
  }

  // sizeof
  if (S->getStmtClass() == Stmt::UnaryExprOrTypeTraitExprClass) {
    return TaintValueV();
  }

  if (dyn_cast<CXXThisExpr>(S)) {
    return (*env).values[0];
  }

  if (ParenExpr *E = dyn_cast<ParenExpr>(S)) {
    return visitStmt(E->getSubExpr(), env);
  }

  if (UnaryOperator *E = dyn_cast<UnaryOperator>(S)) {
    return visitStmt(E->getSubExpr(), env);
  }

  if (CastExpr *E = dyn_cast<CastExpr>(S)) {
    return visitStmt(E->getSubExpr(), env);
  }

  if (MemberExpr *E = dyn_cast<MemberExpr>(S)) {
    return visitStmt(E->getBase(), env);
  }

  if (ArraySubscriptExpr *E = dyn_cast<ArraySubscriptExpr>(S)) {
    return visitStmt(E->getBase(), env);
  }

  if (ConditionalOperator *E = dyn_cast<ConditionalOperator>(S)) {
    return visitStmt(E->getTrueExpr(), env) + visitStmt(E->getFalseExpr(), env);
  }

  if (CXXNewExpr *E = dyn_cast<CXXNewExpr>(S)) {
    return visitStmt(const_cast<CXXConstructExpr *>(E->getConstructExpr()),
                     env);
  }

  if (CXXConstructExpr *E = dyn_cast<CXXConstructExpr>(S)) {
    return visitCXXConstructExpr(E, env);
  }

  if (CallExpr *E = dyn_cast<CallExpr>(S)) {
    return visitCallExpr(E, env);
  }

  if (ReturnStmt *E = dyn_cast<ReturnStmt>(S)) {
    if (E->getRetValue() != nullptr) {
      env->return_value.update(visitStmt(E->getRetValue(), env));
    }
    return TaintValueV();
  }

  if (DeclRefExpr *DRE = dyn_cast<DeclRefExpr>(S)) {
    VarDecl *VD = dyn_cast<VarDecl>(DRE->getDecl());

    ASTVariable *V = common::manager->getASTVariable(VD, env->F);
    if (V == nullptr)
      return TaintValueV();
    return (*env).values[V->getID()];
  }

  if (DeclStmt *DS = dyn_cast<DeclStmt>(S)) {
    for (Decl *D : DS->decls()) {
      VarDecl *VD = dyn_cast<VarDecl>(D);

      if (VD == nullptr || !(VD->hasInit()))
        continue;

      ASTVariable *V = common::manager->getASTVariable(VD, env->F);
      if (V == nullptr)
        continue;
      if (V->isPointerOrReferenceType())
        (*env).values[V->getID()].update(visitStmt(VD->getInit(), env), true);
      else
        (*env).values[V->getID()].update(visitStmt(VD->getInit(), env));
    }
    return TaintValueV();
  }

  if (BinaryOperator *E = dyn_cast<BinaryOperator>(S)) {

    TaintValueV RValue = visitStmt(E->getRHS(), env);
    TaintValueV LValue = visitStmt(E->getLHS(), env);

    if (E->isAssignmentOp()) {

      if (LValue.V != nullptr && LValue.V->isPointerOrReferenceType()) {
        LValue.update(RValue, true);
        return RValue;
      }
      if (E->isCompoundAssignmentOp()) {
        LValue.update(LValue + RValue);
        return LValue;
      }
      LValue.update(RValue);
      return LValue;
    }
    return RValue + LValue;
  }

  return TaintValueV();
}
}
