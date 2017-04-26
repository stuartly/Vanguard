#include "TaintedMemOPChecker.h"

using namespace std;

#define ARRAYSIZE 5

/// entry function for missing check
void TaintedMemOPChecker::check(ASTFunction *F, FunctionDecl *FD,
                                CFG *function_cfg) {
  clock_t start = clock();
  readConfig();
  if (F != NULL) {
    visitStmt(F, FD, function_cfg);
  }
  run_time += clock() - start;
}

TaintedMemOPChecker::TaintedMemOPChecker() {
  WarningNum = 0;
  CheckSampleNum = 0;
  DataNum = 0;
  funReferenceMap.clear();
}

TaintedMemOPChecker::~TaintedMemOPChecker() {
  ofstream process_file(
      (*common::configure)["pathToReport"]["path"].asString() + "Predicate_Function.csv");

  for (map<string, string>::iterator iter = Function_Location.begin();iter != Function_Location.end(); ++iter) 
  {
    string item3 = iter->first + "," + iter->second +"\n";
    process_file << item3 << std::endl;
  }

  if (WarningNum != 0) {
    for (map<string, WarningItem>::iterator iter = WarningList.begin();
         iter != WarningList.end(); ++iter) {
      WarningItem item = iter->second;
      writingToXML(item.getFileName(), item.getCallerFunction(),
                   item.getSensitiveFunction(), item.getTaintedArguments(),
                   item.getLocation(), item.getDescription(),
                   item.getRiskDegree(), item.getLine());
    }

    std::string xmlFile =
        (*common::configure)["pathToReport"]["path"].asString() +
        "MOLINT.TAINTED.MEM.OP.xml";
    doc.save_file(xmlFile.c_str(), "\t",
                  pugi::format_default | pugi::format_no_declaration);
  }

  clock_t repair_time_send = clock();
  if (ReferableRepair) {

    if (!funReferenceMap.empty()) {
      string arr[ARRAYSIZE] = {};
      int brr[ARRAYSIZE] = {};
      for (map<std::string, funReference>::iterator iter =
               funReferenceMap.begin();
           iter != funReferenceMap.end(); ++iter) {

        string sensitive_fun_name = iter->first;
        vector<ifcheckInfo> ifvector = (iter->second).getList();
        // top 5 sort from larger to small
        std::sort(ifvector.begin(), ifvector.end());
        for (unsigned int i = 0; i < ifvector.size(); i++) {
          if (ifvector[i].getIfcon() != "" && i < ARRAYSIZE) {
            arr[i] = ifvector[i].getIfcon();
            brr[i] = ifvector[i].getCount();
          }
        }

        writingToCodeReference(sensitive_fun_name, arr, brr);
      }
    }

    if (CheckSampleNum != 0) {
      std::string CheckSamleXML =
          (*common::configure)["pathToReport"]["path"].asString() +
          "REFERABLE.REPAIR.MISSING.CHECK.xml";
      checkSampleDoc.save_file(CheckSamleXML.c_str(), "\t",
                               pugi::format_default |
                                   pugi::format_no_declaration);
    }
  }
  repair_time += float(clock() - repair_time_send);
}

// visit each stmt, and check the stmt if the stmt is a callexprclass
void TaintedMemOPChecker::visitStmt(ASTFunction *F, FunctionDecl *FD,
                                    CFG *function_cfg) {
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
void TaintedMemOPChecker::readConfig() {
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

  if ((*common::configure)["RightIfCheckMode"]["WithTaint"].empty()) {
    WithTaint = true;

  } else {
    WithTaint = (*common::configure)["RightIfCheckMode"]["WithTaint"].asBool();
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
bool TaintedMemOPChecker::is_MemOP(std::string calleeName) {
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

bool TaintedMemOPChecker::is_Tainted(Expr *arg) {

  if(WithTaint==false)
  {
    return true;
  }

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

std::string TaintedMemOPChecker::getString_of_Expr(Expr *expr) {
  LangOptions L0;
  L0.CPlusPlus = 1;
  std::string buffer1;
  llvm::raw_string_ostream strout1(buffer1);
  expr->printPretty(strout1, nullptr, PrintingPolicy(L0));
  return strout1.str();
}

/// input: call expression and the location of the argument
/// output: check if the argument is tainted. If it is tainted, it will be
/// stored in Tainted_Data_Expr_Set
bool TaintedMemOPChecker::has_Tainted_Arg(CallExpr *callExpr,
                                          string location_of_Arg_str) {

  Tainted_Data_Expr_Set.clear();
  int NumofArgs = callExpr->getNumArgs();
  char *str = const_cast<char *>(location_of_Arg_str.c_str());

  if (strcmp(location_of_Arg_str.c_str(), "all") == 0) {
    for (int i = 0; i < NumofArgs; i++) {
      Expr *arg = const_cast<Expr *>(callExpr->getArg(i));

      string argStr = getString_of_Expr(arg);

      //**************not check the complex exprs******
      Expr *arg2 = callExpr->getArg(i)
                       ->IgnoreImpCasts()
                       ->IgnoreImpCasts()
                       ->IgnoreParenCasts()
                       ->IgnoreImpCasts();
      if (MemopSimpleValue == true) {
        if (dyn_cast<DeclRefExpr>(arg2) ||
            dyn_cast<MemberExpr>(arg2->IgnoreCasts())) {
        } else {
          continue;
        }
      }
      /*
      else if(strstr(argStr.c_str(),"+")||strstr(argStr.c_str(),"-
      ")||strstr(argStr.c_str(),"*")||strstr(argStr.c_str(),"/")||strstr(argStr.c_str(),"%")||strstr(argStr.c_str(),"sizeof")||(strstr(argStr.c_str(),"(")&&
      strstr(argStr.c_str(),")")))
      {
              continue;
      }
      //*******************************************/

      if (is_Tainted(arg)) {
        string temp_tainted_data = getString_of_Expr(arg);
        Tainted_Data_Expr_Set.insert(
            pair<Expr *, string>(arg, temp_tainted_data));
      }
    }
  }

  else if (strstr(location_of_Arg_str.c_str(), "+")) {
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
        common::printLog("error config", common::CheckerName::memoryOPChecker,
                         5, *common::configure);
        return false;
      }
      Expr *arg = const_cast<Expr *>(callExpr->getArg(location_of_Arg[i]));

      string argStr = getString_of_Expr(arg);

      //*************************do not check the complex exprs*******
      Expr *arg2 = callExpr->getArg(location_of_Arg[i])
                       ->IgnoreImpCasts()
                       ->IgnoreImpCasts()
                       ->IgnoreParenCasts()
                       ->IgnoreImpCasts();
      if (MemopSimpleValue == true) {

        if (dyn_cast<DeclRefExpr>(arg2) ||
            dyn_cast<MemberExpr>(arg2->IgnoreCasts())) {
        } else {
          continue;
        }
      }
      /*
      else if(strstr(argStr.c_str(),"+")||strstr(argStr.c_str(),"-
      ")||strstr(argStr.c_str(),"*")||strstr(argStr.c_str(),"/")||strstr(argStr.c_str(),"%")||strstr(argStr.c_str(),"sizeof")||(strstr(argStr.c_str(),"(")&&
      strstr(argStr.c_str(),")")))
      {
              continue;
      }
      /*******************************************************************/

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

    //****************************do not check the complex
    //exprs******************************

    Expr *arg2 = callExpr->getArg(location_of_Arg[0])
                     ->IgnoreImpCasts()
                     ->IgnoreImpCasts()
                     ->IgnoreParenCasts()
                     ->IgnoreImpCasts();
    if (MemopSimpleValue == true) {

      if (dyn_cast<DeclRefExpr>(arg2) ||
          dyn_cast<MemberExpr>(arg2->IgnoreCasts())) {
      } else {
        return false;
      }
    }
    /*
    else if(strstr(argStr.c_str(),"+")||strstr(argStr.c_str(),"-
    ")||strstr(argStr.c_str(),"*")||strstr(argStr.c_str(),"/")||strstr(argStr.c_str(),"%")||strstr(argStr.c_str(),"sizeof")||(strstr(argStr.c_str(),"(")&&
    strstr(argStr.c_str(),")")))
    {
            return false;
    }
    //****************************************************************************************/

    if (is_Tainted(arg)) {
      string temp_tainted_data = getString_of_Expr(arg);
      Tainted_Data_Expr_Set.insert(
          pair<Expr *, string>(arg, temp_tainted_data));
    }
  }

  if (Tainted_Data_Expr_Set.empty()) {
    return false;
  } else {

    Tainted_Data = "";
    for (map<Expr *, string>::iterator taint_data_iter =
             Tainted_Data_Expr_Set.begin();
         taint_data_iter != Tainted_Data_Expr_Set.end(); taint_data_iter++) {
      string Tainted_Data_Expr_String = taint_data_iter->second.c_str();
      Tainted_Data = Tainted_Data + Tainted_Data_Expr_String + "  ";
    }
    return true;
  }
}



// Determine whether there is an if the check
bool TaintedMemOPChecker::has_ifCheck(ASTFunction *caller) 
{
  // If_ConditionExpr_Set.clear();
  IfCheckInfoList.clear();
  string callerstr = caller->getName();
  FunctionDecl *functionDecl = common::manager->getFunctionDecl(caller);

  if (!(functionDecl->hasBody())) 
  {
    common::printLog(callerstr + "has no ifCheck! \n",common::CheckerName::memoryOPChecker, 5, *common::configure);
    return false;
  } 
  else 
  {
    FunctionDecl *FD = common::manager->getFunctionDecl(caller);
    SourceManager *sm;
    sm = &(FD->getASTContext().getSourceManager());

    auto callerCFG = common::manager->getCFG(caller, FD);

    if (callerCFG != NULL) 
    {

		 for(CFG::iterator iter=callerCFG->begin();iter!=callerCFG->end();++iter)
		{
					CFGBlock* block=*iter;					
					Stmt* it=(block->getTerminator().getStmt());
					Expr * cond=NULL;
					if(it!=NULL&&it->getStmtClass()==Stmt::IfStmtClass)
					{
						cond=((IfStmt*)it)->getCond();
						if(cond !=NULL)
						{
							string cond_location=cond->getLocStart().printToString(*sm);
							//If_ConditionExpr_Set.insert(cond);	
							IfCheckInfoList.insert(pair<Expr*,string>(cond,cond_location));
						}						
					}

					if (it!=NULL&&it->getStmtClass() == Stmt::ForStmtClass) 
			        {
			          	cond = ((ForStmt *)it)->getCond();
			          	if (cond != NULL) 
			          	{
			            	string for_cond_location = cond->getLocStart().printToString(*sm);
			            	IfCheckInfoList.insert(pair<Expr *, string>(cond, for_cond_location));
			          	}
			        }

			        if (it!=NULL&&it->getStmtClass() == Stmt::WhileStmtClass) 
			        {
			          	cond = ((WhileStmt *)it)->getCond();
			          	if (cond != NULL) 
			          	{
			            	string while_cond_location = cond->getLocStart().printToString(*sm);
			            	IfCheckInfoList.insert(pair<Expr *, string>(cond, while_cond_location));
			          	}
			        }

			        if (it!=NULL&&it->getStmtClass() == Stmt::SwitchStmtClass) 
			       {
			          	cond = ((SwitchStmt *)it)->getCond();
			          	if (cond != NULL) 
			          	{
			            	string switch_cond_location = cond->getLocStart().printToString(*sm);
			            	IfCheckInfoList.insert(pair<Expr *, string>(cond, switch_cond_location));
			          	}
			        }
					/*

			        if (it!=NULL&&it->getStmtClass() == Stmt::DoStmtClass) 
			        {
			          	cond = ((DoStmt *)it)->getCond();
			          	if (cond != NULL) 
			          	{
			            	string do_cond_location = cond->getLocStart().printToString(*sm);
			            	IfCheckInfoList.insert(pair<Expr *, string>(cond, do_cond_location));
			          	}
			        }
					*/

		 }


		      if (!IfCheckInfoList.empty()) 
		      {
		        return true;
		      } 
		      else 
		      {
		        return false;
		      }

  	 }
    else
	{
	    	return false;
	}
 }
}

std::string
TaintedMemOPChecker::getMemOPCheckStlye(std::string Tainted_MemOPName) {
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
bool TaintedMemOPChecker::is_Relevent_String(
    std::string if_condition_string, std::string Tainted_Data_Expr_string) {
  if (strstr(if_condition_string.c_str(), Tainted_Data_Expr_string.c_str())) {
    return true;
  } else {
    return false;
  }
}

Node *TaintedMemOPChecker::createTree(Expr *expr) {

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

std::string TaintedMemOPChecker::getOpCodeString(int opCode) {
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

void TaintedMemOPChecker::getTreePreOrderString(Node *T) {
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

void TaintedMemOPChecker::NormallizeOrder(Node *T) {
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

bool TaintedMemOPChecker::isSubTree(
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

bool TaintedMemOPChecker::isEqualTree(Node *root1, Node *root2) {
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

int TaintedMemOPChecker::Height(Node *T) {
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
bool TaintedMemOPChecker::is_Relevent_Expr(Expr *if_condition,
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

bool TaintedMemOPChecker::is_Equal_String(std::string if_condition,
                                          std::string rightCheckStlye) {
  if (if_condition == rightCheckStlye) {
    return true;
  } else {
    return false;
  }
}

void TaintedMemOPChecker::getStmtVariables(Expr* ES)
{
	Expr*S = ES ->IgnoreImpCasts()
                       ->IgnoreImpCasts()
                       ->IgnoreParenCasts()
                       ->IgnoreImpCasts();
	
  if (DeclRefExpr *DRE = dyn_cast<DeclRefExpr>(S)) {
	 variableList.push_back(S);	
	 common::printLog("VAR:"+getString_of_Expr(S)+"\n", common::CheckerName::memoryOPChecker, 5, *common::configure);
  }  

  if (S->getStmtClass() == Stmt::IntegerLiteralClass ||
      S->getStmtClass() == Stmt::FloatingLiteralClass) {
		  variableList.push_back(S);
		  common::printLog("VAR:"+getString_of_Expr(S)+"\n", common::CheckerName::memoryOPChecker, 5, *common::configure);
  }

  // sizeof
  if (UnaryExprOrTypeTraitExpr * E=dyn_cast<UnaryExprOrTypeTraitExpr>(S)) {
     getStmtVariables(E->getArgumentExpr());
  }

  if (ParenExpr *E = dyn_cast<ParenExpr>(S)) {
     getStmtVariables(E->getSubExpr());
  }

  if (CallExpr *E = dyn_cast<CallExpr>(S)) {

	  if (CXXMemberCallExpr *CXXCE = dyn_cast<CXXMemberCallExpr>(E)) {
        getStmtVariables(CXXCE->getImplicitObjectArgument());
      }

	  for (auto arg : E->arguments()) {
		getStmtVariables(arg);
	  }
  }

  if (BinaryOperator *E = dyn_cast<BinaryOperator>(S)) {
	getStmtVariables(E->getRHS());
    getStmtVariables(E->getLHS());
  }

  if (UnaryOperator *E = dyn_cast<UnaryOperator>(S)) {
     getStmtVariables(E->getSubExpr());
  }

  if (CastExpr *E = dyn_cast<CastExpr>(S)) {
     getStmtVariables(E->getSubExpr());
  }

  if (MemberExpr *E = dyn_cast<MemberExpr>(S)) {
     getStmtVariables(E->getBase());
  }

  if (ArraySubscriptExpr *E = dyn_cast<ArraySubscriptExpr>(S)) {
     getStmtVariables(E->getBase());
  }

  if (ConditionalOperator *E = dyn_cast<ConditionalOperator>(S)) {
     getStmtVariables(E->getTrueExpr());
	 getStmtVariables(E->getFalseExpr());
  }

  if (CXXNewExpr *E = dyn_cast<CXXNewExpr>(S)) {
     getStmtVariables(const_cast<CXXConstructExpr *>(E->getConstructExpr()));
  }

  if (CXXConstructExpr *E = dyn_cast<CXXConstructExpr>(S)) {
	  for (auto arg : E->arguments()) {
		getStmtVariables(arg);
	  }
  }

 

  if (ReturnStmt *E = dyn_cast<ReturnStmt>(S)) {
    if (E->getRetValue() != nullptr) {
      getStmtVariables(E->getRetValue());
    }
  }

  /*
  if (DeclStmt *DS = dyn_cast<DeclStmt>(S)) {
    for (Decl *D : DS->decls()) {
      VarDecl *VD = dyn_cast<VarDecl>(D);
      if (VD == nullptr || !(VD->hasInit()))
        continue;
	  variableList.push_back(VD);
	  getStmtVariables(VD->getInit());
	}
  }
  */




}

// TODO:: overwrite the is_right_ifCheck function to judge the relation of
// tainted argument and check condition.
bool  TaintedMemOPChecker::is_right_Check(ASTFunction *caller,   // caller of the sensitive function
	                                         string Tainted_MemOPName,   // name of sensitive function
	                                         string Tainted_MemOP_location,  // location of sensitive function and tainted argument
											 CallExpr *RecordcallExpr,
	                                         map<Expr*, string> Tainted_Data_Expr_Set, // set of tainted argument
	                                         map<Expr*, string>IfCheckInfoList)  // map of check condtion of ifstmt, whileStmt, forStmt, SwitchStmt
{
	//common::printLog("Begin is_right_Check -------------------- \n", common::CheckerName::memoryOPChecker, 5, *common::configure);
	// get the variables of  sensitive callexpr argument
	Environment env(caller,false);
	//reset the taint situation of env
	for (auto v: env.values)
	{
		v.update(TaintValue(TaintKind::Untainted));	
	}

	//common::printLog("Reset environemnt -------------------- \n", common::CheckerName::memoryOPChecker, 5, *common::configure);
    FunctionDecl *FD = common::manager->getFunctionDecl(caller);
    auto callerCFG = common::manager->getCFG(caller, FD);

	Environment *penv=&env;

	bool Flag=false;
	for(map<Expr*, string>::iterator iter= IfCheckInfoList.begin();iter != IfCheckInfoList.end();iter++)
	{
		getStmtVariables(iter->first);
		if (callerCFG != NULL) 
		{
			for(CFGBlock* block:*callerCFG)
			{
				if(!Flag) 
						{
							if(block!=NULL)
							{
								Stmt* it=(block->getTerminator().getStmt());
								Expr* cond=NULL;
								if(it!=NULL && it->getStmtClass()==Stmt::IfStmtClass) 
								   cond=((IfStmt*)it)->getCond();
								if(it!=NULL && it->getStmtClass()==Stmt::ForStmtClass)
								   cond=((ForStmt*)it)->getCond();
								if(it!=NULL && it->getStmtClass()==Stmt::WhileStmtClass)
								   cond=((WhileStmt*)it)->getCond();
								//	if(stmt->getStmtClass()==Stmt::SwitchStmtClass) cond=((SwitchStmt*)stmt)->getCond();
								if(cond == iter->first)
								{
									Flag=true;
									getStmtVariables(iter->first);
									for (auto it: env.values)
									{
										for(auto myVD: variableList)
										{
											if(getString_of_Expr(myVD)==it.V->getName())
											{
												it.update(TaintValue(TaintKind::Tainted));
											}
										}
									}
								}
							}
							continue;
						}

				for(auto ele :*block)//::iterator iter2=block->begin();iter2!=block->end();iter2++)
				{
					//CFGElement element=ele;
					if(ele.getKind()==CFGElement::Kind::Statement)
					{
						Stmt* stmt=const_cast<Stmt*>(((CFGStmt*)&ele)->getStmt());		
                        

						if((Stmt*)RecordcallExpr!=stmt)
						{
							taint::visitStmt(stmt, penv);
						}
						else
						{
							for(auto ele:Tainted_Data_Expr_Set)
							{
								auto expr=ele.first;
								variableList.clear();
								getStmtVariables(expr);
							}
							for (auto var:variableList)
							{
								for (auto it: env.values)
								{
									if(getString_of_Expr(var)==it.V->getName() && it.is_tainted())
									{
										if (ReferableRepair) {
											string SensitiveFunName=it.V->getName();
											string ifCondition_String=getString_of_Expr(iter->first);

											map<std::string, funReference>::iterator iter =
												  funReferenceMap.find(SensitiveFunName);

											  if (iter == funReferenceMap.end()) {
												// no element of named sensitiveFunname
												funReference fs(SensitiveFunName);
												// add value to ifcheckInfo
												ifcheckInfo ifs(ifCondition_String);
												ifs.addloc();
												fs.getList().push_back(ifs);
												funReferenceMap.insert(make_pair(SensitiveFunName, fs));

											  } else {
												for (vector<ifcheckInfo>::iterator it =
														 iter->second.getList().begin();
													 it != iter->second.getList().end(); ++it) {
												  if (strcmp((*it).getIfcon().c_str(),ifCondition_String.c_str()) == 0) { // exist
													(*it).addloc();
													return true;
												  }
												}

												// not exist
												ifcheckInfo ifs(ifCondition_String);
												ifs.addloc();
												iter->second.getList().push_back(ifs);
											  }
											}

										return true;
									}
								}
										
							}
							return false;

						}
					}
				}
			}
		}
		
		
	}

}


//TODO:: add a back forward data flow analysis function to obtain the related variables to the tainted arguments

void TaintedMemOPChecker::backForward_DataFlowAnalysis( ASTFunction *caller, string Tainted_MemOP_location, map<Expr *, string> Tainted_Data_Expr_Set)
{
     

	//common::printLog("BFDFA ################# \n", common::CheckerName::memoryOPChecker, 5, *common::configure);

	FunctionDecl *FD = common::manager->getFunctionDecl(caller);
    SourceManager *sm;
    sm = &(FD->getASTContext().getSourceManager());
    auto callerCFG = common::manager->getCFG(caller, FD);
   	if (callerCFG != NULL)
   	{
			for (map<Expr *, string>::iterator iter = Tainted_Data_Expr_Set.begin();iter != Tainted_Data_Expr_Set.end(); ++iter) 
			{
				Expr* taintExpr= iter->first;
				string taintExprStr = getString_of_Expr(taintExpr);
				for(CFG::iterator iter=callerCFG->begin();iter!=callerCFG->end();++iter)
				{
					CFGBlock* block=*iter;		
					for(CFGBlock::reverse_iterator riter= block->rbegin(); riter!= block->rend(); ++riter)
					{
						CFGElement element=*(riter);
						if(element.getKind()==CFGElement::Kind::Statement)
						{
							Stmt *it = const_cast<Stmt *>(element.castAs<CFGStmt>().getStmt());		  
							  if (it != nullptr) 
							  {
		              			if (it->getStmtClass() == Stmt::BinaryOperatorClass) 
		              			{
		              				auto *binaryOpExpr = dyn_cast<BinaryOperator>(it);
		              				//assignment
		              				if(binaryOpExpr->isAssignmentOp()==true)
		              				{

		              					Expr *exprLHS = binaryOpExpr->getLHS();
		              					string leftStr=getString_of_Expr(exprLHS);
		              					Expr *exprRHS = binaryOpExpr->getRHS();
		              					string rightStr=getString_of_Expr(exprRHS);

		              					if(strcmp(taintExprStr.c_str(),leftStr.c_str())==0)
		              					{
		              						Tainted_Data_Expr_Set.insert(pair<Expr *, string>(exprRHS, rightStr));
		              						//common::printLog("add relate expr to Tainted_Data_Expr_Set"+ rightStr +"\n", common::CheckerName::memoryOPChecker, 5, *common::configure);
		              					}
		              					
										else if(strcmp(taintExprStr.c_str(),rightStr.c_str())==0)
		              					{
		              						Tainted_Data_Expr_Set.insert(pair<Expr *, string>(exprLHS, leftStr));
		              						//common::printLog("add relate expr to Tainted_Data_Expr_Set"+ leftStr +"\n", common::CheckerName::memoryOPChecker, 5, *common::configure);
		              					}
		              				}
		              			 }
							  }
		                 }
		        
					}
		    }
		  }
		}


}


// determine whether the ifcheck is right if check
bool TaintedMemOPChecker::is_right_ifCheck(
	  ASTFunction *caller,
    string Tainted_MemOPName, 
    string Tainted_MemOP_location, 
    map<Expr *, string> Tainted_Data_Expr_Set,
    map<Expr *, string> IfCheckInfoList) 
 {

 //	common::printLog("is_right_ifCheck ################# \n", common::CheckerName::memoryOPChecker, 5, *common::configure);

	//backForward_DataFlowAnalysis to collect the related variable of tainted arguments
	backForward_DataFlowAnalysis(caller, Tainted_MemOP_location, Tainted_Data_Expr_Set);

	string SensitiveFunName = Tainted_MemOPName;
  if (StringMode == true) {
    for (map<Expr *, string>::iterator taint_data_iter =
             Tainted_Data_Expr_Set.begin();
         taint_data_iter != Tainted_Data_Expr_Set.end(); taint_data_iter++) {
      string Tainted_Data_Expr_String = taint_data_iter->second.c_str();

      for (map<Expr *, string>::iterator ifcon_iterator =
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
              //common::printLog("Right if Check \n",
                            // common::CheckerName::memoryOPChecker, 1,
                            // *common::configure);
            
            clock_t repair_time_start = clock();
            if (ReferableRepair) {

              map<std::string, funReference>::iterator iter =
                  funReferenceMap.find(SensitiveFunName);

              if (iter == funReferenceMap.end()) {
                // no element of named sensitiveFunname
                funReference fs(SensitiveFunName);
                // add value to ifcheckInfo
                ifcheckInfo ifs(ifCondition_String);
                ifs.addloc();
                fs.getList().push_back(ifs);
                funReferenceMap.insert(make_pair(SensitiveFunName, fs));

              } else {
                // has element of named sensitiveFunName
                // ifcheckInfo exist ifConditionan_string?

                for (vector<ifcheckInfo>::iterator it =
                         iter->second.getList().begin();
                     it != iter->second.getList().end(); ++it) {
                  if (strcmp((*it).getIfcon().c_str(),
                             ifCondition_String.c_str()) == 0) { // exist
                    (*it).addloc();
                    return true;
                  }
                }

                // not exist
                ifcheckInfo ifs(ifCondition_String);
                ifs.addloc();
                iter->second.getList().push_back(ifs);
              }
            }
            repair_time += float(clock() - repair_time_start);
            return true;
          }

        }
      }
    }
  }



  // common::printLog("Wrong if Check \n", common::CheckerName::memoryOPChecker,
  // 5, *common::configure);
  return false;
}



void TaintedMemOPChecker::collect_missing_check_data(ASTFunction *caller,
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
  string F1 = "1:" + std::to_string(NumOfArg) + ",";

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
  string F2 = "2:" + std::to_string(NumOfPlus) + ",";

  // feature3: number of -
  string F3 = "3:" + std::to_string(NumOfMinus) + ",";

  // feature4: number of *
  string F4 = "4:" + std::to_string(NumOfMultiply) + ",";

  // feature5: number of /
  string F5 = "5:" + std::to_string(NumOfDivide) + ",";

  // feature6: number of %
  string F6 = "6:" + std::to_string(NumOfDelively) + ",";

  // feature7: number of simple variable
  string F7 = "7:" + std::to_string(NumOfSimpleVariable) + ",";

  // feature8: number of composite variable
  string F8 = "8:" + std::to_string(NumOfCompositeVariable) + ",";

  // feature9: number of sizeof
  string F9 = "9:" + std::to_string(NumOfSizeof) + ",";

  // feature10: numbers of variables in the body of caller
  int NumOfCallerVariables = common::getVariables(CallerFD).size();
  string F10 = "10:" + std::to_string(NumOfCallerVariables) + ",";

  // feature11: numbers of callexprs in the body of caller
  int NumOfCallerExprs = common::getCallExpr(CallerFD).size();
  string F11 = "11:" + std::to_string(NumOfCallerExprs) + ",";

  // feature12: if callee has body
  int CalleeHasBody = 0;
  // int CalleeHasIfCheck=0;
  if (CalleeFD->hasBody()) {
    CalleeHasBody = 1;
  }
  string F12 = "12:" + std::to_string(CalleeHasBody) + ",";

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

  string F13 = "13:" + std::to_string(NumOfOccurInBinoryOP) + ",";

  // feature14: numbers of binoryop "="
  string F14 = "14:" + std::to_string(NumOfBinoryOP) + ",";

  // feature15: numbers of taint arguments
  string F15 = "15:" + std::to_string(NumOfTaintedArgs) + ",";

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

/// determine if the missing check is dangerous based on the features
// TODO: distinguish upper and lower part of the context
// TODO: consider the features in the function body
// TODO: consider the features used by Mahin
bool TaintedMemOPChecker::is_dangerous_missing_check(ASTFunction *caller,
                                                     CallExpr *MemOPcallExpr,
                                                     FunctionDecl *callee) {
  clock_t Confirm_start = clock();

  FunctionDecl *CallerFD = common::manager->getFunctionDecl(caller);
  FunctionDecl *CalleeFD = callee;
  CFG *CallerCFG = common::manager->getCFG(caller, CallerFD);

  /*****************************************************************
  *record function and location information for feature extraction *
  *****************************************************************/

  SourceManager *sm;
  FunctionDecl *functionDecl = common::manager->getFunctionDecl(caller);
  string callerName=functionDecl->getName();
  string calleeName=callee->getName();
  sm = &(functionDecl->getASTContext().getSourceManager());
  
  string callerN=callerName+",";
  string calleeN=calleeName+",";

  string location = functionDecl->getLocStart().printToString(*sm);
  string loc=location+",";
  std::map<string, string>::iterator got = Function_Location.find(callerName);
  if (got == Function_Location.end()) 
  {
    Function_Location.insert(pair<string, string>(callerName, location));
  } 

  /*********************collect the features*****************************/

  // struct svm_node arr[15];
  // string F0=std::to_string(label)+" ";

  // feature1:numbers of arguments
  int NumOfArg = MemOPcallExpr->getNumArgs();
  string F1 = std::to_string(NumOfArg) + ",";
  // arr[0].index=1;
  // arr[0].value=NumOfArg;

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
  // arr[1].index=2;
  // arr[1].value=NumOfPlus;

  // feature3: number of -
  string F3 = std::to_string(NumOfMinus) + ",";
  // arr[2].index=3;
  // arr[2].value=NumOfMinus;

  // feature4: number of *
  string F4 = std::to_string(NumOfMultiply) + ",";
  // arr[3].index=4;
  // arr[3].value=NumOfMultiply;

  // feature5: number of /
  string F5 =  std::to_string(NumOfDivide) + ",";
  // arr[4].index=5;
  // arr[4].value=NumOfDivide;

  // feature6: number of %
  string F6 =  std::to_string(NumOfDelively) + ",";
  // arr[5].index=6;
  // arr[5].value=NumOfDelively;

  // feature7: number of simple variable
  string F7 =  std::to_string(NumOfSimpleVariable) + ",";
  // arr[6].index=7;
  // arr[6].value=NumOfSimpleVariable;

  // feature8: number of composite variable
  string F8 = std::to_string(NumOfCompositeVariable) + ",";
  // arr[7].index=8;
  // arr[7].value=NumOfCompositeVariable;

  // feature9: number of sizeof
  string F9 =  std::to_string(NumOfSizeof) + ",";
  // arr[8].index=9;
  // arr[8].value=NumOfSizeof;

  // feature10: numbers of variables in the body of caller
  int NumOfCallerVariables = common::getVariables(CallerFD).size();
  string F10 =  std::to_string(NumOfCallerVariables) + ",";
  // arr[9].index=10;
  // arr[9].value=NumOfCallerVariables;

  // feature11: numbers of callexprs in the body of caller
  int NumOfCallerExprs = common::getCallExpr(CallerFD).size();
  string F11 =  std::to_string(NumOfCallerExprs) + ",";
  // arr[10].index=11;
  // arr[10].value=NumOfCallerExprs;

  // feature12: if callee has body
  int CalleeHasBody = 0;
  // int CalleeHasIfCheck=0;
  if (CalleeFD->hasBody()) {
    CalleeHasBody = 1;
  }
  string F12 = std::to_string(CalleeHasBody) + ",";
  // arr[11].index=12;
  // arr[11].value=CalleeHasBody;

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

  string F13 =  std::to_string(NumOfOccurInBinoryOP) + ",";
  // arr[12].index=13;
  // arr[12].value=NumOfOccurInBinoryOP;

  // feature14: numbers of binoryop "="
  string F14 = std::to_string(NumOfBinoryOP) + ",";
  // arr[13].index=14;
  // arr[13].value=NumOfBinoryOP;

  // feature15: numbers of taint arguments
  string F15 =  std::to_string(NumOfTaintedArgs) + ",";
  // arr[14].index=15;
  // arr[14].value=NumOfTaintedArgs;

  // feature10: if callee's body contain a ifcheck in the definition of callee
  /*********************vector data *************************************/

  string item = callerN + loc + F1 + F2 + F3 + F4 + F5 + F6 + F7 + F8 + F9 + F10 + F11 +
                F12 + F13 + F14 + F15 + "\n";

  // writingToFeatureData(item);

  ofstream myfile;
  myfile.open("Predict_Feature_Data.csv", std::ios_base::app);
  myfile << item.c_str();
  myfile.close();

  /*********************Search the libsvm model*************************/

  // the command to call the svm-predicate
  confirm_time += clock() - Confirm_start;

  return true;
}

void TaintedMemOPChecker::OneCheck(ASTFunction *caller) {
  string CallStack = "";
  if (!has_ifCheck(caller)) {
     //collect_missing_check_data(caller, RecordcallExpr, RecordCallee, 1);
     if (is_dangerous_missing_check(caller, RecordcallExpr, RecordCallee)) {
      CallStack = Tainted_FunctionName + "; " + Tainted_MemOPName + "\n";
      Tainted_Description =
          "\n[" + Tainted_MemOPName + "]" +
          " is a memory operation function using tainted data: [" +
          Tainted_Data + "]\n" + "Location : [" + Tainted_MemOP_location +
          "]\n" + "Call Stack:\n" + CallStack;

      AddWarningItemToMap(Tainted_FileName, Tainted_FunctionName,
                          Tainted_MemOPName, Tainted_Data,
                          Tainted_MemOP_location, Tainted_Description, "100",
                          Tainted_Locline);
      return;
    }
  }

  else if (!is_right_ifCheck(caller, Tainted_MemOPName, Tainted_MemOP_location, Tainted_Data_Expr_Set,
                             IfCheckInfoList)) {

   //else if (!is_right_Check(caller, Tainted_MemOPName, Tainted_MemOP_location,RecordcallExpr, Tainted_Data_Expr_Set,
   //                         IfCheckInfoList)) {
   // collect_missing_check_data(caller, RecordcallExpr, RecordCallee, 1);

    if (is_dangerous_missing_check(caller, RecordcallExpr, RecordCallee)) {
      CallStack = Tainted_FunctionName + "; " + Tainted_MemOPName + "\n";
      Tainted_Description =
          "\n[" + Tainted_MemOPName + "]" +
          " is a memory operation function using tainted data: [" +
          Tainted_Data + "]\n" + "Location : [" + Tainted_MemOP_location +
          "]\n" + "Call Stack:\n" + CallStack;

      AddWarningItemToMap(Tainted_FileName, Tainted_FunctionName,
                          Tainted_MemOPName, Tainted_Data,
                          Tainted_MemOP_location, Tainted_Description, "100",
                          Tainted_Locline);
      return;
    }
  }
}

void TaintedMemOPChecker::TwoCheck(ASTFunction *caller) {
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
          if (is_dangerous_missing_check(caller, RecordcallExpr,
                                         RecordCallee)) {
            CallStack = CallStack + Caller2 + "; " + Tainted_FunctionName +
                        "; " + Tainted_MemOPName + "\n";
            Tainted_Description =
                "\n[" + Tainted_MemOPName + "]" +
                " is a memory operation function using tainted data: [" +
                Tainted_Data + "]\n" + "Location : [" + Tainted_MemOP_location +
                "]\n" + "Call Stack:\n" + CallStack;
            AddWarningItemToMap(Tainted_FileName, Tainted_FunctionName,
                                Tainted_MemOPName, Tainted_Data,
                                Tainted_MemOP_location, Tainted_Description,
                                "100", Tainted_Locline);
            return;
          }
        }

       //else if (!is_right_ifCheck(caller, Tainted_MemOPName, Tainted_MemOP_location, Tainted_Data_Expr_Set,
  //                           IfCheckInfoList)) {

        else if (!is_right_ifCheck(caller, Tainted_MemOPName, Tainted_MemOP_location, Tainted_Data_Expr_Set,
                             IfCheckInfoList)) {

          if (is_dangerous_missing_check(caller, RecordcallExpr,
                                         RecordCallee)) {
            CallStack = CallStack + Caller2 + "; " + Tainted_FunctionName +
                        "; " + Tainted_MemOPName + "\n";
            Tainted_Description =
                "\n[" + Tainted_MemOPName + "]" +
                " is a memory operation function using tainted data: [" +
                Tainted_Data + "]\n" + "Location : [" + Tainted_MemOP_location +
                "]\n" + "Call Stack:\n" + CallStack;
            AddWarningItemToMap(Tainted_FileName, Tainted_FunctionName,
                                Tainted_MemOPName, Tainted_Data,
                                Tainted_MemOP_location, Tainted_Description,
                                "100", Tainted_Locline);
            return;
          }
        }
      }

    } else {
      // common::printLog(Tainted_FunctionName + "has no parent callers",
      // common::CheckerName::memoryOPChecker, 5, *common::configure);
      CallStack = CallStack + Caller2 + "; " + Tainted_FunctionName + "; " +
                  Tainted_MemOPName + "\n";
      Tainted_Description =
          "\n[" + Tainted_MemOPName + "]" +
          " is a memory operation function using tainted data: [" +
          Tainted_Data + "]\n" + "Location : [" + Tainted_MemOP_location +
          "]\n" + "Call Stack:\n" + CallStack;
      AddWarningItemToMap(Tainted_FileName, Tainted_FunctionName,
                          Tainted_MemOPName, Tainted_Data,
                          Tainted_MemOP_location, Tainted_Description, "100",
                          Tainted_Locline);
      return;
    }

  } 
   //else if (!is_right_ifCheck(caller, Tainted_MemOPName, Tainted_MemOP_location, Tainted_Data_Expr_Set,
  //                           IfCheckInfoList)) {

   else if (!is_right_ifCheck(caller, Tainted_MemOPName, Tainted_MemOP_location, Tainted_Data_Expr_Set,
                             IfCheckInfoList)) {
    std::vector<ASTFunction *> ParentCallers =
        common::call_graph->getParents(caller);
    if (ParentCallers.size() != 0) {
      for (ASTFunction *Pcaller : ParentCallers) {
        // FunctionDecl* fNode = manager->getFunctionDecl(Pcaller);
        Caller2 = Pcaller->getName();
        if (!has_ifCheck(Pcaller)) {
          CallStack = CallStack + Caller2 + "; " + Tainted_FunctionName + "; " +
                      Tainted_MemOPName + "\n";
          Tainted_Description =
              "\n[" + Tainted_MemOPName + "]" +
              " is a memory operation function using tainted data: [" +
              Tainted_Data + "]\n" + "Location : [" + Tainted_MemOP_location +
              "]\n" + "Call Stack:\n" + CallStack;
          AddWarningItemToMap(Tainted_FileName, Tainted_FunctionName,
                              Tainted_MemOPName, Tainted_Data,
                              Tainted_MemOP_location, Tainted_Description,
                              "100", Tainted_Locline);
          return;
        } 
	 //else if (!is_right_ifCheck(caller, Tainted_MemOPName, Tainted_MemOP_location, Tainted_Data_Expr_Set,
  //                           IfCheckInfoList)) {

        else if (!is_right_ifCheck(caller, Tainted_MemOPName, Tainted_MemOP_location, Tainted_Data_Expr_Set,
                             IfCheckInfoList)) {
          CallStack = CallStack + Caller2 + "; " + Tainted_FunctionName + "; " +
                      Tainted_MemOPName + "\n";
          Tainted_Description =
              "\n[" + Tainted_MemOPName + "]" +
              " is a memory operation function using tainted data: [" +
              Tainted_Data + "]\n" + "Location : [" + Tainted_MemOP_location +
              "]\n" + "Call Stack:\n" + CallStack;
          AddWarningItemToMap(Tainted_FileName, Tainted_FunctionName,
                              Tainted_MemOPName, Tainted_Data,
                              Tainted_MemOP_location, Tainted_Description,
                              "100", Tainted_Locline);
          return;
        }
      }
    } else {
      // common::printLog(Tainted_FunctionName + "has no parent callers",
      // common::CheckerName::memoryOPChecker, 5,*common::configure);
      CallStack = CallStack + Caller2 + "; " + Tainted_FunctionName + "; " +
                  Tainted_MemOPName + "\n";
      Tainted_Description =
          "\n[" + Tainted_MemOPName + "]" +
          " is a memory operation function using tainted data: [" +
          Tainted_Data + "]\n" + "Location : [" + Tainted_MemOP_location +
          "]\n" + "Call Stack:\n" + CallStack;
      AddWarningItemToMap(Tainted_FileName, Tainted_FunctionName,
                          Tainted_MemOPName, Tainted_Data,
                          Tainted_MemOP_location, Tainted_Description, "100",
                          Tainted_Locline);

      return;
    }

  } else {
    return;
  }
}

// N level check the existness and correctness of ifcheck for memory operation
// funcation
void TaintedMemOPChecker::N_Level_Check(ASTFunction *caller, int N) {

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
void TaintedMemOPChecker::visitCallExpr(CallExpr *callExpr,
                                        ASTFunction *caller) {
  Tainted_Locline = "";
  Tainted_MemOPName = "";
  Tainted_FunctionName = "";

  // get the caller name
  std::string callerName;
  FunctionDecl *functionDecl = common::manager->getFunctionDecl(caller);
  if (caller) {
    callerName = caller->getName();
    // common::printLog("caller: " + callerName +"\n",
    // common::CheckerName::memoryOPChecker, 5, *common::configure);
  }

  // get the callee name
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

  // check if the function is security sensitive
  if (!is_MemOP(calleeName)) {
    return;
  } else {

    SourceManager *sm;
    sm = &(functionDecl->getASTContext().getSourceManager());

    if (!sm->isLoadedSourceLocation(callExpr->getLocStart())) {
      // common::printLog("unloaded callExpr: " +
      // callExpr->getDirectCallee()->getNameAsString() + "\n",
      // common::CheckerName::memoryOPChecker, 5, *common::configure);
      return;
    }

    // record the location of the function call
    Tainted_MemOP_location = callExpr->getLocStart().printToString(*sm);

    // check if the arg should be checked
    if (!has_Tainted_Arg(callExpr, location_of_Arg_str)) {
      return;
    } else {
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
      // check if there exists missing check
      N_Level_Check(caller, MemOPCheckLevel);
    }
  }
}

string TaintedMemOPChecker::getCodeLine(string location) {
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

int TaintedMemOPChecker::substringCount(string str, string sub) {

  int nRet = 0, nStart = 0;
  while (-1 != (nStart = str.find(sub, nStart))) {
    nStart += sub.length();
    ++nRet;
  }
  return nRet;
}

std::string TaintedMemOPChecker::printStmt(Stmt *stmt) {
  LangOptions L0;
  L0.CPlusPlus = 1;
  std::string buffer1;
  llvm::raw_string_ostream strout1(buffer1);
  stmt->printPretty(strout1, nullptr, PrintingPolicy(L0));
  return "" + strout1.str() + "";
}

std::string TaintedMemOPChecker::itos(int n) {
  const int max_size =
      std::numeric_limits<int>::digits10 + 1 /*sign*/ + 1 /*0-terminator*/;
  char buffer[max_size] = {0};
  sprintf(buffer, "%d", n);
  return std::string(buffer);
}

// write the warning information into the XML file.
void TaintedMemOPChecker::writingToXML(
    std::string fileName, std::string funName, std::string sensitiveFunName,
    std::string taintedArgs, std::string location, std::string description,
    std::string riskdegree, std::string locLine) {

  // TODO: check the output for dangerous missing check warnings
  pugi::xml_node node = doc.append_child("error");

  pugi::xml_node checker = node.append_child("checker");
  checker.append_child(pugi::node_pcdata).set_value("MOLINT.TAINTED.MEM.OP");

  pugi::xml_node domain = node.append_child("domain");
  domain.append_child(pugi::node_pcdata).set_value("STATIC_C");

  pugi::xml_node event = node.append_child("Event");

  pugi::xml_node file = event.append_child("file");
  file.append_child(pugi::node_pcdata).set_value(fileName.c_str());

  pugi::xml_node caller = event.append_child("Callerfunction");
  caller.append_child(pugi::node_pcdata).set_value(funName.c_str());

  pugi::xml_node sensitivefun = event.append_child("Sensitivefunction");
  sensitivefun.append_child(pugi::node_pcdata)
      .set_value(sensitiveFunName.c_str());

  pugi::xml_node tainted = event.append_child("TaintedArguments");
  tainted.append_child(pugi::node_pcdata).set_value(taintedArgs.c_str());

  pugi::xml_node taintedLocation = event.append_child("Location");
  taintedLocation.append_child(pugi::node_pcdata).set_value(location.c_str());

  pugi::xml_node descr = event.append_child("Description");
  descr.append_child(pugi::node_pcdata).set_value(description.c_str());

  pugi::xml_node riks = event.append_child("riskDegree");
  riks.append_child(pugi::node_pcdata).set_value(riskdegree.c_str());

  pugi::xml_node line = event.append_child("line");
  line.append_child(pugi::node_pcdata).set_value(locLine.c_str());
}

// write the warning information into the XML file.
void TaintedMemOPChecker::writingToCodeReference(string sensitivename,
                                                 string arr[], int brr[]) {
  CheckSampleNum++;

  // string link=fileName+":"+startLocation;
  pugi::xml_node node =
      checkSampleDoc.append_child("Reference_for_Repairing_Missing_Check");

  pugi::xml_node SensitiveFunction = node.append_child("SensitiveFunction");
  SensitiveFunction.append_child(pugi::node_pcdata)
      .set_value(sensitivename.c_str());

  // TODO: check the OUTPUT for repairs
  // TODO: assertion: size(arr) >= ARRAYSIZE
  for (int i = 0; i < ARRAYSIZE; i++) {
    if (arr[i] != "") {
      pugi::xml_node ifCheck = node.append_child("ReferableCheck");
      ifCheck.append_child(pugi::node_pcdata).set_value(arr[i].c_str());

      pugi::xml_node Num = ifCheck.append_child("Number");

      Num.append_child(pugi::node_pcdata).set_value(itos(brr[i]).c_str());
    }
  }
}

void TaintedMemOPChecker::writingToFeatureData(string data_item) {
  DataNum++;

  // string link=fileName+":"+startLocation;
  pugi::xml_node node =
      Label_Feature_Data_Doc.append_child("Label_Feature_Data");

  pugi::xml_node item = node.append_child("item");
  item.append_child(pugi::node_pcdata).set_value(data_item.c_str());
}

void TaintedMemOPChecker::AddWarningItemToMap(
    std::string fileName, std::string funName, std::string sensitiveFunName,
    std::string taintedArgs, std::string location, std::string description,
    std::string riskdegree, std::string locLine) {
  WarningNum++;
  WarningList.insert(pair<string, WarningItem>(
      location, WarningItem(fileName, funName, sensitiveFunName, taintedArgs,
                            location, description, riskdegree, locLine)));
}

/**************************************************************************/
/** auxilliary functions **************************************************/  
/**************************************************************************/  

clock_t TaintedMemOPChecker::get_time() { return run_time; }

clock_t TaintedMemOPChecker::get_Confirm_time() { return confirm_time; }

clock_t TaintedMemOPChecker::get_Repair_time() { return repair_time; }

clock_t TaintedMemOPChecker::get_Detect_time() {
  detect_time = run_time - confirm_time - repair_time;
  return detect_time;
}

