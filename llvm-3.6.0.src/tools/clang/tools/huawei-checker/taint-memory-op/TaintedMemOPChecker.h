#ifndef TAINT_MEM_OP_CHECK_H
#define TAINT_MEM_OP_CHECK_H

#include <sstream>
#include <iostream>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <list>
#include <queue>
#include <vector>
#include <fstream>

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
//#include "../taint-check/TaintValue.h"
//#include "../taint-check/TaintEnv.h"
//#include "../taint-check/FunctionEnv.h"
#include "../library/xml/pugixml/pugixml.hpp"
#include "../library/libsvm/svm.h"
//#include "../PreCheckDataSet/PreCheckDataSet.h"



using namespace clang;
using namespace llvm;
using namespace clang::driver;
using namespace clang::tooling;
using namespace std;

using namespace taint;

class WarningItem {
private:
  string fileName;
  string callerFunction;
  string sensitiveFunction;
  string taintedArguments;
  string location;
  string description;
  string riskDegree;
  string line;

public:
  WarningItem(string fileName, string callerFunction, string sensitiveFunction,
              string taintedArguments, string location, string description,
              string riskDegree, string line) {
    this->fileName = fileName;
    this->callerFunction = callerFunction;
    this->sensitiveFunction = sensitiveFunction;
    this->taintedArguments = taintedArguments;
    this->location = location;
    this->description = description;
    this->riskDegree = riskDegree;
    this->line = line;
  }

  string &getFileName() { return this->fileName; }

  string &getCallerFunction() { return this->callerFunction; }

  string &getSensitiveFunction() { return this->sensitiveFunction; }

  string &getTaintedArguments() { return this->taintedArguments; }

  string &getLocation() { return this->location; }

  string &getDescription() { return this->description; }

  string &getRiskDegree() { return this->riskDegree; }

  string &getLine() { return this->line; }
};

struct Node {
  std::string NodeValue;
  struct Node *LHS;
  struct Node *RHS;
};

class Tree {
public:
  Node *root;
  Tree() { root = NULL; }
};

class ifcheckInfo {
private:
  string ifcheckcondtion;
  // vector<string> location;
  int count;

public:
  ifcheckInfo(string con) : ifcheckcondtion(con), count(0) {}

  string getIfcon() { return this->ifcheckcondtion; }

  int getCount() { return this->count; }

  void addloc() { this->count++; }

  bool operator<(ifcheckInfo &m) { return this->count < m.getCount(); }
};

class funReference {
private:
  string funName;
  vector<ifcheckInfo> iflist;

public:
  funReference(string funname) : funName(funname) {}

  string getFunName() { return this->funName; }

  vector<ifcheckInfo> &getList() { return this->iflist; }
};

class TaintedMemOPChecker {
public:
  void check(ASTFunction *F, FunctionDecl *FD, CFG *function_cfg);
  void visitCallExpr(CallExpr *callExpr, ASTFunction *caller);
  enum Color { WHITE, BLACK, GRAY };
  pugi::xml_document doc;
  pugi::xml_document checkSampleDoc;
  pugi::xml_document Label_Feature_Data_Doc;
  int WarningNum;
  int CheckSampleNum;
  int DataNum;
  clock_t get_time();
  clock_t get_Confirm_time();
  clock_t get_Repair_time();
  clock_t get_Detect_time();
  TaintedMemOPChecker();
  ~TaintedMemOPChecker();

private:
  clock_t run_time;
  clock_t detect_time;
  clock_t confirm_time;
  clock_t repair_time;

  bool StringMode;
  bool ASTMode;
  bool Z3Mode;
  bool MemopSimpleValue;
  bool ReferableRepair;
  bool WithTaint;
  std::string PreOrderString;
  std::string Tainted_Data;
  std::map<Expr*, string> Tainted_Data_Expr_Set;
  std::string Tainted_Data_Location;
  std::map<std::string, funReference> funReferenceMap;

  map<string, WarningItem> WarningList;

  FunctionDecl *RecordfunDel;

  CFGBlock *Recordblock;
  Stmt *Recordstmt;
  CallExpr *RecordcallExpr;
  FunctionDecl *RecordCallee;

  std::string Tainted_FileName;
  std::string Tainted_FunctionName;
  std::string Tainted_MemOPName;
  std::string Tainted_Locline;
  std::string Tainted_Description;

  std::string Caller2;
  std::string Caller3;
  std::string Tainted_MemOP_location;

  string location_of_Arg_str;
  int location_of_Arg[10];
  //std::set<Expr *> If_ConditionExpr_Set;
  std::map<Expr *, string> IfCheckInfoList;
  std::map<string, string> Function_Location;
  vector<Expr*>variableList;

  int MemOPCheckLevel;

  std::unordered_map<std::string, std::string> MemoryOPFunction;
  std::unordered_map<std::string, std::string> MemOPCheckStlye;
  std::unordered_map<std::string, std::string> RightIfCheckMode;
  std::unordered_map<std::string, std::string> SimpleExprCheckOnly;

  std::vector<ASTFunction *> getNonTopoOrder();
  void visitStmt(ASTFunction *F, FunctionDecl *FD, CFG *function_cfg);

  std::string getString_of_Expr(Expr *expr);
  bool is_MemOP(std::string calleeName);
  bool has_Tainted_Arg(CallExpr *callExpr, string location_of_Arg_str);
  bool is_Tainted(Expr *arg);
  bool has_ifCheck(ASTFunction *caller);
  bool is_right_ifCheck(string Tainted_MemOPName, 
                        string Tainted_MemOP_location, 
                        map<Expr *, string> Tainted_Data_Expr_Set,
                        map<Expr *, string> IfCheckInfoList);

  bool is_right_ifCheck(ASTFunction *caller,   // caller of the sensitive function
                                            string Tainted_MemOPName,   // name of sensitive function
                                            string Tainted_MemOP_location,  // location of sensitive function and tainted argument
                                            map<Expr*, string> Tainted_Data_Expr_Set, // set of tainted argument
                                            map<Expr*, string>IfCheckInfoList);

  bool is_right_Check(ASTFunction *caller,   // caller of the sensitive function
                                            string Tainted_MemOPName,   // name of sensitive function
                                            string Tainted_MemOP_location,  // location of sensitive function and tainted argument
											CallExpr *RecordcallExpr,
                                            map<Expr*, string> Tainted_Data_Expr_Set, // set of tainted argument
                                            map<Expr*, string>IfCheckInfoList);

  void collect_missing_check_data(ASTFunction *caller,
                                                 CallExpr *MemOPcallExpr,
                                                 FunctionDecl *callee,
                                                 int label) ;

 // void backForward_DataFlowAnalysis( ASTFunction *caller, string Tainted_MemOP_location, map<Expr *, string> Tainted_Data_Expr_Set);
  void backForward_DataFlowAnalysis( ASTFunction *caller, string Tainted_MemOP_location, map<Expr *, string> Tainted_Data_Expr_Set);

  std::string getMemOPCheckStlye(std::string Tainted_MemOPName);

  bool is_Relevent_String(std::string if_condition_string,
                          std::string Tainted_Data_Expr_string);
  bool is_Relevent_Expr(Expr *if_condition, Expr *Tainted_Data_Expr);
  bool is_Equal_String(std::string if_condition, std::string rightCheckStlye);
  bool is_dangerous_missing_check(ASTFunction *caller, CallExpr *MemOPcallExpr,
                                  FunctionDecl *callee);

  void N_Level_Check(ASTFunction *caller, int N);
  void writingToXML(std::string fileName, std::string funName,
                    std::string sensitiveFunName, std::string taintedArgs,
                    std::string location, std::string description,
                    std::string riskdegree, std::string locLine);
  void writingToCodeReference(string sensitivename, string arr[]);
  void writingToCodeReference(string sensitivename, string arr[], int brr[]);
  void writingToFeatureData(string data_item);
  void AddWarningItemToMap(std::string fileName, std::string funName,
                           std::string sensitiveFunName,
                           std::string taintedArgs, std::string location,
                           std::string description, std::string riskdegree,
                           std::string locLine);

  void getStmtVariables(Expr* ES);
  void readConfig();

  void TwoCheck(ASTFunction *caller);
  void OneCheck(ASTFunction *caller);

  std::string getOpCodeString(int opCode);
  Node *createTree(Expr *expr);
  void getTreePreOrderString(Node *T);
  int Height(Node *T);
  void NormallizeOrder(Node *T);
  bool isSubTree(Node *bigTreeRoot, Node *smallTreeRoot);
  bool isEqualTree(Node *root1, Node *root2);
  string getCodeLine(string location);
  int substringCount(string str, string sub);
  std::string printStmt(Stmt *stmt);

  std::string itos(int n);
};

#endif
