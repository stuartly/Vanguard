#include "TaintedMemOPChecker.h"


void TaintedMemOPChecker::check(ASTFunction *F, FunctionDecl *FD, CFG *function_cfg)
{
	clock_t start = clock(); 
    readConfig();   
	if(F != NULL)
	{	 
		visitStmt(F,FD,function_cfg);
	}
	run_time += clock() - start;
}

clock_t TaintedMemOPChecker::get_time() 
{
    
    return run_time;
}

TaintedMemOPChecker::TaintedMemOPChecker()
{
	WarningNum=0;
}

TaintedMemOPChecker::~TaintedMemOPChecker()
{
	
	if(WarningNum != 0)
	{
		std::string xmlFile = (*common::configure)["pathToReport"]["path"].asString() + "MOLINT.TAINTED.MEM.OP.xml";
		doc.save_file(xmlFile.c_str(),"\t", pugi::format_default|pugi::format_no_declaration);
	}


}

    
  // visit each stmt, and check the stmt if the stmt is a callexprclass  
void TaintedMemOPChecker::visitStmt(ASTFunction *F, FunctionDecl *FD, CFG *function_cfg)
{	    
		common::printLog( "visit function:----------------"+F->getName(), common::CheckerName::memoryOPChecker, 3, *common::configure);	
		for (CFGBlock* block :  *function_cfg)
		{
				for (auto element : *block)
				{
					if (element.getKind() == CFGElement::Statement)
					{
						const Stmt* stmt = element.castAs<CFGStmt>().getStmt();
						if(stmt!=nullptr)
							{
								if(stmt->getStmtClass() == Stmt::CallExprClass)
								{
									 if(const CallExpr* callExpr = dyn_cast<CallExpr>(stmt))
									{
										if(callExpr!=nullptr)
										{											
											RecordfunDel= const_cast<FunctionDecl *>(FD);
											Recordblock=const_cast<CFGBlock*>(block);
											Recordstmt=const_cast<Stmt*>(stmt);
											RecordcallExpr=const_cast<CallExpr*>(callExpr);
											common::printLog("visit CallExpr:----------------\n ", common::CheckerName::memoryOPChecker, 3, *common::configure);									
											visitCallExpr(RecordcallExpr,F);
										}
	            					}
								}
							}   
					 }
  				}
			}
	
	return;

}

// read the informaion from config, including the memory operation funcation names, check level, check mode, etc.
void TaintedMemOPChecker::readConfig()
{
    if((*common::configure)["MemOPCheckLevel"]["N"].empty())  
	{
		MemOPCheckLevel = 1;
	}
	else
	{
		MemOPCheckLevel = (*common::configure)["MemOPCheckLevel"]["N"].asInt();
	}

	if((*common::configure)["RightIfCheckMode"]["StringMode"].empty())
	{
		StringMode = true;

	}
	else
	{
		StringMode=(*common::configure)["RightIfCheckMode"]["StringMode"].asBool();
	}

	if((*common::configure)["RightIfCheckMode"]["ASTMode"].empty())
	{
		ASTMode = true;

	}
	else
	{
		ASTMode=(*common::configure)["RightIfCheckMode"]["ASTMode"].asBool();
	}


	if((*common::configure)["RightIfCheckMode"]["Z3Mode"].empty())
	{
		Z3Mode = true;

	}
	else
	{
		Z3Mode=(*common::configure)["RightIfCheckMode"]["Z3Mode"].asBool();
	}

	if((*common::configure)["SimpleExprCheckOnly"]["MemopSimpleValue"].empty())
	{
		MemopSimpleValue=true;
	}
	else
	{

		MemopSimpleValue=(*common::configure)["SimpleExprCheckOnly"]["MemopSimpleValue"].asBool();
	}

	for(auto mem:(*common::configure)["MemoryOPFunction"].getMemberNames())
	{
		MemoryOPFunction[mem] = (*common::configure)["MemoryOPFunction"][mem].asInt();
	}

}

//determine whether it is a memory operation function by comparing the funcation name
bool TaintedMemOPChecker::is_MemOP(std::string calleeName)	
{
	std::unordered_map<std::string, int>::iterator got = MemoryOPFunction.find(calleeName);	
	if(got == MemoryOPFunction.end())
	{
        return false;
	}
    else
	{
		location_of_Arg=got->second;
		common::printLog(calleeName +" is a memory operation function!\n", common::CheckerName::memoryOPChecker, 5,*common::configure);
        return true;
	}
	
}

bool TaintedMemOPChecker::is_Tainted(Expr *arg)
{
	
	
	if(RecordfunDel!=nullptr&&Recordblock!=nullptr&&Recordstmt!=nullptr&&RecordcallExpr!=nullptr&&arg!=nullptr)
	 {
		
		 if(taint::TaintChecker::is_tainted(RecordfunDel,Recordblock,Recordstmt,arg))
		 {
			 common::printLog(getString_of_Expr(arg)+ " is tainted data!\n", common::CheckerName::memoryOPChecker, 1, *common::configure);
			 return true;  
		 }
		 else
		 {
			 common::printLog(getString_of_Expr(arg)+ " is not tainted data!\n", common::CheckerName::memoryOPChecker, 1, *common::configure);
			 return false;
		 }

	 }
	 else
	 {
		 return false;
	 }
       

}

std::string TaintedMemOPChecker::getString_of_Expr(Expr* expr)
{	
	LangOptions L0;
	L0.CPlusPlus=1;
	std::string buffer1;
	llvm::raw_string_ostream strout1(buffer1);
	expr->printPretty(strout1,nullptr,PrintingPolicy(L0));
	return strout1.str();
}

bool  TaintedMemOPChecker::has_Tainted_Arg(CallExpr* callExpr,int location_of_Arg)
{
	
	Tainted_Data="";
	Tainted_Data_Expr=NULL;
	int NumofArgs=callExpr->getNumArgs(); 	
	if(location_of_Arg>=NumofArgs)
	{
		common::printLog("error config", common::CheckerName::memoryOPChecker, 1, *common::configure);
		return false;
	}
	else
	{
		Expr *arg =const_cast<Expr*>(callExpr->getArg(location_of_Arg));	
		Expr *arg2=callExpr->getArg(location_of_Arg)->IgnoreImpCasts()->IgnoreImpCasts()->IgnoreParenCasts()->IgnoreImpCasts();
		string argStr=getString_of_Expr(arg);

		//do not check the complex exprs		
		if(MemopSimpleValue==true) 
		{
			if(dyn_cast<DeclRefExpr>(arg2))
			{
				
			}
			else			
			{
				return false;
			}
		}
		else if(strstr(argStr.c_str(),"+")||strstr(argStr.c_str(),"- ")||strstr(argStr.c_str(),"*")||strstr(argStr.c_str(),"/")||strstr(argStr.c_str(),"%")||strstr(argStr.c_str(),"sizeof")||(strstr(argStr.c_str(),"(")&& strstr(argStr.c_str(),")")))
		{
			
			
			
			return false;

		}

		
		
		
		
		if(is_Tainted(arg))
        {
			Tainted_Data_Expr=arg;
			Tainted_Data=getString_of_Expr(arg);
			return true;
			
        }
		else
		{
			return false;
		}
		
	}
	
}

//Determine whether there is an if the check
bool  TaintedMemOPChecker::has_ifCheck( ASTFunction * caller)
{

	    If_ConditionExpr_Set.clear();
		string callerstr=caller->getName();
		FunctionDecl* functionDecl = common::manager->getFunctionDecl(caller);
        if(!(functionDecl->hasBody()))
        {
			common::printLog(callerstr + "has no ifCheck! \n", common::CheckerName::memoryOPChecker, 1, *common::configure);
            return false;
        }
        else
        {
			FunctionDecl* FD = common::manager->getFunctionDecl(caller);
		    auto callerCFG = common::manager->getCFG(caller,FD);       
			if(callerCFG!=NULL)
			{
				 LangOptions LO;
				 LO.CPlusPlus=true;
				
				 for(CFG::iterator iter=callerCFG->begin();iter!=callerCFG->end();++iter)
				 {
					CFGBlock* block=*iter;					
					Stmt* it=(block->getTerminator().getStmt());					
					if(it!=NULL&&it->getStmtClass()==Stmt::IfStmtClass)
					{
						Expr * cond=((IfStmt*)it)->getCond();
						if(cond !=NULL)
						{
							If_ConditionExpr_Set.insert(cond);
												
						}						
					}
				 }

				 if(If_ConditionExpr_Set.size()!=0)
				 {
					 common::printLog(callerstr + "has ifCheck! \n", common::CheckerName::memoryOPChecker, 1, *common::configure);
					 return true;
				 }
				 else
				 {
					 common::printLog(callerstr + "has no ifCheck! \n", common::CheckerName::memoryOPChecker, 1, *common::configure);
					 return false;
				 }
			}
			else
			{
				common::printLog(callerstr + "has no ifCheck! \n", common::CheckerName::memoryOPChecker, 1, *common::configure);
				return false;
			}
			
			
        }
		
      
}

std::string TaintedMemOPChecker::getMemOPCheckStlye(std::string Tainted_MemOPName)
{
	std::unordered_map<std::string,std::string>::iterator got = MemOPCheckStlye.find(Tainted_MemOPName);
	std::string checkStlye;
	if(got == MemOPCheckStlye.end())
	{
		checkStlye="NotFind";
	}
    else
	{
		checkStlye=got->second;
	}
	return checkStlye;
	 
}

// determine whethere the tainted_data has been checked rightly in the if_condition based on the string mode.
bool TaintedMemOPChecker::is_Relevent_String(std::string if_condition_string,std::string Tainted_Data_Expr_string)
{
	
	if(strstr(if_condition_string.c_str(),Tainted_Data_Expr_string.c_str()))
	{
		
		//common::printLog("Is Revelent of "+if_condition_string+" and  "+Tainted_Data_Expr_string+"\n", common::CheckerName::memoryOPChecker, 1,*common::configure);
		return true;
	}
	else
	{
		
		//common::printLog("Not Revelent of "+if_condition_string+" and  "+Tainted_Data_Expr_string+"\n", common::CheckerName::memoryOPChecker, 1, *common::configure);
		return false;

	}
}

Node* TaintedMemOPChecker::createTree(Expr* expr)
{
	
	if(expr!=NULL)
	{
		Node* root=new Node;
		
		while(expr->getStmtClass()==Stmt::ParenExprClass)
		{
			ParenExpr * parExpr=dyn_cast<ParenExpr>(expr);
			expr =parExpr->getSubExpr();
		}

		if(expr->getStmtClass()==Stmt::BinaryOperatorClass)
		{
			BinaryOperator * binaryOpExpr=dyn_cast<BinaryOperator>(expr);	
			int opCode=binaryOpExpr->getOpcode();
			std::string value=getOpCodeString(opCode);
			root->NodeValue=getOpCodeString(opCode);
			Expr* exprLHS=binaryOpExpr->getLHS();
			Expr* exprRHS=binaryOpExpr->getRHS();
			
			if(exprLHS!=NULL)
			{
				root->LHS=createTree(exprLHS);
			}
			else
			{
				root->LHS=NULL;
			}

			if(exprRHS!=NULL)
			{	
				root->RHS=createTree(exprRHS);

			}
			else
			{
				root->RHS=NULL;
			}
				
		}
		else 
		{
			root->NodeValue=getString_of_Expr(expr);
			root->LHS=NULL;
			root->RHS=NULL;
			
		}
		return root;
		
	}
	else
	{
		return NULL;
	}		
	
}

std::string TaintedMemOPChecker::getOpCodeString(int opCode)
	{
		std::string result="";
		switch(opCode)
		{
			case 5:  
				{
					result="+";
					break;
				}
			case 24: 
				{
					result="+=";
					break;
				}
			case 15: 
				{
					result="&";
					break;
				}
			case 28: 
				{
					result="&=";
					break;
				}
			case 20: 
				{
					result="=";
					break;
				}
			case 31:
				{
					result=",";
					break;
				}
			case 3:  
				{
					result="/";
					break;
				}
			case 22: 
				{
					result="/=";
					break;
				}
			case 13:  
				{
					result="==";
					break;
				}
			case 12: 
				{
					result=">=";
					break;
				}
			case 10:  
				{
					result=">";
					break;
				}
			case 18:  
				{
					result="&&";
					break;
				}
			case 11: 
				{
					result="<=";
					break;
				}
			case 9:  
				{
					result="<";
					break;
				}
			case 2:  
				{
					result="*";
					break;
				}
			case 21:  
				{
					result="*=";
					break;
				}
			case 14:  
				{
					result="!=";
					break;
				}
			case 17:  
				{
					result="||";
					break;
				}
			case 30: 
				{
					result="|=";
					break;
				}
			case 6:  
				{
					result="-";
					break;
				}
            case 25:  
				{
					result="-=";
					break;
				}
			default:break;
		}
		return result;
		
	}

void TaintedMemOPChecker::getTreePreOrderString(Node* T)
{
	
	if(T!=NULL)
	{
		
		if(PreOrderString=="")
		{
			PreOrderString=T->NodeValue;
		}
		else
		{
			PreOrderString+=T->NodeValue;

		}
		getTreePreOrderString(T->LHS);
		getTreePreOrderString(T->RHS);
	}

	

	 
	
}

void  TaintedMemOPChecker::NormallizeOrder(Node* T)
{
	if(T!=NULL)
	{
		if(T->NodeValue=="+"||T->NodeValue=="*")
		{
			if(Height(T->LHS)>Height(T->RHS))
			{
				Node * temp;
				temp=T->LHS;
				T->LHS=T->RHS;
				T->RHS=temp;
			}
			else if(Height(T->LHS)==Height(T->RHS))
			{

				if(strcmp((T->LHS->NodeValue).c_str(),(T->RHS->NodeValue).c_str())>0)
				{
					Node * temp;
					temp=T->LHS;
					T->LHS=T->RHS;
					T->RHS=temp;

				}
			}
			
		}
		
		NormallizeOrder(T->LHS);
		NormallizeOrder(T->RHS);
		
	}
}

bool TaintedMemOPChecker::isSubTree(Node *bigTreeRoot,Node *smallTreeRoot) // if smallTree is the subtree of bigTree ?
{
       if(smallTreeRoot==NULL)
	   {
            return true;
	   }

       if(bigTreeRoot==NULL)
	   {
            return false;
	   }

	   if(bigTreeRoot->NodeValue==smallTreeRoot->NodeValue)

		   return isSubTree(bigTreeRoot->LHS,smallTreeRoot->LHS) &&isSubTree(bigTreeRoot->RHS,smallTreeRoot->RHS);

       else

		   return isSubTree(bigTreeRoot->LHS,smallTreeRoot) || isSubTree(bigTreeRoot->RHS,smallTreeRoot);

}

bool TaintedMemOPChecker::isEqualTree(Node* root1, Node* root2) 
{
     if(root1==NULL && root2==NULL)
	 {
		 return true;
	 }
     else if (root1==NULL || root2==NULL)
		 {
			return false;
		 }
		 else
		 {
			 return (root1->NodeValue==root2->NodeValue) && isEqualTree(root1->LHS,root2->LHS) && isEqualTree(root1->RHS,root2->RHS);
		 }     
}

int TaintedMemOPChecker::Height(Node* T)  
{    
    if(T==NULL) return 0;  
    else  
    {  
		int m = Height(T->LHS);  
		int n = Height(T->RHS);  
        return (m > n) ? (m+1) : (n+1);   
    }  
}


// determine whether the tainted_data has been checked rightly in the if_condition based on the AST mode.
bool TaintedMemOPChecker::is_Relevent_Expr(Expr* if_condition, Expr* Tainted_Data_Expr)
{
	Tree ifConditonTree;
	ifConditonTree.root=createTree(if_condition);
	
	NormallizeOrder(ifConditonTree.root);
	
	Tree taintDataTree;
	taintDataTree.root=createTree(Tainted_Data_Expr);
	NormallizeOrder(taintDataTree.root);

	if(isSubTree(ifConditonTree.root,taintDataTree.root))
	{
		//common::printLog("The tainted data is part of if_condition\n", common::CheckerName::memoryOPChecker, 1, *common::configure);
		return true;
	}
	else
	{
	   // common::printLog("The tainted data is not part of if_condition\n", common::CheckerName::memoryOPChecker, 1, *common::configure);
		return false;
	}
	return false;	
}




bool TaintedMemOPChecker::is_Equal_String(std::string if_condition, std::string rightCheckStlye)
{
	if(if_condition==rightCheckStlye)
	{
		return true;
	}
	else
	{
		return false;
	}
}


//determine whether the ifcheck is right if check.
bool  TaintedMemOPChecker::is_right_ifCheck(std::string Tainted_MemOPName, Expr *Tainted_Data_Expr, std::set<Expr*> If_ConditionExpr_Set) 
{
	
	if(StringMode==true)
	{
		int N=0;
		
		common::printLog("StringMode--------------------\n", common::CheckerName::memoryOPChecker, 1,*common::configure);
		for(auto if_condition: If_ConditionExpr_Set)
		{
			
				std::string ifCondition_String=getString_of_Expr(if_condition).c_str();
				string Tainted_Data_Expr_String=getString_of_Expr(Tainted_Data_Expr).c_str();

					if(is_Relevent_String(ifCondition_String,Tainted_Data_Expr_String.c_str()))
					{	
						common::printLog("Right if Check \n", common::CheckerName::memoryOPChecker, 1, *common::configure);
						return true;             
                 
						/*
						get the right check style from config file
						std::string checkStlye=getMemOPCheckStlye(Tainted_MemOPName);
			
						if(checkStlye =="NotFind")
						{
							//llvm::outs()<<"not find checkStlye \n";
							common::printLog("Not find checkStlye \n", common::CheckerName::memoryOPChecker, 1, *common::configure);
						}
						else
						{
							//llvm::outs()<<Tainted_MemOPName<<" CheckStlye:"<<checkStlye<<"\n";
							common::printLog(Tainted_MemOPName+" CheckStlye:"+checkStlye+"\n", common::CheckerName::memoryOPChecker, 1,*common::configure);
							if(is_Equal_String(ifCondition_String,checkStlye))
							{
								return true;
							}
						}
						*/
					
					}
			
		}
		
	}

	//ASTMode	
	else  if(ASTMode==true)
	{
		//common::printLog("ASTMode-----------------------\n", common::CheckerName::memoryOPChecker, 1, *common::configure);
		for(auto if_condition: If_ConditionExpr_Set)
		{  			
				if(is_Relevent_Expr(if_condition,Tainted_Data_Expr))
				{
					common::printLog("Right if Check \n", common::CheckerName::memoryOPChecker, 1, *common::configure);
					return true;
				    /*
					//get the right check style from config file
					std::string checkStlye=getMemOPCheckStlye(Tainted_MemOPName);
			

					if(checkStlye =="NotFind")
					{
						
						common::printLog("not find checkStlye \n", common::CheckerName::memoryOPChecker, 1, *common::configure);
					}
					else
					{
						
						common::printLog(Tainted_MemOPName+" CheckStlye:"+checkStlye+"\n", common::CheckerName::memoryOPChecker, 1, *common::configure);
						if(is_Equal_String(ifCondition_String,checkStlye))
						{
							return true;
						}
					}
					*/
					
				}			
		}
	}	

	common::printLog("Wrong if Check \n", common::CheckerName::memoryOPChecker, 1, *common::configure);
	return false;

}




void TaintedMemOPChecker::TwoCheck(ASTFunction* caller)
{
	    string CallStack="";
				
		if(!has_ifCheck(caller))
		{		
				
			std::vector<ASTFunction *> ParentCallers=common::call_graph->getParents(caller) ;	
				if(ParentCallers.size()!=0)
				{
					for(ASTFunction * Pcaller: ParentCallers)
					{	
						Caller2=Pcaller->getName();
						if(!has_ifCheck(Pcaller))
							{
								CallStack=CallStack + Caller2 +"; " + Tainted_FunctionName +"; " +Tainted_MemOPName +"\n";	
								Tainted_Description="\n["+Tainted_MemOPName+"]"+" is a memory operation function using tainted data: ["+Tainted_Data+ "]\n"+"Location : ["+Tainted_MemOP_location+"]\n"+"Call Stack:\n"+ CallStack;
								writingToXML(Tainted_FileName, Tainted_FunctionName, Tainted_Description, Tainted_Locline);
								return;
									
							}
						else if(!is_right_ifCheck(Tainted_MemOPName,Tainted_Data_Expr,If_ConditionExpr_Set))
								{
									CallStack=CallStack + Caller2 +"; " + Tainted_FunctionName +"; " +Tainted_MemOPName +"\n";
									Tainted_Description="\n["+Tainted_MemOPName+"]"+" is a memory operation function using tainted data: ["+Tainted_Data+ "]\n"+"Location : ["+Tainted_MemOP_location+"]\n"+"Call Stack:\n"+ CallStack;
									writingToXML(Tainted_FileName, Tainted_FunctionName, Tainted_Description, Tainted_Locline);
									return;										
								}
							
					}
					

					
				}
				else
				{
					common::printLog(Tainted_FunctionName + "has no parent callers", common::CheckerName::memoryOPChecker, 1, *common::configure);
					CallStack=CallStack + Caller2 +"; " + Tainted_FunctionName +"; " +Tainted_MemOPName +"\n";
					Tainted_Description="\n["+Tainted_MemOPName+"]"+" is a memory operation function using tainted data: ["+Tainted_Data+ "]\n"+"Location : ["+Tainted_MemOP_location+"]\n"+"Call Stack:\n"+ CallStack;
					writingToXML(Tainted_FileName, Tainted_FunctionName, Tainted_Description, Tainted_Locline);
					return;
					

				}

		}
	    else if(!is_right_ifCheck(Tainted_MemOPName,Tainted_Data_Expr,If_ConditionExpr_Set))
			{
				std::vector<ASTFunction *> ParentCallers=common::call_graph->getParents(caller) ;	
				if(ParentCallers.size()!=0)
				{
					for(ASTFunction * Pcaller: ParentCallers)
					{	
						//FunctionDecl* fNode = manager->getFunctionDecl(Pcaller);
						Caller2=Pcaller->getName();
						if(!has_ifCheck(Pcaller))
							{
								CallStack=CallStack + Caller2 +"; " + Tainted_FunctionName +"; " +Tainted_MemOPName +"\n";
								Tainted_Description="\n["+Tainted_MemOPName+"]"+" is a memory operation function using tainted data: ["+Tainted_Data+ "]\n"+"Location : ["+Tainted_MemOP_location+"]\n"+"Call Stack:\n"+ CallStack;
								writingToXML(Tainted_FileName, Tainted_FunctionName, Tainted_Description, Tainted_Locline);
								return;
								
								
								
							}
						else if(!is_right_ifCheck(Tainted_MemOPName,Tainted_Data_Expr,If_ConditionExpr_Set))
								{
									CallStack=CallStack + Caller2 +"; " + Tainted_FunctionName +"; " +Tainted_MemOPName +"\n";
									Tainted_Description="\n["+Tainted_MemOPName+"]"+" is a memory operation function using tainted data: ["+Tainted_Data+ "]\n"+"Location : ["+Tainted_MemOP_location+"]\n"+"Call Stack:\n"+ CallStack;
									writingToXML(Tainted_FileName, Tainted_FunctionName, Tainted_Description, Tainted_Locline);
									return;
																		
								}
								
					}
				}
				else
				{
					common::printLog(Tainted_FunctionName + "has no parent callers", common::CheckerName::memoryOPChecker, 1,*common::configure);
					CallStack=CallStack + Caller2 +"; " + Tainted_FunctionName +"; " +Tainted_MemOPName +"\n";
					Tainted_Description="\n["+Tainted_MemOPName+"]"+" is a memory operation function using tainted data: ["+Tainted_Data+ "]\n"+"Location : ["+Tainted_MemOP_location+"]\n"+"Call Stack:\n"+ CallStack;
					writingToXML(Tainted_FileName, Tainted_FunctionName, Tainted_Description, Tainted_Locline);
					return;
					

				}
				
			}
		else

		{
			return;
		}
}

//N level check the existness and correctness of ifcheck for memory operation funcation
void TaintedMemOPChecker::N_Level_Check( ASTFunction* caller,int N)
{

	if(caller==NULL )
	{
		//common::printLog("caller is null !", common::CheckerName::memoryOPChecker, 5, *common::configure);
		return;
	}

	if(N<0||N >1)
	{
		//common::printLog("Warning: MemOPCheckLevel config is wrong !", common::CheckerName::memoryOPChecker, 5, *common::configure);
		N=1;		
	}	
	
	if(N==1)
	{
		TwoCheck(caller);		
	}
   
}

string   replace_all(string   str,const   string   old_value,const   string   new_value)     
{     
    while(true)   {     
        string::size_type   pos(0);     
        if(   (pos=str.find(old_value))!=string::npos   )     
            str.replace(pos,old_value.length(),new_value);     
        else   break;     
    }     
    return   str;     
} 

// Visit each callexpr, determine whether it's a memory operation funcation, if it is, determine whether there is a ifcheck and validate the correctness of ifcheck.
void TaintedMemOPChecker::visitCallExpr(CallExpr* callExpr, ASTFunction* caller)
{
	Tainted_Locline="";
	Tainted_MemOPName="";
	Tainted_FunctionName="";
	std::string callerName;
	FunctionDecl* functionDecl = common::manager->getFunctionDecl(caller);
	if(caller)
	{
		callerName=caller->getName();		
		common::printLog("caller: " + callerName +"\n", common::CheckerName::memoryOPChecker, 1, *common::configure);
	}

	std::string calleeName;
	if(callExpr->getDirectCallee()!=NULL)
	{
		 const FunctionDecl* callee = callExpr->getDirectCallee();
		 calleeName=callee->getNameAsString ();
		 common::printLog("callee: " + calleeName +"\n", common::CheckerName::memoryOPChecker, 1, *common::configure);
	}
	else
	{
		return;
	}

    if(!is_MemOP(calleeName))
    {
        return;
    }
    else
    {
        SourceManager *sm;
        sm = &(functionDecl->getASTContext().getSourceManager());

        if (!sm->isLoadedSourceLocation(callExpr->getLocStart()))
		{
			common::printLog("unloaded callExpr: " + callExpr->getDirectCallee()->getNameAsString() + "\n", common::CheckerName::memoryOPChecker, 1, *common::configure);
            return ;
        }

        Tainted_MemOP_location = callExpr->getLocStart().printToString(*sm);

		// record the location of MemOP
		std::string tempLoc=Tainted_MemOP_location;
	    std::string LineNoRow,Line;	 
	
        if(!has_Tainted_Arg (callExpr,location_of_Arg))
        {
            return;
        }
        else 
		{
			// record the location of MemOP
			int pos1 = tempLoc.find_last_of('<');
			if(pos1 == -1)
			{
				pos1 = tempLoc.find_last_of(':');
			}
			else
			{
    			tempLoc.assign(tempLoc.c_str(),pos1);
    			pos1 = tempLoc.find_last_of(':');
			}

            LineNoRow.assign(tempLoc.c_str(),pos1);
			int pos2=LineNoRow.find_last_of(':');
			Tainted_FileName.assign(LineNoRow.c_str(),pos2);
			Tainted_FileName=replace_all(Tainted_FileName,"\\","/");
			Tainted_MemOP_location=replace_all(Tainted_MemOP_location,"\\","/");
			Line=LineNoRow.substr(pos2+1);
			Tainted_Locline=Line;		
			Tainted_MemOPName=calleeName;
			Tainted_FunctionName=callerName;
			Caller2="";
			Caller3="";	
			N_Level_Check(caller,MemOPCheckLevel);
		}		
    }	
}

    

//write the warning information into the XML file.
void TaintedMemOPChecker::writingToXML(std::string fileName, std::string funName, std::string descr, std::string locLine)
{
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







