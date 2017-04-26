#include "PreCheckDataSet.h"

namespace CheckDataSet {

#define ARRAYSIZE 5
void PreCheckDataSet::check(ASTFunction *F, FunctionDecl *FD,
                            CFG *function_cfg) {
  clock_t start = clock();
  readConfig();
  if (F != NULL) {
    visitStmt(F, FD, function_cfg);
  }
  run_time += clock() - start;
}

clock_t PreCheckDataSet::get_time() { return run_time; }

PreCheckDataSet::PreCheckDataSet() {
  WarningNum = 0;
  DataNum = 0;
  // funReferenceMap.clear();
}

PreCheckDataSet::~PreCheckDataSet() {
  if (DataNum != 0) {
    std::string xmlFile =
        (*common::configure)["pathToReport"]["path"].asString() +
        "Label_Feature_Data.xml";
    Label_Feature_Data_Doc.save_file(xmlFile.c_str(), "\t",
                                     pugi::format_default |
                                         pugi::format_no_declaration);
  }

  ofstream process_file(
      (*common::configure)["pathToReport"]["path"].asString() + "Train_Function.csv");

  for (map<string, string>::iterator iter = Function_Location.begin();iter != Function_Location.end(); ++iter) 
  {
    string item3 = iter->first + "," + iter->second +"\n";
    process_file << item3 << std::endl;
  }
}

// visit each stmt, and check the stmt if the stmt is a callexprclass
void PreCheckDataSet::visitStmt(ASTFunction *F, FunctionDecl *FD,
                                CFG *function_cfg) {
  // common::printLog( "visit function:----------------"+ F->getName(),
  // common::CheckerName::memoryOPChecker, 5, *common::configure);
  for (CFGBlock *block : *function_cfg) {
    for (auto element : *block) {
      if (element.getKind() == CFGElement::Statement) {
        const Stmt *stmt = element.castAs<CFGStmt>().getStmt();
        if (stmt != nullptr) {
          if (stmt->getStmtClass() == Stmt::CallExprClass) {
            if (const CallExpr *callExpr = dyn_cast<CallExpr>(stmt)) {
              if (callExpr != nullptr) {
                RecordfunDel = const_cast<FunctionDecl *>(FD);
                Recordblock = const_cast<CFGBlock *>(block);
                Recordstmt = const_cast<Stmt *>(stmt);
                RecordcallExpr = const_cast<CallExpr *>(callExpr);
                // common::printLog("visit CallExpr:----------------\n ",
                // common::CheckerName::memoryOPChecker, 5, *common::configure);
                visitCallExpr(RecordcallExpr, F);
              }
            }
          }
        }
      }
    }
  }

  return;
}

// read the informaion from config, including the memory operation funcation
// names, check level, check mode, etc.
void PreCheckDataSet::readConfig() {
  if ((*common::configure)["MemOPCheckLevel"]["N"].empty()) {
    MemOPCheckLevel = 1;
  } else {
    MemOPCheckLevel = (*common::configure)["MemOPCheckLevel"]["N"].asInt();
  }

  if ((*common::configure)["RightIfCheckMode"]["StringMode"].empty()) {
    StringMode = true;

  } else {
    StringMode =
        (*common::configure)["RightIfCheckMode"]["StringMode"].asBool();
  }

  if ((*common::configure)["RightIfCheckMode"]["ASTMode"].empty()) {
    ASTMode = true;

  } else {
    ASTMode = (*common::configure)["RightIfCheckMode"]["ASTMode"].asBool();
  }

  if ((*common::configure)["RightIfCheckMode"]["Z3Mode"].empty()) {
    Z3Mode = true;

  } else {
    Z3Mode = (*common::configure)["RightIfCheckMode"]["Z3Mode"].asBool();
  }

  if ((*common::configure)["SimpleExprCheckOnly"]["MemopSimpleValue"].empty()) {
    MemopSimpleValue = true;
  } else {
    MemopSimpleValue =
        (*common::configure)["SimpleExprCheckOnly"]["MemopSimpleValue"]
            .asBool();
  }

  if ((*common::configure)["SimpleExprCheckOnly"]["ReferableRepair"].empty()) {
    ReferableRepair = false;
  } else {
    ReferableRepair =
        (*common::configure)["SimpleExprCheckOnly"]["ReferableRepair"].asBool();
  }

  for (auto mem : (*common::configure)["MemoryOPFunction"].getMemberNames()) {
    MemoryOPFunction[mem] =
        (*common::configure)["MemoryOPFunction"][mem].asString();
  }
}

// determine whether it is a memory operation function by comparing the
// funcation name

bool PreCheckDataSet::is_MemOP(std::string calleeName) {
  // all the functin is security-sensitive function
  location_of_Arg_str = "all";
  return true;

  std::unordered_map<std::string, std::string>::iterator got =
      MemoryOPFunction.find(calleeName);
  if (got == MemoryOPFunction.end()) {
    return false;
  } else {
    // string args=got->second;
    location_of_Arg_str = got->second;
    // common::printLog(calleeName + location_of_Arg_str +" is a memory
    // operation function!\n", common::CheckerName::memoryOPChecker,
    // 5,*common::configure);
    return true;
  }
}

bool PreCheckDataSet::is_Tainted(Expr *arg) {

   return true;

  if (RecordfunDel != nullptr && Recordblock != nullptr &&
      Recordstmt != nullptr && RecordcallExpr != nullptr && arg != nullptr) {

    if (taint::TaintChecker::is_tainted(RecordfunDel, Recordblock, Recordstmt,
                                        arg)) {
      // common::printLog(getString_of_Expr(arg)+ " is tainted data!\n",
      // common::CheckerName::memoryOPChecker, 5, *common::configure);
      return true;
    } else {
      // common::printLog(getString_of_Expr(arg)+ " is not tainted data!\n",
      // common::CheckerName::memoryOPChecker, 5, *common::configure);
      return false;
    }

  } else {
    return false;
  }
}

std::string PreCheckDataSet::getString_of_Expr(Expr *expr) {
  LangOptions L0;
  L0.CPlusPlus = 1;
  std::string buffer1;
  llvm::raw_string_ostream strout1(buffer1);
  expr->printPretty(strout1, nullptr, PrintingPolicy(L0));
  return strout1.str();
}

bool PreCheckDataSet::has_Tainted_Arg(CallExpr *callExpr,
                                      string location_of_Arg_str) {

  Tainted_Data_Expr_Set.clear();
  int NumofArgs = callExpr->getNumArgs();
  char *str = const_cast<char *>(location_of_Arg_str.c_str());

  if (strcmp(location_of_Arg_str.c_str(), "all") == 0) {
    for (int i = 0; i < NumofArgs; i++) {
      Expr *arg = const_cast<Expr *>(callExpr->getArg(i));

      string argStr = getString_of_Expr(arg);

      /**************not check the complex exprs******
      Expr
      *arg2=callExpr->getArg(i)->IgnoreImpCasts()->IgnoreImpCasts()->IgnoreParenCasts()->IgnoreImpCasts();
      if(MemopSimpleValue==true)
      {
               if(dyn_cast<DeclRefExpr>(arg2)||dyn_cast<MemberExpr>(arg2->IgnoreCasts()))
                      {
                      }
                      else
                      {
                              continue;
                      }
      }
      else if(strstr(argStr.c_str(),"+")||strstr(argStr.c_str(),"-
      ")||strstr(argStr.c_str(),"*")||strstr(argStr.c_str(),"/")||strstr(argStr.c_str(),"%")||strstr(argStr.c_str(),"sizeof")||(strstr(argStr.c_str(),"(")&&
      strstr(argStr.c_str(),")")))
      {
              continue;
      }
      *******************************************/

      if (is_Tainted(arg)) {
        string temp_tainted_data = getString_of_Expr(arg);
        Tainted_Data_Expr_Set.insert(
            pair<Expr *, string>(arg, temp_tainted_data));
      }
    }
  } else if (strstr(location_of_Arg_str.c_str(), "+")) {
    char *delim = "+";
    char *token = NULL;
    token = strtok(str, delim);
    int num = 0;
    for (int i = 0; token != NULL; i++) {
      location_of_Arg[i] = atoi(token);
      num++;
      token = strtok(NULL, delim);
    }

    for (int i = 0; i < num; i++) {
      if (location_of_Arg[i] >= NumofArgs) {
        // common::printLog("error config",
        // common::CheckerName::memoryOPChecker, 5, *common::configure);
        return false;
      }
      Expr *arg = const_cast<Expr *>(callExpr->getArg(location_of_Arg[i]));

      string argStr = getString_of_Expr(arg);

      /*************************do not check the complex exprs*******
      Expr
*arg2=callExpr->getArg(location_of_Arg[i])->IgnoreImpCasts()->IgnoreImpCasts()->IgnoreParenCasts()->IgnoreImpCasts();
      if(MemopSimpleValue==true)
{

if(dyn_cast<DeclRefExpr>(arg2)||dyn_cast<MemberExpr>(arg2->IgnoreCasts()))
{
}
else
              {
                      continue;
}

}
      else if(strstr(argStr.c_str(),"+")||strstr(argStr.c_str(),"-
")||strstr(argStr.c_str(),"*")||strstr(argStr.c_str(),"/")||strstr(argStr.c_str(),"%")||strstr(argStr.c_str(),"sizeof")||(strstr(argStr.c_str(),"(")&&
strstr(argStr.c_str(),")")))
      {
              continue;
      }
      *******************************************************************/

      if (is_Tainted(arg)) {
        string temp_tainted_data = getString_of_Expr(arg);
        Tainted_Data_Expr_Set.insert(
            pair<Expr *, string>(arg, temp_tainted_data));
      }
    }
  } else {
    location_of_Arg[0] = atoi(location_of_Arg_str.c_str());
    if (location_of_Arg[0] >= NumofArgs) {
      // common::printLog("error config", common::CheckerName::memoryOPChecker,
      // 5, *common::configure);
      return false;
    }
    Expr *arg = const_cast<Expr *>(callExpr->getArg(location_of_Arg[0]));

    string argStr = getString_of_Expr(arg);

    /****************************do not check the complex
exprs******************************

    Expr
*arg2=callExpr->getArg(location_of_Arg[0])->IgnoreImpCasts()->IgnoreImpCasts()->IgnoreParenCasts()->IgnoreImpCasts();
    if(MemopSimpleValue==true)
{

if(dyn_cast<DeclRefExpr>(arg2)||dyn_cast<MemberExpr>(arg2->IgnoreCasts()))
{
}
else
            {
                    return false;
}

}
    else if(strstr(argStr.c_str(),"+")||strstr(argStr.c_str(),"-
")||strstr(argStr.c_str(),"*")||strstr(argStr.c_str(),"/")||strstr(argStr.c_str(),"%")||strstr(argStr.c_str(),"sizeof")||(strstr(argStr.c_str(),"(")&&
strstr(argStr.c_str(),")")))
    {
            return false;
    }
    ****************************************************************************************/

    if (is_Tainted(arg)) {
      string temp_tainted_data = getString_of_Expr(arg);
      Tainted_Data_Expr_Set.insert(
          pair<Expr *, string>(arg, temp_tainted_data));
    }
  }

  if (Tainted_Data_Expr_Set.empty()) {
    return false;
  } else {
    return true;
  }
}

// Determine whether there is an if the check
bool PreCheckDataSet::has_ifCheck(ASTFunction *caller) {
  // If_ConditionExpr_Set.clear();
  IfCheckInfoList.clear();
  string callerstr = caller->getName();
  FunctionDecl *functionDecl = common::manager->getFunctionDecl(caller);

  Tainted_Data = "";
  for (unordered_map<Expr *, string>::iterator taint_data_iter =
           Tainted_Data_Expr_Set.begin();
       taint_data_iter != Tainted_Data_Expr_Set.end(); taint_data_iter++) {
    string Tainted_Data_Expr_String = taint_data_iter->second.c_str();
    Tainted_Data = Tainted_Data + Tainted_Data_Expr_String + "  ";
  }

  if (!(functionDecl->hasBody())) {
    // common::printLog(callerstr + "has no ifCheck! \n",
    // common::CheckerName::memoryOPChecker, 5, *common::configure);
    return false;
  } else {
    FunctionDecl *FD = common::manager->getFunctionDecl(caller);
    SourceManager *sm;
    sm = &(FD->getASTContext().getSourceManager());

    auto callerCFG = common::manager->getCFG(caller, FD);
    if (callerCFG != NULL) {
      LangOptions LO;
      LO.CPlusPlus = true;
      for (CFG::iterator iter = callerCFG->begin(); iter != callerCFG->end();
           ++iter) {
        CFGBlock *block = *iter;
        Stmt *it = (block->getTerminator().getStmt());
        if (it != NULL && it->getStmtClass() == Stmt::IfStmtClass) {
          Expr *cond = ((IfStmt *)it)->getCond();
          if (cond != NULL) {
            string cond_location = cond->getLocStart().printToString(*sm);
            // If_ConditionExpr_Set.insert(cond);
            IfCheckInfoList.insert(pair<Expr *, string>(cond, cond_location));
          }
        }
      }

      if (!IfCheckInfoList.empty()) {
        // common::printLog(callerstr + "has ifCheck! \n",
        // common::CheckerName::memoryOPChecker, 5, *common::configure);
        return true;
      } else {
        // common::printLog(callerstr + "has no ifCheck! \n",
        // common::CheckerName::memoryOPChecker, 5, *common::configure);

        return false;
      }
    } else {
      // common::printLog(callerstr + "has no ifCheck! \n",
      // common::CheckerName::memoryOPChecker, 5, *common::configure);

      return false;
    }
  }
}

std::string PreCheckDataSet::getMemOPCheckStlye(std::string Tainted_MemOPName) {
  std::unordered_map<std::string, std::string>::iterator got =
      MemOPCheckStlye.find(Tainted_MemOPName);
  std::string checkStlye;
  if (got == MemOPCheckStlye.end()) {
    checkStlye = "NotFind";
  } else {
    checkStlye = got->second;
  }
  return checkStlye;
}

// determine whethere the tainted_data has been checked rightly in the
// if_condition based on the string mode.
bool PreCheckDataSet::is_Relevent_String(std::string if_condition_string,
                                         std::string Tainted_Data_Expr_string) {
  if (strstr(if_condition_string.c_str(), Tainted_Data_Expr_string.c_str())) {
    return true;
  } else {
    return false;
  }
}

Node *PreCheckDataSet::createTree(Expr *expr) {

  if (expr != NULL) {
    Node *root = new Node;

    while (expr->getStmtClass() == Stmt::ParenExprClass) {
      ParenExpr *parExpr = dyn_cast<ParenExpr>(expr);
      expr = parExpr->getSubExpr();
    }

    if (expr->getStmtClass() == Stmt::BinaryOperatorClass) {
      BinaryOperator *binaryOpExpr = dyn_cast<BinaryOperator>(expr);
      int opCode = binaryOpExpr->getOpcode();
      std::string value = getOpCodeString(opCode);
      root->NodeValue = getOpCodeString(opCode);
      Expr *exprLHS = binaryOpExpr->getLHS();
      Expr *exprRHS = binaryOpExpr->getRHS();

      if (exprLHS != NULL) {
        root->LHS = createTree(exprLHS);
      } else {
        root->LHS = NULL;
      }

      if (exprRHS != NULL) {
        root->RHS = createTree(exprRHS);

      } else {
        root->RHS = NULL;
      }

    } else {
      root->NodeValue = getString_of_Expr(expr);
      root->LHS = NULL;
      root->RHS = NULL;
    }
    return root;

  } else {
    return NULL;
  }
}

std::string PreCheckDataSet::getOpCodeString(int opCode) {
  std::string result = "";
  switch (opCode) {
  case 5: {
    result = "+";
    break;
  }
  case 24: {
    result = "+=";
    break;
  }
  case 15: {
    result = "&";
    break;
  }
  case 28: {
    result = "&=";
    break;
  }
  case 20: {
    result = "=";
    break;
  }
  case 31: {
    result = ",";
    break;
  }
  case 3: {
    result = "/";
    break;
  }
  case 22: {
    result = "/=";
    break;
  }
  case 13: {
    result = "==";
    break;
  }
  case 12: {
    result = ">=";
    break;
  }
  case 10: {
    result = ">";
    break;
  }
  case 18: {
    result = "&&";
    break;
  }
  case 11: {
    result = "<=";
    break;
  }
  case 9: {
    result = "<";
    break;
  }
  case 2: {
    result = "*";
    break;
  }
  case 21: {
    result = "*=";
    break;
  }
  case 14: {
    result = "!=";
    break;
  }
  case 17: {
    result = "||";
    break;
  }
  case 30: {
    result = "|=";
    break;
  }
  case 6: {
    result = "-";
    break;
  }
  case 25: {
    result = "-=";
    break;
  }
  default:
    break;
  }
  return result;
}

void PreCheckDataSet::getTreePreOrderString(Node *T) {
  if (T != NULL) {
    if (PreOrderString == "") {
      PreOrderString = T->NodeValue;
    } else {
      PreOrderString += T->NodeValue;
    }
    getTreePreOrderString(T->LHS);
    getTreePreOrderString(T->RHS);
  }
}

void PreCheckDataSet::NormallizeOrder(Node *T) {
  if (T != NULL) {
    if (T->NodeValue == "+" || T->NodeValue == "*") {
      if (Height(T->LHS) > Height(T->RHS)) {
        Node *temp;
        temp = T->LHS;
        T->LHS = T->RHS;
        T->RHS = temp;
      } else if (Height(T->LHS) == Height(T->RHS)) {
        if (strcmp((T->LHS->NodeValue).c_str(), (T->RHS->NodeValue).c_str()) >
            0) {
          Node *temp;
          temp = T->LHS;
          T->LHS = T->RHS;
          T->RHS = temp;
        }
      }
    }

    NormallizeOrder(T->LHS);
    NormallizeOrder(T->RHS);
  }
}

bool PreCheckDataSet::isSubTree(
    Node *bigTreeRoot,
    Node *smallTreeRoot) // if smallTree is the subtree of bigTree ?
{
  if (smallTreeRoot == NULL) {
    return true;
  }

  if (bigTreeRoot == NULL) {
    return false;
  }

  if (bigTreeRoot->NodeValue == smallTreeRoot->NodeValue) {

    return isSubTree(bigTreeRoot->LHS, smallTreeRoot->LHS) &&
           isSubTree(bigTreeRoot->RHS, smallTreeRoot->RHS);
  } else {

    return isSubTree(bigTreeRoot->LHS, smallTreeRoot) ||
           isSubTree(bigTreeRoot->RHS, smallTreeRoot);
  }
}

bool PreCheckDataSet::isEqualTree(Node *root1, Node *root2) {
  if (root1 == NULL && root2 == NULL) {
    return true;
  } else if (root1 == NULL || root2 == NULL) {
    return false;
  } else {
    return (root1->NodeValue == root2->NodeValue) &&
           isEqualTree(root1->LHS, root2->LHS) &&
           isEqualTree(root1->RHS, root2->RHS);
  }
}

int PreCheckDataSet::Height(Node *T) {
  if (T == NULL)
    return 0;
  else {
    int m = Height(T->LHS);
    int n = Height(T->RHS);
    return (m > n) ? (m + 1) : (n + 1);
  }
}

// determine whether the tainted_data has been checked rightly in the
// if_condition based on the AST mode.
bool PreCheckDataSet::is_Relevent_Expr(Expr *if_condition,
                                       Expr *Tainted_Data_Expr) {
  Tree ifConditonTree;
  ifConditonTree.root = createTree(if_condition);
  NormallizeOrder(ifConditonTree.root);
  Tree taintDataTree;
  taintDataTree.root = createTree(Tainted_Data_Expr);
  NormallizeOrder(taintDataTree.root);
  if (isSubTree(ifConditonTree.root, taintDataTree.root)) {
    return true;
  } else {
    return false;
  }
}

bool PreCheckDataSet::is_Equal_String(std::string if_condition,
                                      std::string rightCheckStlye) {
  if (if_condition == rightCheckStlye) {
    return true;
  } else {
    return false;
  }
}

// determine whether the ifcheck is right if check.
bool PreCheckDataSet::is_right_ifCheck(
    string Tainted_MemOPName,
    unordered_map<Expr *, string> Tainted_Data_Expr_Set,
    unordered_map<Expr *, string> IfCheckInfoList) {
  string SensitiveFunName = Tainted_MemOPName;
  Tainted_Data = "";
  if (StringMode == true) {
    // int N=0;

    // common::printLog("StringMode--------------------\n",
    // common::CheckerName::memoryOPChecker, 5,*common::configure);

    for (unordered_map<Expr *, string>::iterator taint_data_iter =
             Tainted_Data_Expr_Set.begin();
         taint_data_iter != Tainted_Data_Expr_Set.end(); taint_data_iter++) {
      string Tainted_Data_Expr_String = taint_data_iter->second.c_str();

      for (unordered_map<Expr *, string>::iterator ifcon_iterator =
               IfCheckInfoList.begin();
           ifcon_iterator != IfCheckInfoList.end(); ifcon_iterator++) {
        Expr *if_condition = ifcon_iterator->first;

        string ifCondition_String = getString_of_Expr(if_condition).c_str();

        string if_conditon_location = ifcon_iterator->second;
        string ifCheck_Location = getCodeLine(if_conditon_location);

        if (is_Relevent_String(ifCondition_String,
                               Tainted_Data_Expr_String.c_str())) {
          if (std::atoi(ifCheck_Location.c_str()) <
              std::atoi(Tainted_Locline.c_str())) {
            // common::printLog("Right if Check \n",
            // common::CheckerName::memoryOPChecker, 5, *common::configure);
            // collect the code fragment of right ifcheck for funcation call.

            if (ReferableRepair) {
              map<std::string, funReference>::iterator iter =
                  funReferenceMap.find(SensitiveFunName);

              if (iter == funReferenceMap.end()) {
                // no element of named sensitiveFunname
                funReference fs(SensitiveFunName);
                // add value to ifcheckInfo
                ifcheckInfo ifs(ifCondition_String);
                ifs.addloc(ifCheck_Location);
                fs.getList().push_back(ifs);
                funReferenceMap.insert(make_pair(SensitiveFunName, fs));

              } else {
                // has element of named sensitiveFunName
                // ifcheckInfo exist ifConditionan_string?
                funReference fs = iter->second;
                vector<ifcheckInfo> iflist = fs.getList();

                for (vector<ifcheckInfo>::iterator it = iflist.begin();
                     it != iflist.end(); ++it) {
                  if (strcmp((*it).getIfcon().c_str(),
                             ifCondition_String.c_str()) == 0) { // exist
                    (*it).addloc(if_conditon_location);
                    return true;
                  }
                }

                // not exist
                ifcheckInfo ifs(ifCondition_String);
                ifs.addloc(if_conditon_location);
                iter->second.getList().push_back(ifs);
              }
            }
            return true;

          } else {
            continue;
          }
        }

        Tainted_Data = Tainted_Data + Tainted_Data_Expr_String + " ";
      }
    }
  }

  // common::printLog("Wrong if Check \n", common::CheckerName::memoryOPChecker,
  // 5, *common::configure);
  return false;
}

/*
 *
 */
int PreCheckDataSet::substringCount(string str, string sub) {

  int nRet = 0, nStart = 0;
  while (-1 != (nStart = str.find(sub, nStart))) {
    nStart += sub.length();
    ++nRet;
  }
  return nRet;
}

std::string PreCheckDataSet::printStmt(Stmt *stmt) {
  LangOptions L0;
  L0.CPlusPlus = 1;
  std::string buffer1;
  llvm::raw_string_ostream strout1(buffer1);
  stmt->printPretty(strout1, nullptr, PrintingPolicy(L0));
  return "" + strout1.str() + "";
}

void PreCheckDataSet::collect_missing_check_data(ASTFunction *caller,
                                                 CallExpr *MemOPcallExpr,
                                                 FunctionDecl *callee,
                                                 int label) {
  //string callerName=caller->getNameAsString();
  SourceManager *sm;
  FunctionDecl *functionDecl = common::manager->getFunctionDecl(caller);
  string callerName=functionDecl->getName();
  string calleeName=callee->getName();
  sm = &(functionDecl->getASTContext().getSourceManager());

  if (!sm->isLoadedSourceLocation(functionDecl->getLocStart())) {
    return;
  }

  string callerN=callerName+",";
  string calleeN=calleeName+",";

  string location = functionDecl->getLocStart().printToString(*sm);
  string loc=location+",";

  std::map<string, string>::iterator got = Function_Location.find(callerName);
  if (got == Function_Location.end()) 
  {
    Function_Location.insert(pair<string, string>(callerName, location));
  } 

  /*

  ofstream FunNameAndLocation;
  FunNameAndLocation.open("FunctionList.txt", std::ios_base::app);
  FunNameAndLocation << item2.c_str();
  FunNameAndLocation.close();
*/

  FunctionDecl *CallerFD = common::manager->getFunctionDecl(caller);
  FunctionDecl *CalleeFD = callee;
  CFG *CallerCFG = common::manager->getCFG(caller, CallerFD);
  /*********************collect the features*****************************/

  string F0 = std::to_string(label);

  // feature1:numbers of arguments
  int NumOfArg = MemOPcallExpr->getNumArgs();
  string F1 = std::to_string(NumOfArg) + ",";

  // feature2-7: number of +,-,*,/,simplevariable, CompositeVariable in
  // arguments
  int NumOfPlus = 0, NumOfMinus = 0, NumOfMultiply = 0, NumOfDivide = 0,
      NumOfDelively = 0, NumOfSimpleVariable = 0, NumOfCompositeVariable = 0,
      NumOfSizeof = 0, NumOfTaintedArgs = 0;
  for (int i = 0; i < NumOfArg; i++) {
    Expr *arg = const_cast<Expr *>(MemOPcallExpr->getArg(i));
    if (is_Tainted(arg)) {
      NumOfTaintedArgs += 1;
    }
    string argStr = getString_of_Expr(arg);
    NumOfPlus += substringCount(argStr, "+");
    NumOfMinus += substringCount(argStr, "-");
    NumOfMultiply += substringCount(argStr, "*");
    NumOfDivide += substringCount(argStr, "/");
    NumOfDelively += substringCount(argStr, "%");
    NumOfSizeof += substringCount(argStr, "sizeof");
    Expr *arg2 = MemOPcallExpr->getArg(i)
                     ->IgnoreImpCasts()
                     ->IgnoreImpCasts()
                     ->IgnoreParenCasts()
                     ->IgnoreImpCasts();
    if (dyn_cast<DeclRefExpr>(arg2) ||
        dyn_cast<MemberExpr>(arg2->IgnoreCasts())) {
      NumOfSimpleVariable += 1;
    } else {
      NumOfCompositeVariable += 1;
    }
  }

  // feature2: number of +
  string F2 = std::to_string(NumOfPlus) + ",";

  // feature3: number of -
  string F3 =  std::to_string(NumOfMinus) + ",";

  // feature4: number of *
  string F4 = std::to_string(NumOfMultiply) + ",";

  // feature5: number of /
  string F5 =  std::to_string(NumOfDivide) + ",";

  // feature6: number of %
  string F6 =  std::to_string(NumOfDelively) + ",";

  // feature7: number of simple variable
  string F7 =  std::to_string(NumOfSimpleVariable) + ",";

  // feature8: number of composite variable
  string F8 = std::to_string(NumOfCompositeVariable) + ",";

  // feature9: number of sizeof
  string F9 =  std::to_string(NumOfSizeof) + ",";

  // feature10: numbers of variables in the body of caller
  int NumOfCallerVariables = common::getVariables(CallerFD).size();
  string F10 = std::to_string(NumOfCallerVariables) + ",";

  // feature11: numbers of callexprs in the body of caller
  int NumOfCallerExprs = common::getCallExpr(CallerFD).size();
  string F11 = std::to_string(NumOfCallerExprs) + ",";

  // feature12: if callee has body
  int CalleeHasBody = 0;
  // int CalleeHasIfCheck=0;
  if (CalleeFD->hasBody()) {
    CalleeHasBody = 1;
  }
  string F12 = std::to_string(CalleeHasBody) + ",";

  // feature13: argument occur in the left or right head of = in front code
  int NumOfOccurInBinoryOP = 0;
  int NumOfBinoryOP = 0;
  for (CFGBlock *block : *CallerCFG) {
    for (auto element : *block) {
      if (element.getKind() == CFGElement::Statement) {
        const Stmt *stmt = element.castAs<CFGStmt>().getStmt();
        if (stmt != nullptr &&
            stmt->getStmtClass() == Stmt::BinaryOperatorClass) {
          const BinaryOperator *it = dyn_cast<BinaryOperator>(stmt);
          if (it->getOpcode() == clang::BinaryOperatorKind::BO_EQ) {
            NumOfBinoryOP += 1;
            Expr *lhs = it->getLHS()->IgnoreImpCasts()->IgnoreParenCasts();
            Expr *rhs = it->getRHS()->IgnoreImpCasts()->IgnoreParenCasts();
            string leftExprStr = printStmt(lhs);
            string rightExprStr = printStmt(rhs);

            for (int i = 0; i < NumOfArg; i++) {
              Expr *arg = const_cast<Expr *>(MemOPcallExpr->getArg(i));
              string argStr = getString_of_Expr(arg);
              if (strcmp(leftExprStr.c_str(), argStr.c_str()) == 0) {
                NumOfOccurInBinoryOP += 1;
              }
            }
          }
        }
      }
    }
  }

  string F13 = std::to_string(NumOfOccurInBinoryOP) + ",";

  // feature14: numbers of binoryop "="
  string F14 =  std::to_string(NumOfBinoryOP) + ",";

  // feature15: numbers of taint arguments
  string F15 =  std::to_string(NumOfTaintedArgs) + ",";

  // feature8: code lines of caller function
  // feature10: if callee's body contain a ifcheck in the definition of callee
  /*********************vector data *************************************/

  string item = callerN + loc + F1 + F2 + F3 + F4 + F5 + F6 + F7 + F8 + F9 + F10 + F11 +
                F12 + F13 + F14 + F15 + F0+ "\n";

  writingToFeatureData(item);

  ofstream myfile;
  myfile.open("Train_Feature_Data_NoTaint.csv", std::ios_base::app);
  myfile << item.c_str();
  myfile.close();

  /*********************Search the libsvm model*************************/
}

void PreCheckDataSet::OneCheck(ASTFunction *caller) {

  string CallStack = "";
  if (!has_ifCheck(caller)) {

    collect_missing_check_data(caller, RecordcallExpr, RecordCallee, 1);

  }

  else if (!is_right_ifCheck(Tainted_MemOPName, Tainted_Data_Expr_Set,
                             IfCheckInfoList)) {

    collect_missing_check_data(caller, RecordcallExpr, RecordCallee, 1);

  } 
   //else {
    //collect_missing_check_data(caller, RecordcallExpr, RecordCallee, 0);
  //}
}

void PreCheckDataSet::TwoCheck(ASTFunction *caller) {
  // string
  // refer=CheckDataSet::PreCheckDataSet::SearchRefeCheck(Tainted_MemOPName);
  string CallStack = "";

  if (!has_ifCheck(caller)) {

    std::vector<ASTFunction *> ParentCallers =
        common::call_graph->getParents(caller);
    if (ParentCallers.size() != 0) {
      for (ASTFunction *Pcaller : ParentCallers) {
        Caller2 = Pcaller->getName();
        if (!has_ifCheck(Pcaller)) {
          collect_missing_check_data(caller, RecordcallExpr, RecordCallee, 1);
          CallStack = CallStack + Caller2 + "; " + Tainted_FunctionName + "; " +
                      Tainted_MemOPName + "\n";
          Tainted_Description =
              "\n[" + Tainted_MemOPName + "]" +
              " is a memory operation function using tainted data: [" +
              Tainted_Data + "]\n" + "Location : [" + Tainted_MemOP_location +
              "]\n" + "Call Stack:\n" + CallStack;
          writingToXML(Tainted_FileName, Tainted_FunctionName,
                       Tainted_Description, Tainted_Locline);
          return;
        }

        else if (!is_right_ifCheck(Tainted_MemOPName, Tainted_Data_Expr_Set,
                                   IfCheckInfoList)) {
          collect_missing_check_data(caller, RecordcallExpr, RecordCallee, 1);
          CallStack = CallStack + Caller2 + "; " + Tainted_FunctionName + "; " +
                      Tainted_MemOPName + "\n";
          Tainted_Description =
              "\n[" + Tainted_MemOPName + "]" +
              " is a memory operation function using tainted data: [" +
              Tainted_Data + "]\n" + "Location : [" + Tainted_MemOP_location +
              "]\n" + "Call Stack:\n" + CallStack;
          writingToXML(Tainted_FileName, Tainted_FunctionName,
                       Tainted_Description, Tainted_Locline);
          return;

        } else {
          collect_missing_check_data(caller, RecordcallExpr, RecordCallee, 0);
        }
      }

    } else {
      collect_missing_check_data(caller, RecordcallExpr, RecordCallee, 1);
      // common::printLog(Tainted_FunctionName + "has no parent callers",
      // common::CheckerName::memoryOPChecker, 5, *common::configure);
      CallStack = CallStack + Caller2 + "; " + Tainted_FunctionName + "; " +
                  Tainted_MemOPName + "\n";
      Tainted_Description =
          "\n[" + Tainted_MemOPName + "]" +
          " is a memory operation function using tainted data: [" +
          Tainted_Data + "]\n" + "Location : [" + Tainted_MemOP_location +
          "]\n" + "Call Stack:\n" + CallStack;
      writingToXML(Tainted_FileName, Tainted_FunctionName, Tainted_Description,
                   Tainted_Locline);
      return;
    }

  } else if (!is_right_ifCheck(Tainted_MemOPName, Tainted_Data_Expr_Set,
                               IfCheckInfoList)) {
    std::vector<ASTFunction *> ParentCallers =
        common::call_graph->getParents(caller);
    if (ParentCallers.size() != 0) {
      for (ASTFunction *Pcaller : ParentCallers) {
        // FunctionDecl* fNode = manager->getFunctionDecl(Pcaller);
        Caller2 = Pcaller->getName();
        if (!has_ifCheck(Pcaller)) {
          collect_missing_check_data(caller, RecordcallExpr, RecordCallee, 1);
          CallStack = CallStack + Caller2 + "; " + Tainted_FunctionName + "; " +
                      Tainted_MemOPName + "\n";
          Tainted_Description =
              "\n[" + Tainted_MemOPName + "]" +
              " is a memory operation function using tainted data: [" +
              Tainted_Data + "]\n" + "Location : [" + Tainted_MemOP_location +
              "]\n" + "Call Stack:\n" + CallStack;
          writingToXML(Tainted_FileName, Tainted_FunctionName,
                       Tainted_Description, Tainted_Locline);
          return;
        } else if (!is_right_ifCheck(Tainted_MemOPName, Tainted_Data_Expr_Set,
                                     IfCheckInfoList)) {
          collect_missing_check_data(caller, RecordcallExpr, RecordCallee, 1);
          CallStack = CallStack + Caller2 + "; " + Tainted_FunctionName + "; " +
                      Tainted_MemOPName + "\n";
          Tainted_Description =
              "\n[" + Tainted_MemOPName + "]" +
              " is a memory operation function using tainted data: [" +
              Tainted_Data + "]\n" + "Location : [" + Tainted_MemOP_location +
              "]\n" + "Call Stack:\n" + CallStack;
          writingToXML(Tainted_FileName, Tainted_FunctionName,
                       Tainted_Description, Tainted_Locline);
          return;
        } else {
          collect_missing_check_data(caller, RecordcallExpr, RecordCallee, 0);
        }
      }
    } else {
      collect_missing_check_data(caller, RecordcallExpr, RecordCallee, 1);
      // common::printLog(Tainted_FunctionName + "has no parent callers",
      // common::CheckerName::memoryOPChecker, 5,*common::configure);
      CallStack = CallStack + Caller2 + "; " + Tainted_FunctionName + "; " +
                  Tainted_MemOPName + "\n";
      Tainted_Description =
          "\n[" + Tainted_MemOPName + "]" +
          " is a memory operation function using tainted data: [" +
          Tainted_Data + "]\n" + "Location : [" + Tainted_MemOP_location +
          "]\n" + "Call Stack:\n" + CallStack;
      writingToXML(Tainted_FileName, Tainted_FunctionName, Tainted_Description,
                   Tainted_Locline);
      return;
    }

  } else {
    return;
  }
}

// N level check the existness and correctness of ifcheck for memory operation
// funcation
void PreCheckDataSet::N_Level_Check(ASTFunction *caller, int N) {

  if (caller == NULL) {
    return;
  }

  if (N < 0 || N > 1) {
    N = 1;
  }

  if (N == 1) {
    TwoCheck(caller);
  }

  if (N == 0) {
    OneCheck(caller);
  }
}

string replace_all(string str, const string old_value, const string new_value) {
  while (true) {
    string::size_type pos(0);
    if ((pos = str.find(old_value)) != string::npos)
      str.replace(pos, old_value.length(), new_value);
    else
      break;
  }
  return str;
}

// Visit each callexpr, determine whether it's a memory operation funcation, if
// it is, determine whether there is a ifcheck and validate the correctness of
// ifcheck.
void PreCheckDataSet::visitCallExpr(CallExpr *callExpr, ASTFunction *caller) {
  Tainted_Locline = "";
  Tainted_MemOPName = "";
  Tainted_FunctionName = "";

  std::string callerName;
  FunctionDecl *functionDecl = common::manager->getFunctionDecl(caller);
  if (caller) {
    callerName = caller->getName();
    // common::printLog("caller: " + callerName +"\n",
    // common::CheckerName::memoryOPChecker, 5, *common::configure);
  }

  std::string calleeName;
  if (callExpr->getDirectCallee() != NULL) {
    const FunctionDecl *callee = callExpr->getDirectCallee();
    RecordCallee = const_cast<FunctionDecl *>(callee);
    calleeName = callee->getNameAsString();
    // common::printLog("callee: " + calleeName +"\n",
    // common::CheckerName::memoryOPChecker, 5, *common::configure);
  } else {
    return;
  }

  SourceManager *sm;
  sm = &(functionDecl->getASTContext().getSourceManager());

  if (!sm->isLoadedSourceLocation(callExpr->getLocStart())) {
    // common::printLog("unloaded callExpr: " +
    // callExpr->getDirectCallee()->getNameAsString() + "\n",
    // common::CheckerName::memoryOPChecker, 5, *common::configure);
    return;
  }

  Tainted_MemOP_location = callExpr->getLocStart().printToString(*sm);

  {
    // record the location of MemOP
    std::string tempLoc = Tainted_MemOP_location;
    std::string LineNoRow, Line;
    int pos1 = tempLoc.find_last_of('<');
    if (pos1 == -1) {
      pos1 = tempLoc.find_last_of(':');
    } else {
      tempLoc.assign(tempLoc.c_str(), pos1);
      pos1 = tempLoc.find_last_of(':');
    }
    LineNoRow.assign(tempLoc.c_str(), pos1);
    int pos2 = LineNoRow.find_last_of(':');
    Tainted_FileName.assign(LineNoRow.c_str(), pos2);
    Tainted_FileName = replace_all(Tainted_FileName, "\\", "/");
    Tainted_MemOP_location = replace_all(Tainted_MemOP_location, "\\", "/");
    Line = LineNoRow.substr(pos2 + 1);

    Tainted_Locline = Line;
    Tainted_MemOPName = calleeName;
    Tainted_FunctionName = callerName;
    Caller2 = "";
    Caller3 = "";
    N_Level_Check(caller, MemOPCheckLevel);
  }
}

string PreCheckDataSet::getCodeLine(string location) {
  std::string tempLoc = location;
  std::string LineNoRow, Line;
  int pos1 = tempLoc.find_last_of('<');
  if (pos1 == -1) {
    pos1 = tempLoc.find_last_of(':');
  } else {
    tempLoc.assign(tempLoc.c_str(), pos1);
    pos1 = tempLoc.find_last_of(':');
  }
  LineNoRow.assign(tempLoc.c_str(), pos1);
  int pos2 = LineNoRow.find_last_of(':');

  location = replace_all(location, "\\", "/");
  Line = LineNoRow.substr(pos2 + 1);

  return Line;
}

// write the warning information into the XML file.
void PreCheckDataSet::writingToXML(std::string fileName, std::string funName,
                                   std::string descr, std::string locLine) {
  WarningNum++;

  pugi::xml_node node = doc.append_child("error");

  pugi::xml_node checker = node.append_child("checker");
  checker.append_child(pugi::node_pcdata).set_value("MOLINT.TAINTED.MEM.OP");

  pugi::xml_node domain = node.append_child("domain");
  domain.append_child(pugi::node_pcdata).set_value("STATIC_C");

  pugi::xml_node file = node.append_child("file");
  file.append_child(pugi::node_pcdata).set_value(fileName.c_str());

  pugi::xml_node function = node.append_child("function");
  function.append_child(pugi::node_pcdata).set_value(funName.c_str());

  pugi::xml_node score = node.append_child("score");
  score.append_child(pugi::node_pcdata).set_value("100");

  pugi::xml_node ordered = node.append_child("ordered");
  ordered.append_child(pugi::node_pcdata).set_value("false");

  pugi::xml_node event = node.append_child("event");

  pugi::xml_node main = event.append_child("main");
  main.append_child(pugi::node_pcdata).set_value("true");

  pugi::xml_node tag = event.append_child("tag");
  tag.append_child(pugi::node_pcdata).set_value("Error");

  pugi::xml_node description = event.append_child("description");
  description.append_child(pugi::node_pcdata).set_value(descr.c_str());

  pugi::xml_node line = event.append_child("line");
  line.append_child(pugi::node_pcdata).set_value(locLine.c_str());

  pugi::xml_node extra = node.append_child("extra");
  extra.append_child(pugi::node_pcdata).set_value("none");

  pugi::xml_node subcategory = node.append_child("subcategory");
  subcategory.append_child(pugi::node_pcdata).set_value("none");
}

// write the warning information into the XML file.
void PreCheckDataSet::writingToFeatureData(string data_item) {
  DataNum++;

  // string link=fileName+":"+startLocation;
  pugi::xml_node node = Label_Feature_Data_Doc.append_child("item");
  node.append_child(pugi::node_pcdata).set_value(data_item.c_str());

  // pugi::xml_node item = node.append_child("item");
  // item.append_child(pugi::node_pcdata).set_value(data_item.c_str());
}
}
