
#include "ArrayOutOfBoundsCheckerV4.h"

#include "clang/Frontend/FrontendAction.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/DeclBase.h"
#include "clang/AST/Decl.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Sema/Sema.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Support/MemoryBuffer.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Basic/IdentifierTable.h"
#include "clang/Frontend/ASTUnit.h"

using namespace std;

map<bool, int> calculateExpr(Expr* e){
	map<bool, int> result;
	if(e == NULL){
		result.insert(make_pair(false, -1));
		return result;
	}
	if(ParenExpr* pa_expr = dyn_cast<ParenExpr>(e)){
		Expr* sub_expr = pa_expr->getSubExpr();
		if(sub_expr == NULL){
			result.insert(make_pair(false, -1));
			return result;
		}else{
			return calculateExpr(sub_expr);
		}
	}else if(BinaryOperator* bo_expr = dyn_cast<BinaryOperator>(e)){
		Expr* lhs = bo_expr->getLHS();
		Expr* rhs = bo_expr->getRHS();
		if(lhs == NULL || rhs == NULL){
			result.insert(make_pair(false, -1));
			return result;
		}else{
			auto l_result = calculateExpr(lhs);
			auto r_result = calculateExpr(rhs);
			if(l_result.find(false) != l_result.end() || r_result.find(false) != r_result.end()){
				result.insert(make_pair(false, -1));
				return result;
			}else{
				BinaryOperatorKind op = bo_expr->getOpcode();
				if(op == BinaryOperatorKind::BO_Assign){
					return r_result;
				}else if(op == BinaryOperatorKind::BO_Add || op == BinaryOperatorKind::BO_AddAssign){
					result.insert(make_pair(true, l_result[true]+r_result[true]));
					return result;
				}else if(op == BinaryOperatorKind::BO_And || op == BinaryOperatorKind::BO_AndAssign){
					result.insert(make_pair(true, l_result[true]&r_result[true]));
					return result;
				}else if(op == BinaryOperatorKind::BO_Div || op == BinaryOperatorKind::BO_DivAssign){
					result.insert(make_pair(true, l_result[true]/r_result[true]));
					return result;
				}else if(op == BinaryOperatorKind::BO_Mul || op == BinaryOperatorKind::BO_MulAssign){
					result.insert(make_pair(true, l_result[true]*r_result[true]));
					return result;
				}else if(op == BinaryOperatorKind::BO_Or || op == BinaryOperatorKind::BO_OrAssign){
					result.insert(make_pair(true, l_result[true]|r_result[true]));
					return result;
				}else if(op == BinaryOperatorKind::BO_Rem || op == BinaryOperatorKind::BO_RemAssign){
					result.insert(make_pair(true, l_result[true]%r_result[true]));
					return result;
				}else if(op == BinaryOperatorKind::BO_Shl || op == BinaryOperatorKind::BO_ShlAssign){
					result.insert(make_pair(true, l_result[true]<<r_result[true]));
					return result;
				}else if(op == BinaryOperatorKind::BO_Shr || op == BinaryOperatorKind::BO_ShrAssign){
					result.insert(make_pair(true, l_result[true]>>r_result[true]));
					return result;
				}else if(op == BinaryOperatorKind::BO_Sub || op == BinaryOperatorKind::BO_SubAssign){
					result.insert(make_pair(true, l_result[true]-r_result[true]));
					return result;
				}else if(op == BinaryOperatorKind::BO_Xor || op == BinaryOperatorKind::BO_XorAssign){
					result.insert(make_pair(true, l_result[true]^r_result[true]));
					return result;
				}else{
					result.insert(make_pair(false, -1));
					return result;
				}
			}
		}
	}else if(IntegerLiteral* IL_expr = dyn_cast<IntegerLiteral>(e)){
		int value = IL_expr->getValue().getSExtValue();
		result.insert(make_pair(true, value));
		return result;
	}else if(ImplicitCastExpr* ICE_expr = dyn_cast<ImplicitCastExpr>(e)){
		Expr* sub_expr = ICE_expr->getSubExpr();
		auto val = calculateExpr(sub_expr);
		if(val.find(false) == val.end()){
			result.insert(make_pair(true, val[true]));
			return result;
		}else{
			result.insert(make_pair(false, -1));
			return result;
		}
		
	}else if(UnaryOperator* uo_expr = dyn_cast<UnaryOperator>(e)){
		Expr* sub_expr = uo_expr->getSubExpr();
		//sub_expr->dump();
		if(sub_expr != NULL && uo_expr->getOpcode() == UnaryOperatorKind::UO_Minus){
			auto val = calculateExpr(sub_expr);
			if(val.find(false) != val.end()){
				result.insert(make_pair(false, -1));
				return result;
			}else{
				result.insert(make_pair(true, -val[true]));
				return result;
			}
		}else{
			result.insert(make_pair(false, -1));
			return result;
		}
	}else{
		result.insert(make_pair(false, -1));
		return result;
	}
}


std::string printStmt(Stmt* stmt)
{
	LangOptions L0;
	L0.CPlusPlus=1;
	std::string buffer1;
	llvm::raw_string_ostream strout1(buffer1);
	stmt->printPretty(strout1,nullptr,PrintingPolicy(L0));
	return ""+strout1.str()+"";
}

bool equal(AtomicAPInt& l,AtomicAPInt& r)
{
		if(l.op==r.op&&printStmt(l.lhs)==printStmt(r.lhs)&&l.rhs==r.rhs)return true;
		else
			return false;
}

bool equal(vector<AtomicAPInt>& l,vector<AtomicAPInt>& r)
{
	if(l.size()!=r.size())return false;
	for (unsigned i = 0; i < l.size(); ++i)
	{
		bool flag=false;
		for (unsigned j = 0; j < r.size(); ++j)
		{
			/* code */
			if(equal(l[i],r[j]))
			{
				flag=true;
				break;
			}
		}
		if(!flag)return false;
	}
	return true;
}

bool equal(ArraySubscript& l,ArraySubscript& r)
{
	if(l.func==r.func&&l.block==r.block&&l.stmt==r.stmt&&l.index==r.index&&equal(l.condition,r.condition)&&\
		l.location==r.location&&l.ID==r.ID&&l.isLoopBoundChecking==r.isLoopBoundChecking&&l.indexCnt==r.indexCnt&&l.arrayName==r.arrayName&&l.arrayExpr==r.arrayExpr)return true;
		else
			return false;
}

vector<Expr*> Intersection(vector<Expr*>& l,vector<Expr*>& s)
{
	vector<Expr*> r;
	for(unsigned i=0;i<l.size();i++)
		for(unsigned j=0;j<s.size();j++)
			if(printStmt(l[i])==printStmt(s[j]))
				r.push_back(l[i]);
	return r;
}

vector<Expr*> Union(vector<Expr*>& l,vector<Expr*>& s)
{
	vector<Expr*> r;
	for(unsigned i=0;i<l.size();i++)	r.push_back(l[i]);
	
	for(unsigned i=0;i<s.size();i++)	
	{
		if(find(r.begin(),r.end(),s[i])==r.end())
		r.push_back(s[i]);
	}
	return r;
}

vector<ArraySubscript> Union(vector<ArraySubscript>& l,vector<ArraySubscript>& s)
{
	vector<ArraySubscript> r;
	for(unsigned i=0;i<l.size();i++)	r.push_back(l[i]);
	
	for(unsigned i=0;i<s.size();i++)	
	{
		bool flag=false;
		for(unsigned j=0;j<r.size();j++)
		{
			if(equal(r[j],s[i]))//s[i]
			{
				flag=true;
				break;
			}
		}
		if(!flag)
		r.push_back(s[i]);
	}
	return r;
}

vector<ArraySubscript> Intersection(vector<ArraySubscript>& l,vector<ArraySubscript>& s)
{
	vector<ArraySubscript> r;
	for(unsigned i=0;i<l.size();i++)	
	{
		bool flag=false;
		for(unsigned j=0;j<s.size();j++)
		{
			if(equal(s[j],l[i]))//s[i]
			{
				flag=true;
				break;
			}
		}
		if(!flag)
			r.push_back(l[i]);
	}
	return r;
}

bool isEqual(vector<Expr*>& l,vector<Expr*>& r)
{
	if(l.size()!=r.size())return false;
	for(unsigned i=0;i<l.size();i++)
	{
		bool flag=false;
		for(unsigned j=0;j<r.size();j++)
		{
			if(printStmt(l[i])==printStmt(r[j]))
			{
				flag=true;
				break;
			}
		}
		if(!flag)
		{
			return false;
		}
	}
	return true;
}






bool isEuql(vector<ArraySubscript>& l,vector<ArraySubscript>& r)
{
	if(l.size()!=r.size())return false;
	for (unsigned i = 0; i < l.size(); ++i)
	{
		bool flag=false;
		for (unsigned j = 0; j < r.size(); ++j)
		{
			/* code */
			if(equal(l[i],r[j]))
			{
				flag=true;
				break;
			}
		}
		if(!flag)return false;
	}
	return true;
}


bool isEqual( vector<pair<CFGBlock*,vector<Expr*> > >& l, vector<pair<CFGBlock*,vector<Expr*> > >& r)
{
	if(l.size()!=r.size())return false;
	for(unsigned i=0;i<l.size();i++)
	{
		for(unsigned j=0;j<r.size();j++)
		{
			if(l[i].first==r[j].first)
			{
				vector<Expr*> left=l[i].second;
				vector<Expr*> right=r[j].second;
				if(!isEqual(left,right))return false;
				break;
			}
		}
	}
	return true;
}


//ArrayOutOfBoundsCheckerV4::ArrayOutOfBoundsCheckerV4(ASTResource *resource, ASTManager *manager, CallGraph *call_graph, Config *common::configure) {
//	depth=1;
//}
//void ArrayOutOfBoundsCheckerV4::check(ASTFunction *F, FunctionDecl *FD, CFG *function_cfg){
	/*SimpleCheckEnable=false;
	doc_loopbound_empty=true;
	doc_arraybound_empty=true;
	SimpleExprCheckOnly=true;
	IndexIgnoreConditionalOperator=true;
	depth=(*common::configure)["ArrayBound"]["checkdepth"].asInt64();

	if((*common::configure)["ArrayBound"]["SimpleCheckEnable"].asInt()){
		SimpleCheckEnable=true;
	}
	if(!((*common::configure)["ArrayBound"]["IndexIgnoreConditionalOperator"].asInt())){
		IndexIgnoreConditionalOperator=false;	
	}
	if(!((*common::configure)["ArrayBound"]["SimpleExprCheckOnly"].asInt())){
		SimpleExprCheckOnly=false;	
	}
	warningCount = 0;*/

	//checkFunc(F);
	
	//std::////std::cerr<<c<<std::endl;
	//common::printLog( "TOTAL warning:"+int2string(warningCount)+"\n",common::CheckerName::arrayBound,4,*common::configure);
	//common::printLog( reportWarnings,common::CheckerName::arrayBound,0,*common::configure);
    /*std::string xmlFile_loopbound = (*common::configure)["pathToReport"]["path"].asString() + "MOLINT.TAINTED.LOOP.BOUND.xml";
	std::string xmlFile_arraybound = (*common::configure)["pathToReport"]["path"].asString()  + "MOLINT.TAINTED.ARRAY.INDEX.xml";
	if(!doc_loopbound_empty)	doc_loopbound.save_file(xmlFile_loopbound.c_str(), "\t", pugi::format_no_declaration);
	if(!doc_arraybound_empty) doc_arraybound.save_file(xmlFile_arraybound.c_str(), "\t", pugi::format_no_declaration);*/
//}

class ASTCalledExprFind : public RecursiveASTVisitor<ASTCalledExprFind> {
    
public:
    bool VisitCallExpr(CallExpr *E) {
        if (FunctionDecl *FD = E->getDirectCallee()) {
	    std::string fullName=common::getFullName(FD);
            functions.insert(fullName);
            if(map.find(fullName)==map.end())
            {
            		std::vector<CallExpr*> tmp;
            		tmp.push_back(E);
            		map.insert(pair<string,std::vector<CallExpr* > >(fullName,tmp));
            }
            else
            		map[fullName].push_back(E);
        }
        return true;
    }

    bool getFunctions(FunctionDecl* FD) {
        return functions.find(common::getFullName(FD))!=functions.end();
    }

    std::vector<CallExpr*> getCallExpr(FunctionDecl* FD)
    {
    	return map[common::getFullName(FD)];
    }

private:
    std::set<std::string> functions;
    std::map<std::string,std::vector<CallExpr* > > map;
};

vector<ArraySubscript> ArrayOutOfBoundsCheckerV4::getFatherScript(vector<ArraySubscript> list,ASTFunction * father,ASTFunction * current,unordered_set<int>& errors,unordered_set<int>& erase)
{
	vector<ArraySubscript> result;

	FunctionDecl *pFunc = common::manager->getFunctionDecl(father);
	FunctionDecl *cFunc = common::manager->getFunctionDecl(current);
	CFG * pCFG= common::manager->getCFG(father,pFunc);	
	//CFG * cCFG= common::manager->getCFG(current,cFunc);
	for(CFG::iterator iter=pCFG->begin();iter!=pCFG->end();++iter){
		CFGBlock* block=*iter;
		for(CFGBlock::iterator iter=block->begin();iter!=block->end();iter++)
			{
				CFGElement element=*(iter);
				if(element.getKind()==CFGElement::Kind::Statement)
				{
					const Stmt* it=((CFGStmt*)&element)->getStmt();
					
					 ASTCalledExprFind load;
    					 load.TraverseStmt((Stmt*)it);
   					 if(load.getFunctions(cFunc))
   					 {
   					 	std::vector<CallExpr*> callexprs=load.getCallExpr(cFunc);
   					 	for(auto sc : list)
   					 	{
   					 		for(auto ce : callexprs)
   					 		{
	   					 		string index=printStmt(sc.index);
	   					 		common::printLog( "find index:"+index+"\n",common::CheckerName::arrayBound,1,*common::configure);
	   					 		bool hasP=false;
	   					 		if (MemberExpr *MRE = dyn_cast<MemberExpr>(sc.index))
								{
									Expr* tmp;
									if (CXXConstructExpr *CXXC = dyn_cast<CXXConstructExpr>(MRE->getBase()))
									{

										tmp=CXXC->getArg(0);
									}
									else
									{
										tmp=MRE->getBase();
									}
									if (DeclRefExpr *ref = dyn_cast<DeclRefExpr>(tmp->IgnoreImpCasts()->IgnoreParenCasts()))
									{
										if (ParmVarDecl *pvd = dyn_cast<ParmVarDecl>(ref->getDecl()))
										{
											index=pvd->getNameAsString();
										}
									}
								}
								//cout<<index<<endl;
	   					 		for(unsigned i=0;i<cFunc->getNumParams();i++)
								{
									auto p=cFunc->getParamDecl(i);
									string name=p->getNameAsString();
									//cout<<"\t"<<name<<endl;
									common::printLog( "check var:"+name+"\n",common::CheckerName::arrayBound,1,*common::configure);
									if(name==index)
									{
										hasP=true;
										ArraySubscript tmp=sc.copy();
			   					 		tmp.block=block;
			   					 		tmp.stmt=(Stmt*)it;
			   					 		tmp.isLoopBoundChecking=false;
			   					 		tmp.func=pFunc;
			   				// 	 		////cerr<<"********************"<<endl;
			   				// 	 		////cerr<<printStmt(tmp.index)<<endl;
										// for(auto sub : tmp.condition)
										// 	////cerr<<"\t"<<printStmt(sub.lhs)<<endl;
										Expr* rhs=ce->getArg(i)->IgnoreImpCasts()->IgnoreParenCasts();
										if(IntegerLiteral *ILE = dyn_cast<IntegerLiteral>(rhs)) 
										{
											uint64_t value=ILE->getValue().getLimitedValue();
											//cout<<value<<end;
											bool flag=true;
											for(unsigned j=0;j<sc.condition.size();j++)
											{
												AtomicAPInt temp=sc.condition[j];
												if(temp.op==clang::BinaryOperatorKind::BO_LT)
												{
													if(value>=temp.rhs)flag=false;
												}
												if(temp.op==clang::BinaryOperatorKind::BO_GE)
												{
													if(value<temp.rhs)flag=false;
												}
											}
											if(flag)
											{
												erase.insert(sc.ID);
												//cout<<"erase"<<endl;
												break;
											}
										}
										else if(printStmt(rhs).find("sizeof")!=std::string::npos)
										{
											erase.insert(sc.ID);
											break;
										}
										else if(FloatingLiteral *ILE = dyn_cast<FloatingLiteral>(rhs))//-ImplicitCastExpr
										{
											double value=ILE->getValue().convertToDouble();
											//////////cerr<<value<<endl;
											bool flag=true;
											for(unsigned j=0;j<sc.condition.size();j++)
											{
												AtomicAPInt temp=sc.condition[j];
												if(temp.op==clang::BinaryOperatorKind::BO_LT)
												{
													if(value>=temp.rhs)flag=false;
												}
												if(temp.op==clang::BinaryOperatorKind::BO_GE)
												{
													if(value<temp.rhs)flag=false;
												}
											}
											if(flag)
											{
												erase.insert(sc.ID);
												break;
											}
										}
										else if(DeclRefExpr * ref=dyn_cast<DeclRefExpr>(rhs)){
											
											if(EnumConstantDecl * EC=dyn_cast<EnumConstantDecl>(ref->getDecl())){
												uint64_t value=EC->getInitVal().getLimitedValue ();
												common::printLog( "EnumConstantDecl:"+int2string(value)+"\n",common::CheckerName::arrayBound,0,*common::configure);	
												bool flag=true;	
												for(unsigned j=0;j<sc.condition.size();j++)
												{
													AtomicAPInt temp=sc.condition[j];
													if(temp.op==clang::BinaryOperatorKind::BO_LT)
													{
														if(temp.op==clang::BinaryOperatorKind::BO_LT)
														{
															if(value>=temp.rhs)flag=false;
														}
														if(temp.op==clang::BinaryOperatorKind::BO_GE)
														{
															if(value<temp.rhs)flag=false;
														}

													}
													
												}
												if(flag)
												{
													erase.insert(sc.ID);
													break;
												}
											}
										}
										tmp.changeIndex(ce->getArg(i),true);
										//tmp.print();
										// ////cerr<<"********************"<<endl;
										//cerr<<printStmt(tmp.index)<<endl;
										// for(auto sub : tmp.condition)
										// 	////cerr<<"\t"<<printStmt(sub.lhs)<<endl;
										common::printLog( "get Expr:"+printStmt(ce->getArg(i))+"\n",common::CheckerName::arrayBound,3,*common::configure);
   					 					result.push_back(tmp);	
										break;
									}
								}
								if(!hasP)
   					 			{
   					 				errors.insert(sc.ID);
   					 				mapToPath[sc.ID].insert(callpath);
   					 			}
   					 		}
   					 		
   					 	}
   					 }
				}
			}
	}
	return result;
}

void ArrayOutOfBoundsCheckerV4::DFS2func(ASTFunction *astFunc,int level,vector<ArraySubscript> list,unordered_set<int>& result)
{
	FunctionDecl *f = common::manager->getFunctionDecl(astFunc);
	DeclarationName DeclName = f->getNameInfo().getName();
	std::string FuncName = DeclName.getAsString();
	callpath.push_back(FuncName);
	common::printLog( "DFS into:"+FuncName+"\n",common::CheckerName::arrayBound,3,*common::configure);
	if(level<=0)
	{
		for(ArraySubscript s : list)
		{
			result.insert(s.ID);
			//cerr<<"push callpath"<<endl;
			mapToPath[s.ID].insert(callpath);
		}
		callpath.pop_back();
		common::printLog( "DFS out:"+FuncName+"\n",common::CheckerName::arrayBound,3,*common::configure);
		return;
	}

	std::vector<ASTFunction *> parents=common::call_graph->getParents(astFunc);
	if(parents.size()==0)
	{
		for(ArraySubscript s : list)
		{
			result.insert(s.ID);
			//cerr<<"push callpath"<<endl;
			mapToPath[s.ID].insert(callpath);
		}
		callpath.pop_back();
		common::printLog( "DFS out:"+FuncName+"\n",common::CheckerName::arrayBound,3,*common::configure);
		return;
	}
	else
	{
		for(ASTFunction* father : parents)
		{
			FuncNow=common::manager->getFunctionDecl(father);
			CFG * pCFG= common::manager->getCFG(father,FuncNow);					
			unordered_set<int> erase;
			std::vector<ArraySubscript> input=getFatherScript(list,father,astFunc,result,erase);
			
			if(input.size()>0)
			{
				vector<ArraySubscript> temp = backForwordAnalyse(pCFG,input,result);
				//////cerr<<"Remain Condition:"<<temp.size()<<endl;
				DFS2func(father,level-1,temp,result);
			}
			else
			{
				for(ArraySubscript s : list)
				{
					if(erase.find(s.ID)!=erase.end())continue;
					result.insert(s.ID);
					mapToPath[s.ID].insert(callpath);
				}
			}
			for(ArraySubscript s : input)
			{
				s.resetIndex(true);
				
			}
		}
	}
	callpath.pop_back();
	common::printLog( "DFS out:"+FuncName+"\n",common::CheckerName::arrayBound,3,*common::configure);
}
void ArrayOutOfBoundsCheckerV4::check(ASTFunction *F, FunctionDecl *FD, CFG *function_cfg)
//void ArrayOutOfBoundsCheckerV4::checkFunc(ASTFunction *astFunc)
{   
	common::printLog( "Start checkFunc:"+FD->getNameAsString()+"\n",common::CheckerName::arrayBound,4,*common::configure);
	/*
		FunctionDecl *Func = common::manager->getFunctionDecl(astFunc);
	std::unique_ptr<CFG> & myCFG= common::manager->getCFG(astFunc,Func);*/	
	if ( DebugMode&& DebugFunctionName!="None" && DebugFunctionName != "" && DebugFunctionName != FD->getNameAsString()) {
	        return;
	}
	clock_t start = clock();
   
	FuncNow=FD;

	if(DebugMode)
	{
		LangOptions L0;
		L0.CPlusPlus=1;
		function_cfg->dump(L0,true);
	}

	callpath.clear();
	mapToPath.clear();	
	reportedID.clear();
	sameID.clear();
		
	vector<ArraySubscript> list = LocatingTaintExpr(FD,function_cfg);

	//////cerr<<"LocatingTaintExpr end, list size is:"<<list.size()<<endl;
	common::printLog( "LocatingTaintExpr end, list size is:"+int2string(list.size())+"\n",common::CheckerName::arrayBound,1,*common::configure);
	if(list.size()==0) {
		common::printLog( "list size is 0, checkFunc return\n",common::CheckerName::arrayBound,1,*common::configure);
		return;	
	}
	set<std::vector<string> > tt;
	for(unsigned i=0;i<list.size();i++)
	{
		list[i].ID=i;
		mapToPath[i]=tt;
		list[i].orignIndex=list[i].index;
	}
	vector<ArraySubscript> temp = backForwordAnalyse(function_cfg,list);
	common::printLog( "OUT backForwordAnalyse:\n",common::CheckerName::arrayBound,1,*common::configure);
	common::printLog( "check depth:"+int2string(depth)+"\n",common::CheckerName::arrayBound,4,*common::configure);
	// for(unsigned i=0;i<temp.size();i++)
	// {
	// 	//temp[i].changeIndex(temp[i].orignIndex);
	// 	temp[i].print();
	// }
	// for(unsigned i=0;i<temp.size()-1;i++)
	// 	for(unsigned j=i+1;j<temp.size();j++)
	// 	{
	// 		if(temp[i].ID==temp[j].ID)
	// 			temp.erase(temp.begin()+j);
	// 	}
	if(temp.size()!=0)
	{
		//baocuo
		unordered_set<int> result;
		DFS2func(F,depth,temp,result);
		for(unsigned i=0;i<temp.size();i++){
			common::printLog( "after dfs:"+int2string(i)+"\n",common::CheckerName::arrayBound,0,*common::configure);
			if(result.find(temp[i].ID)!=result.end())
			{
				common::printLog( "ID:"+int2string(temp[i].ID)+"\n",common::CheckerName::arrayBound,0,*common::configure);
				map<int,vector<ArraySubscript>>::iterator find=sameID.find(temp[i].ID);
				temp[i].resetIndex(temp[i].orignIndex);
				if(find!=sameID.end()){
					vector<ArraySubscript> sameIDAS = find->second;
					reportWarningSameID(sameIDAS);
					
				}else{
					reportWarning(temp[i]);
				}
				// vector<vector<string> > tmp_path=mapToPath[temp[i].ID];
				// for(vector<string> path : tmp_path)
				// {
				// 	for(string str :path)
				// 		cerr<<str<<"->";
				// 	cerr<<endl;
				// }
				result.erase(temp[i].ID);
			}
		}
		//for(unsigned i=0;i<temp.size();i++)
		//	//////cerr<<"error:"<<temp[i].location<<endl;
	}

	common::printLog( "End checkFunc!\n",common::CheckerName::arrayBound,4,*common::configure);
	callpath.clear();
	reportedID.clear();
	mapToPath.clear();
	sameID.clear();
	
	run_time += clock() - start;
}
clock_t ArrayOutOfBoundsCheckerV4::get_time() {
    
    return run_time;
}
vector<ArraySubscript> ArrayOutOfBoundsCheckerV4::backForwordAnalyse(CFG * cfg,vector<ArraySubscript> list,unordered_set<int>& set)
{
	common::printLog( "Begin backForwordAnalyse:\n",common::CheckerName::arrayBound,3,*common::configure);
	CFGBlock* Entry;
	unfinished.clear();
	mapToBlockIn.clear();
	mapToBlockOut.clear();
	//prevous strp
	common::printLog( "initial map\n",common::CheckerName::arrayBound,1,*common::configure);
	for(CFG::iterator iter=cfg->begin();iter!=cfg->end();++iter){
		CFGBlock* block=*iter;
		if(block->pred_begin()==block->pred_end())
		{
			Entry=block;
		}
		vector<ArraySubscript> temp;
		mapToBlockIn.insert(pair<CFGBlock*, vector<ArraySubscript> >(block,temp));
		mapToBlockOut.insert(pair<CFGBlock*, vector<ArraySubscript> >(block,temp));
	}
	//add condition
	for (unsigned i = 0; i < list.size(); ++i)
	{
		/* code */

		CFGBlock* block=list[i].block;
		Stmt *stmt=list[i].stmt;
		stack<const Stmt*> s;
		if(list[i].isLoopBoundChecking)
		{
			for(CFGBlock::iterator iter=block->begin();iter!=block->end();iter++)
			{
				CFGElement element=*(iter);
				if(element.getKind()==CFGElement::Kind::Statement)
				{
					const Stmt* it=((CFGStmt*)&element)->getStmt();
					
					s.push(it);
				}
			}
		}
		else
		{
			for(CFGBlock::iterator iter=block->begin();iter!=block->end();iter++)
			{
				CFGElement element=*(iter);
				if(element.getKind()==CFGElement::Kind::Statement)
				{
					const Stmt* it=((CFGStmt*)&element)->getStmt();
					if(it==stmt)
					{
						break;
					}
					else
					{
						s.push(it);
					}
				}
			}
		}
		bool n=true;
		while(!s.empty())
		{
			const Stmt* it=s.top();
			s.pop();
			int result=throughStmt((Stmt*)it,list[i],block);
			if(result==1)
			{
				n=false;
				list.erase(list.begin()+i);
				i--;
				while(!s.empty()) s.pop();
				break;
			}
			else if(result==-1)
			{		
				set.insert(list[i].ID);		
				//reportWarning(list[i]);
				////////cerr<<"result==-1"<<endl;
				////////cerr<<"error："<<list[i].location<<endl;
				n=false;
				list.erase(list.begin()+i);
				i--;
				while(!s.empty()) s.pop();
				break;
			}
			else
			{
			}
		}
		if(n)
		{
			////////cerr<<"backForwardAnalyse:"<<endl;
			//block->dump();
			map<CFGBlock*, vector<ArraySubscript> >::iterator block_out_iter=mapToBlockOut.find(block);
		//	if(block_out_iter == mapToBlockOut.end()) {//////cerr<<"!!!Error!!!!!!!!!!!!!!mapToBlockOut not find block"<<endl;return list;}
			vector<ArraySubscript> temp=mapToBlockOut[block];
			temp.push_back(list[i]);
			mapToBlockOut.erase(block_out_iter);
			mapToBlockOut.insert(pair<CFGBlock*, vector<ArraySubscript> >(block,temp));
			for(CFGBlock::pred_iterator pred_iter=block->pred_begin();pred_iter!=block->pred_end();++pred_iter){
				CFGBlock *pred=*pred_iter;
				if(pred==NULL)continue;
				unfinished.push_back(pred);
				/* code */
			}
		}
	}
	common::printLog( "map condition over\n",common::CheckerName::arrayBound,1,*common::configure);
	////////cerr<<"end in"<<endl;
	//flow analyse
	while(unfinished.size()!=0){
		CFGBlock* block=unfinished[0];
		////////cerr<<block->getBlockID()<<endl;
		common::printLog( "data analyse for block "+int2string(block->getBlockID())+"\n",common::CheckerName::arrayBound,1,*common::configure);
		unfinished.erase(unfinished.begin());
		vector<ArraySubscript> temp_in;
		//vector<ArraySubscript> temp_out;
		bool T=true;
		for(CFGBlock::succ_iterator succ_iter=block->succ_begin();succ_iter!=block->succ_end();++succ_iter){
			CFGBlock* succ=*succ_iter;
			if(succ==NULL)continue;
			vector<ArraySubscript> c_in=mapToBlockOut[succ];
			Stmt* it=(block->getTerminator().getStmt());
			if(it!=NULL)
			{
				vector<ArraySubscript> tmp_child=getScript(block,it,c_in,T);
				temp_in=Union(temp_in,tmp_child);
			}
			else
			{
				temp_in=Union(temp_in,c_in);
			}
			T=false;
		}
		
		common::printLog( "get condition from children\n",common::CheckerName::arrayBound,1,*common::configure);
		std::vector<ArraySubscript> temp_out=flowThrogth(block,temp_in);
		common::printLog( "analyse stmt in block\n",common::CheckerName::arrayBound,1,*common::configure);
		std::vector<ArraySubscript> orign_out=mapToBlockOut[block];
		
		if(!isEuql(temp_out,orign_out))
		{
			for(CFGBlock::pred_iterator pred_iter=block->pred_begin();pred_iter!=block->pred_end();++pred_iter){
				CFGBlock *pred=*pred_iter;
				if(pred==NULL)continue;
				unfinished.push_back(pred);
			}

				map<CFGBlock*, vector<ArraySubscript> >::iterator block_out_iter=mapToBlockOut.find(block);
				
				mapToBlockOut.erase(block_out_iter);
				
				mapToBlockOut.insert(pair<CFGBlock*, vector<ArraySubscript> >(block,Union(temp_out,orign_out)));
			
		}
		common::printLog( "update condition in block\n",common::CheckerName::arrayBound,1,*common::configure);
	}
	common::printLog( "End backForwordAnalyse:\n",common::CheckerName::arrayBound,3,*common::configure);
	return mapToBlockOut[Entry];
}


vector<ArraySubscript> ArrayOutOfBoundsCheckerV4::backForwordAnalyse(CFG * cfg,vector<ArraySubscript> list)
{
	common::printLog( "Begin backForwordAnalyse:\n",common::CheckerName::arrayBound,3,*common::configure);
	//cfg->dump();
	CFGBlock* Entry;
	unfinished.clear();
	mapToBlockIn.clear();
	mapToBlockOut.clear();
	//prevous strp
	common::printLog( "initial map\n",common::CheckerName::arrayBound,1,*common::configure);
	for(CFG::iterator iter=cfg->begin();iter!=cfg->end();++iter){
		CFGBlock* block=*iter;
		if(block->pred_begin()==block->pred_end())
		{
			Entry=block;
		}
		vector<ArraySubscript> temp;
		mapToBlockIn.insert(pair<CFGBlock*, vector<ArraySubscript> >(block,temp));
		mapToBlockOut.insert(pair<CFGBlock*, vector<ArraySubscript> >(block,temp));
	}
	//add condition
	for (unsigned i = 0; i < list.size(); ++i)
	{
		/* code */

		CFGBlock* block=list[i].block;
		Stmt *stmt=list[i].stmt;
		stack<const Stmt*> s;
		//if(list[i].isLoopBoundChecking)
		//{
		//	for(CFGBlock::iterator iter=block->begin();iter!=block->end();iter++)
		//	{
		//		CFGElement element=*(iter);
		//		if(element.getKind()==CFGElement::Kind::Statement)
		//		{
		//			const Stmt* it=((CFGStmt*)&element)->getStmt();
		//			
		//			s.push(it);
		//		}
		//	}
		//}
		//else
		{
			for(CFGBlock::iterator iter=block->begin();iter!=block->end();iter++)
			{
				CFGElement element=*(iter);
				if(element.getKind()==CFGElement::Kind::Statement)
				{
					const Stmt* it=((CFGStmt*)&element)->getStmt();
					if(it==stmt)
					{
						break;
					}
					else
					{
						s.push(it);
					}
				}
			}
		}
		bool n=true;
		while(!s.empty())
		{
			const Stmt* it=s.top();
			s.pop();
			int result=throughStmt((Stmt*)it,list[i],block);
			if(result==1)
			{
				n=false;
				list.erase(list.begin()+i);
				i--;
				while(!s.empty()) s.pop();
				break;
			}
			else if(result==-1)
			{				
				reportWarning(list[i]);
				////////cerr<<"result==-1"<<endl;
				////////cerr<<"error："<<list[i].location<<endl;
				n=false;
				list.erase(list.begin()+i);
				i--;
				while(!s.empty()) s.pop();
				break;
			}
			else
			{
			}
		}
		if(n)
		{
			////////cerr<<"backForwardAnalyse:"<<endl;
			//block->dump();
			map<CFGBlock*, vector<ArraySubscript> >::iterator block_out_iter=mapToBlockOut.find(block);
		//	if(block_out_iter == mapToBlockOut.end()) {//////cerr<<"!!!Error!!!!!!!!!!!!!!mapToBlockOut not find block"<<endl;return list;}
			vector<ArraySubscript> temp=mapToBlockOut[block];
			temp.push_back(list[i]);
			mapToBlockOut.erase(block_out_iter);
			mapToBlockOut.insert(pair<CFGBlock*, vector<ArraySubscript> >(block,temp));
			for(CFGBlock::pred_iterator pred_iter=block->pred_begin();pred_iter!=block->pred_end();++pred_iter){
				CFGBlock *pred=*pred_iter;
				if(pred==NULL)continue;
				unfinished.push_back(pred);
				/* code */
			}
		}
	}
	common::printLog( "map condition over\n",common::CheckerName::arrayBound,1,*common::configure);
	////////cerr<<"end in"<<endl;
	//flow analyse
	while(unfinished.size()!=0){
		CFGBlock* block=unfinished[0];
		//cout<<block->getBlockID()<<endl;
		common::printLog( "data analyse for block "+int2string(block->getBlockID())+"\n",common::CheckerName::arrayBound,1,*common::configure);
		unfinished.erase(unfinished.begin());
		vector<ArraySubscript> temp_in;
		//vector<ArraySubscript> temp_out;
		bool T=true;
		for(CFGBlock::succ_iterator succ_iter=block->succ_begin();succ_iter!=block->succ_end();++succ_iter){
			CFGBlock* succ=*succ_iter;
			//common::printLog( "succ block ID"+int2string(succ->getBlockID())+"\n",common::CheckerName::arrayBound,1,*common::configure);
			if(succ==NULL)continue;
			vector<ArraySubscript> c_in=mapToBlockOut[succ];
			Stmt* it=(block->getTerminator().getStmt());
			if(it!=NULL)
			{
				vector<ArraySubscript> tmp_child=getScript(block,it,c_in,T);
				temp_in=Union(temp_in,tmp_child);
			}
			else
			{
				temp_in=Union(temp_in,c_in);
			}
			T=false;
		}

		if(DebugMode)
		{
			cout<<"temp in #################"<<endl;
			for(int i=0;i<temp_in.size();i++)
			{
				cout<<"\tblock:"<<temp_in[i].block->getBlockID()<<endl;
				cout<<"\tstmt:"<<printStmt(temp_in[i].stmt)<<endl;
				cout<<"\tindex:"<<printStmt(temp_in[i].index)<<endl;
				cout<<"\tloc:"<<temp_in[i].location<<endl;
				vector<AtomicAPInt> temp_list=temp_in[i].condition;
				for(int j=0;j<temp_list.size();j++)
					cout<<"\tconstrain:"<<printStmt(temp_list[j].lhs)<<"\t"<<temp_list[j].rhs<<endl;
			}
		}
		
		common::printLog( "after get in_state from succ blocks, temp_in size:"+int2string(temp_in.size())+"\n",common::CheckerName::arrayBound,0,*common::configure);
		std::vector<ArraySubscript> temp_out=flowThrogth(block,temp_in);
		common::printLog( "after analyse stmt in block, temp_out size:"+int2string(temp_out.size())+"\n",common::CheckerName::arrayBound,0,*common::configure);
		std::vector<ArraySubscript> orign_out=mapToBlockOut[block];
		
		if(DebugMode)
		{
			cout<<"temp out #################"<<endl;
			for(int i=0;i<temp_out.size();i++)
			{
				cout<<"\tblock:"<<temp_out[i].block->getBlockID()<<endl;
				cout<<"\tstmt:"<<printStmt(temp_out[i].stmt)<<endl;
				cout<<"\tindex:"<<printStmt(temp_out[i].index)<<endl;
				cout<<"\tloc:"<<temp_out[i].location<<endl;
				vector<AtomicAPInt> temp_list=temp_out[i].condition;
				for(int j=0;j<temp_list.size();j++)
					cout<<"\tconstrain:"<<printStmt(temp_list[j].lhs)<<"\t"<<temp_list[j].rhs<<endl;
			}
		}
		/*//////cerr<<"orign out #################"<<endl;
		for(int i=0;i<orign_out.size();i++)
		{
			//////cerr<<"\tblock:"<<orign_out[i].block->getBlockID()<<endl;
			////////cerr<<"\tstmt:"<<printStmt(orign_out[i].stmt)<<endl;
			//////cerr<<"\tindex:"<<printStmt(orign_out[i].index)<<endl;
			//////cerr<<"\tloc:"<<orign_out[i].location<<endl;
			vector<AtomicAPInt> temp_list=orign_out[i].condition;
			for(int j=0;j<temp_list.size();j++)
				//////cerr<<"\tconstrain:"<<printStmt(temp_list[j].lhs)<<"\t"<<temp_list[j].rhs<<endl;
		}*/
		
		if(!isEuql(temp_out,orign_out))
		{
			////////cerr<<"not Equal"<<endl;
			for(CFGBlock::pred_iterator pred_iter=block->pred_begin();pred_iter!=block->pred_end();++pred_iter){
				CFGBlock *pred=*pred_iter;
				if(pred==NULL)continue;
				unfinished.push_back(pred);
			}

				map<CFGBlock*, vector<ArraySubscript> >::iterator block_out_iter=mapToBlockOut.find(block);
				
				mapToBlockOut.erase(block_out_iter);
				
				mapToBlockOut.insert(pair<CFGBlock*, vector<ArraySubscript> >(block,Union(temp_out,orign_out)));
			
		}
		common::printLog( "update condition in block\n",common::CheckerName::arrayBound,1,*common::configure);
	}
	common::printLog( "End backForwordAnalyse:\n",common::CheckerName::arrayBound,3,*common::configure);
	return mapToBlockOut[Entry];
}



vector<ArraySubscript> ArrayOutOfBoundsCheckerV4::flowThrogth(CFGBlock* block,vector<ArraySubscript> list)
{
	//block->dump();
	for (unsigned i = 0; i < list.size(); ++i)
	{
		/* code */

		//CFGBlock* block=list[i].block;
		stack<const Stmt*> s;
		for(CFGBlock::iterator iter=block->begin();iter!=block->end();iter++)
		{
			CFGElement element=*(iter);
			//cerr<<element.getKind()<<endl;
			if(element.getKind()==CFGElement::Kind::Statement)
			{
				const Stmt* it=((CFGStmt*)&element)->getStmt();	
				s.push(it);
				//cerr<<block->getBlockID()<<endl;
				//cerr<<printStmt((Stmt*)it);
				//it->dump();
			}
		}
		bool n=true;
		while(!s.empty())
		{
			const Stmt* it=s.top();
			s.pop();
			int result=throughStmt((Stmt*)it,list[i],block);
			if(result==1)
			{
				n=false;
				list.erase(list.begin()+i);
				i--;
				while(!s.empty()) s.pop();
				break;
			}
			else if(result==-1)
			{
				////////cerr<<"error："<<list[i].location<<endl;
				reportWarning(list[i]);
				n=false;
				list.erase(list.begin()+i);
				i--;
				while(!s.empty()) s.pop();
				break;
			}
			else
			{
			}
		}

	}
	return list;
}
//vector<ArraySubscript> ArrayOutOfBoundsCheckerV4::getScript(Stmt* stmt,vector<ArraySubscript> input,bool T)
//{
//	////////cerr<<"getScript input size:"<<input.size()<<endl;
//	Expr* cond=NULL;
//	switch(stmt->getStmtClass())
//	{
//		case Stmt::ForStmtClass:
//			{
//				cond=((ForStmt*)stmt)->getCond();				
//				break;
//			}
//		case Stmt::IfStmtClass:
//			{
//				cond=((IfStmt*)stmt)->getCond();				
//				break;
//			}
//		case Stmt::WhileStmtClass:
//			{
//				cond=((WhileStmt*)stmt)->getCond();				
//				break;
//			}
//		//case Stmt::DoStmtClass:
//		//	Stmt* cond=((DoStmt*)it)->getCond();
//		//	if(cond!=NULL)
//		//	{
//		//		return checkConditionStmt(cond,input,T);
//		//	}
//		//	break;
//		default:return input;
//	}
//	if(cond!=NULL)
//	{
//		if(SimpleCheckEnable){
//			vector<ArraySubscript> result;
//			////////cerr<<"input size:"<<input.size()<<endl;
//			for(unsigned i=0;i<input.size();i++)
//			{				
//				//input[i].index->dump();
//				////////cerr<<"input[i].isLoopBoundChecking:"<<input[i].isLoopBoundChecking<<endl;
//				vector<AtomicAPInt> cons=input[i].condition;
//				if(input[i].isLoopBoundChecking){
//					////////cerr<<printStmt(input[i].index)<<" isLoopBoundChecking"<<endl;
//					for(unsigned j=0;j<cons.size();j++)
//					{
//						if(checkConditionStmt(cond,input[i],cons[j],T))
//						{
//							cons.erase(cons.begin()+j);
//							j--;
//						}
//					}
//					if(cons.size()!=0)
//					{
//						input[i].condition=cons;
//						result.push_back(input[i]);	
//					}
//				}
//				else{
//					for(unsigned j=0;j<cons.size();j++)
//					{
//						if(checkConditionStmtHasIdx(cond,input[i],cons[j].getLHS(),T))
//						{
//							cons.erase(cons.begin()+j);
//							j--;
//						}
//					}
//					if(cons.size()!=0)
//					{
//						input[i].condition=cons;
//						result.push_back(input[i]);	
//					}
//				}
//			}
//			return result;
//		}
//		else{			
//			vector<ArraySubscript> result;
//			for(unsigned i=0;i<input.size();i++)
//			{
//				vector<AtomicAPInt> cons=input[i].condition;
//				for(unsigned j=0;j<cons.size();j++)
//				{
//					if(checkConditionStmt(cond,input[i],cons[j],T))
//					{
//						cons.erase(cons.begin()+j);
//						j--;
//					}
//				}
//				if(cons.size()!=0)
//				{
//					input[i].condition=cons;
//					result.push_back(input[i]);	
//				}
//			}
//			return result;
//		}
//	}
//	return input;
//}
vector<ArraySubscript> ArrayOutOfBoundsCheckerV4::ReplaceIdxLoopBound(CFGBlock* block,Stmt* cond,Expr* expr,ArraySubscript & input){
	//this function will replace index with loop bound when encountering a for or while statement
	common::printLog( "ReplaceIdxLoopBound,index:"+printStmt(input.condition[0].getLHS())+"\n",common::CheckerName::arrayBound,0,*common::configure);
	common::printLog( "ReplaceIdxLoopBound,condition:"+printStmt(expr)+"\n",common::CheckerName::arrayBound,0,*common::configure);
	vector<ArraySubscript> inputVec;
	if(BinaryOperator * it=dyn_cast<BinaryOperator>(expr)){
		switch(it->getOpcode())
		{
		case clang::BinaryOperatorKind::BO_LAnd:
			{
				vector<ArraySubscript> input1=ReplaceIdxLoopBound(block,cond,it->getLHS()->IgnoreParenCasts()->IgnoreImpCasts(), input);
				vector<ArraySubscript> input2=ReplaceIdxLoopBound(block,cond,it->getRHS()->IgnoreParenCasts()->IgnoreImpCasts(), input);				
				for(auto i1:input1){
					if(i1.condition.size()!=0){
						inputVec.push_back(i1);
					}
				}
				for(auto i2:input2){
					if(i2.condition.size()!=0){
						inputVec.push_back(i2);
					}
				}
				break;
			}
		case clang::BinaryOperatorKind::BO_LOr:
			{
				vector<ArraySubscript> input1=ReplaceIdxLoopBound(block,cond,it->getLHS()->IgnoreParenCasts()->IgnoreImpCasts(), input);
				vector<ArraySubscript> input2=ReplaceIdxLoopBound(block,cond,it->getRHS()->IgnoreParenCasts()->IgnoreImpCasts(), input);
				for(auto i1:input1){
					if(i1.condition.size()!=0){
						inputVec.push_back(i1);
					}
				}
				for(auto i2:input2){
					if(i2.condition.size()!=0){
						inputVec.push_back(i2);
					}
				}
				break;
			}
		default:
			{
				ArraySubscript inputtmp;
				Expr* lhs=it->getLHS()->IgnoreImpCasts()->IgnoreImpCasts()->IgnoreParenCasts()->IgnoreImpCasts();
				Expr* rhs=it->getRHS()->IgnoreImpCasts()->IgnoreImpCasts()->IgnoreParenCasts()->IgnoreImpCasts();
				for(auto con:input.condition){
					if(con.op==clang::BinaryOperatorKind::BO_LT)
					{
						//common::printLog( "ReplaceIdxLoopBound, index:"+printStmt(con.getLHS())+"\n",common::CheckerName::arrayBound,0,*common::configure);
						VarDecl* idx=getVarDecl(con.getLHS());
						//idx->dump();
						VarDecl* lhsV=getVarDecl(lhs);
						VarDecl* rhsV=getVarDecl(rhs);
						bool idxEqlhs=false;
						bool idxEqrhs=false;
						if(SimpleExprCheckOnly){
							 if(dyn_cast<DeclRefExpr>(rhs) || dyn_cast<MemberExpr>(rhs) || dyn_cast<ArraySubscriptExpr>(rhs)) {
							 }
							 else{
							 	common::printLog( "SimpleExprCheckOnly, not DeclRefExpr:"+printStmt(rhs)+",return\n",common::CheckerName::arrayBound,0,*common::configure);
							 	return inputVec;
							 }
							if(idx==NULL||lhsV==NULL||rhsV==NULL){
								idxEqlhs=printStmt(con.getLHS())==printStmt(lhs);
								idxEqrhs=printStmt(con.getLHS())==printStmt(rhs);
							}
							else{
								idxEqlhs=idx==lhsV;
								idxEqrhs=idx==rhsV;
							}
						}else{
							idxEqlhs=printStmt(con.getLHS())==printStmt(lhs);
							idxEqrhs=printStmt(con.getLHS())==printStmt(rhs);
						}
						if(it->getOpcode()==BinaryOperatorKind::BO_GT||it->getOpcode()==BinaryOperatorKind::BO_GE){
							//n>i,i<size--n<=size
							//n>=i,i<size--n<size
							if(idxEqrhs){
								common::printLog( "ReplaceIdxLoopBound, index==rhs:"+printStmt(rhs)+"\n",common::CheckerName::arrayBound,0,*common::configure);								
								//common::printLog( "FuncNow:"+FuncNow->getNameAsString()+"\n",common::CheckerName::arrayBound,0,*common::configure);
								//common::printLog( "block id:"+int2string(block->getBlockID())+"\n",common::CheckerName::arrayBound,0,*common::configure);
								//common::printLog( "rhs:"+printStmt(rhs)+"\n",common::CheckerName::arrayBound,0,*common::configure);
								if(isTainted(FuncNow,block,cond,lhs))
								{								
									//input.changeIndex(rhs);
									//input.condition.clear();
									//vector<AtomicAPInt>(input.condition).swap(input.condition);
									common::printLog( "ReplaceIdxLoopBound,"+printStmt(lhs)+" isTainted\n",common::CheckerName::arrayBound,0,*common::configure);
									AtomicAPInt newCon;
									newCon.lhs=lhs;
									newCon.op=con.op;
									newCon.rhs=con.rhs;
									if(it->getOpcode()==BinaryOperatorKind::BO_GT) newCon.rhs=con.rhs+1;
									vector<AtomicAPInt> tmp;
									tmp.push_back(newCon);
									//inputtmp=input;
									inputtmp.condition=tmp;
									inputtmp.isLoopBoundChecking=true;
									inputtmp.block=block;
									inputtmp.stmt=cond;
									SourceManager *sm;
									sm = &(FuncNow->getASTContext().getSourceManager()); 
									string loc = rhs->getLocStart().printToString(*sm);
									//common::printLog( "use FuncNow,get location end\n",common::CheckerName::arrayBound,0,*common::configure);
									inputtmp.location=loc;
									inputtmp.ID=input.ID;
									inputtmp.orignIndex=lhs;
									inputtmp.indexCnt=input.indexCnt;
									inputtmp.arrayName=input.arrayName;
									inputtmp.arrayExpr=input.arrayExpr;
									inputtmp.index=lhs;
									inputtmp.arrayIdx=input.arrayIdx;
									inputtmp.func=input.func;
									inputtmp.originalLocation=input.originalLocation;
									//input.condition=tmp;
									inputVec.push_back(inputtmp);
								}
								else{
									common::printLog( "ReplaceIdxLoopBound,bound "+printStmt(lhs)+" not tainted\n",common::CheckerName::arrayBound,0,*common::configure);
								}
							}
						}
						if(it->getOpcode()==BinaryOperatorKind::BO_LT||it->getOpcode()==BinaryOperatorKind::BO_LE){
							//i<n,i<size--n<size+1
							//i<=n,i<size--n<size
							if(idxEqlhs){
								common::printLog( "ReplaceIdxLoopBound, index==lhs:"+printStmt(lhs)+"\n",common::CheckerName::arrayBound,0,*common::configure);								
								common::printLog( "FuncNow:"+FuncNow->getNameAsString()+"\n",common::CheckerName::arrayBound,0,*common::configure);
								common::printLog( "block id:"+int2string(block->getBlockID())+"\n",common::CheckerName::arrayBound,0,*common::configure);
								common::printLog( "rhs:"+printStmt(rhs)+"\n",common::CheckerName::arrayBound,0,*common::configure);
								if(isTainted(FuncNow,block,cond,rhs))
								{								
									//input.changeIndex(rhs);
									//input.condition.clear();
									//vector<AtomicAPInt>(input.condition).swap(input.condition);
									common::printLog( "ReplaceIdxLoopBound,"+printStmt(rhs)+" isTainted\n",common::CheckerName::arrayBound,0,*common::configure);
									AtomicAPInt newCon;
									newCon.lhs=rhs;
									newCon.op=con.op;
									newCon.rhs=con.rhs;
									if(it->getOpcode()==BinaryOperatorKind::BO_LT) newCon.rhs=con.rhs+1;
									vector<AtomicAPInt> tmp;
									tmp.push_back(newCon);
									//inputtmp=input;
									inputtmp.condition=tmp;
									inputtmp.isLoopBoundChecking=true;
									inputtmp.block=block;
									inputtmp.stmt=cond;
									SourceManager *sm;
									sm = &(FuncNow->getASTContext().getSourceManager()); 
									string loc = rhs->getLocStart().printToString(*sm);
									//common::printLog( "use FuncNow,get location end\n",common::CheckerName::arrayBound,0,*common::configure);
									inputtmp.location=loc;
									inputtmp.ID=input.ID;
									inputtmp.orignIndex=rhs;
									inputtmp.indexCnt=input.indexCnt;
									inputtmp.arrayName=input.arrayName;
									inputtmp.arrayExpr=input.arrayExpr;
									inputtmp.index=rhs;
									inputtmp.arrayIdx=input.arrayIdx;
									inputtmp.func=input.func;
									inputtmp.originalLocation=input.originalLocation;
									//input.condition=tmp;
									inputVec.push_back(inputtmp);
								}
								else{
									common::printLog( "ReplaceIdxLoopBound,bound "+printStmt(rhs)+" not tainted\n",common::CheckerName::arrayBound,0,*common::configure);
								}
							}
						}
					}
				}
			}
		}
	}
	
	sameID[input.ID]=inputVec;
	return inputVec;
}
vector<ArraySubscript> ArrayOutOfBoundsCheckerV4::getScript(CFGBlock* block,Stmt* stmt,vector<ArraySubscript> input,bool T)
{
	//this function deal with if, for, while statement
	common::printLog( "analyse conditon from child from condition stmt\n",common::CheckerName::arrayBound,1,*common::configure);
	//if(SimpleCheckEnable){

	//	////////cerr<<"getScript input size:"<<input.size()<<endl;
	//	Expr* cond=NULL;
	//	if(stmt->getStmtClass() == Stmt::IfStmtClass){		
	//		cond=((IfStmt*)stmt)->getCond();							
	//	}
	//	else{
	//		return input;
	//	}
	//	if(cond!=NULL)
	//	{				
	//		vector<ArraySubscript> result;
	//		////////cerr<<"input size:"<<input.size()<<endl;
	//		for(unsigned i=0;i<input.size();i++)
	//		{				
	//			vector<AtomicAPInt> cons=input[i].condition;
	//			for(unsigned j=0;j<cons.size();j++)
	//			{
	//				if(checkConditionStmtHasIdx(cond,input[i],cons[j].getLHS(),T))
	//				{
	//					cons.erase(cons.begin()+j);
	//					j--;
	//				}
	//			}
	//			if(cons.size()!=0)
	//			{
	//				input[i].condition=cons;
	//				result.push_back(input[i]);	
	//			}
	//		}
	//		return result;
	//	}
	//	return input;
	//}
	//else
	{
		////////cerr<<"getScript input size:"<<input.size()<<endl;
		Expr* cond=NULL;
		switch(stmt->getStmtClass())
		{
			case Stmt::ForStmtClass:
				{
					cond=((ForStmt*)stmt)->getCond();
					if(cond!=NULL)
					{	
						common::printLog( "for condition: "+printStmt(cond)+"\n",common::CheckerName::arrayBound,0,*common::configure);			
						vector<ArraySubscript> result;
						for(unsigned i=0;i<input.size();i++)
						{
							vector<AtomicAPInt> cons=input[i].condition;
							for(unsigned j=0;j<cons.size();j++)
							{
								int r;
								if(SimpleCheckEnable){
									r=checkConditionStmtHasIdx(block,stmt,cond,input[i],cons[j],T,true);
									//the last argument is true, we check whehter index<untainted, if index<taint, return false,
									//to avoid for(i<m) making i<size checked.
								}
								else{r=checkConditionStmt(cond,input[i],cons[j],T);}
								common::printLog( "checkConditionStmt result: "+int2string(r)+"\n",common::CheckerName::arrayBound,0,*common::configure);	
								if(r)
								{
									cons.erase(cons.begin()+j);
									j--;
								}
								if(!r){
									//common::printLog( "checkConditionStmt false\n",common::CheckerName::arrayBound,0,*common::configure);
									vector<ArraySubscript> newInput=ReplaceIdxLoopBound(block,cond,cond,input[i]);
									common::printLog( "-------ReplaceIdxLoopBound end------\n",common::CheckerName::arrayBound,0,*common::configure);
									common::printLog( "After ReplaceIdxLoopBound:\n",common::CheckerName::arrayBound,0,*common::configure);
									for(auto in:newInput){
										if(in.condition.size()!=0){
											vector<AtomicAPInt> cons2=in.condition;
											for(unsigned j=0;j<cons2.size();j++)
											{										
												common::printLog( printStmt(cons2[j].getLHS())+" op "+int2string(cons2[j].getRHS())+"\n",common::CheckerName::arrayBound,0,*common::configure);	
											}
											cons.clear();
											result.push_back(in);
										}
									}
									break;
								}
							}
							if(cons.size()!=0)
							{
								input[i].condition=cons;
								result.push_back(input[i]);	
							}
						}
						return result;
		
					}
					break;
				}
			case Stmt::IfStmtClass:
				{
					cond=((IfStmt*)stmt)->getCond();	
					common::printLog( "if condition: "+printStmt(cond)+"\n",common::CheckerName::arrayBound,0,*common::configure);			
					vector<ArraySubscript> result;
					for(unsigned i=0;i<input.size();i++)
					{
						common::printLog( "input i: "+int2string(i)+"\n",common::CheckerName::arrayBound,0,*common::configure);
						common::printLog( "input ID: "+int2string(input[i].ID)+"\n",common::CheckerName::arrayBound,0,*common::configure);
						vector<AtomicAPInt> cons=input[i].condition;
						for(unsigned j=0;j<cons.size();j++)
						{
							common::printLog( "cons j: "+int2string(j)+"\n",common::CheckerName::arrayBound,0,*common::configure);
							int r;
							if(SimpleCheckEnable){r=checkConditionStmtHasIdx(block,stmt,cond,input[i],cons[j],T,false);}
							else{r=checkConditionStmt(cond,input[i],cons[j],T);}
							if(r)
							{
								common::printLog( "checkConditionStmt true,find if check: "+printStmt(cond)+", "+int2string(T)+"\n",common::CheckerName::arrayBound,0,*common::configure);
								cons.erase(cons.begin()+j);
								j--;
							}
						}
						if(cons.size()!=0)
						{
							input[i].condition=cons;
							result.push_back(input[i]);	
						}
					}
					return result;
					break;
				}
			case Stmt::WhileStmtClass:
				{
					cond=((WhileStmt*)stmt)->getCond();	
					if(cond!=NULL)
					{	
						common::printLog( "while condition: "+printStmt(cond)+"\n",common::CheckerName::arrayBound,0,*common::configure);			
						vector<ArraySubscript> result;
						for(unsigned i=0;i<input.size();i++)
						{
							vector<AtomicAPInt> cons=input[i].condition;
							for(unsigned j=0;j<cons.size();j++)
							{
								int r;
								if(SimpleCheckEnable){
									r=checkConditionStmtHasIdx(block,stmt,cond,input[i],cons[j],T,true);
									//the last argument is true, we check whehter index<untainted, if index<taint, return false,
									//to avoid for(i<m) making i<size checked.
								}
								else{r=checkConditionStmt(cond,input[i],cons[j],T);}								
								common::printLog( "checkConditionStmt result: "+int2string(r)+"\n",common::CheckerName::arrayBound,0,*common::configure);	
								if(r)
								{
									cons.erase(cons.begin()+j);
									j--;
								}
								if(!r){
									common::printLog( "checkConditionStmt false\n",common::CheckerName::arrayBound,0,*common::configure);
									vector<ArraySubscript> newInput=ReplaceIdxLoopBound(block,cond,cond,input[i]);
									common::printLog( "-------ReplaceIdxLoopBound end------\n",common::CheckerName::arrayBound,0,*common::configure);
									common::printLog( "After ReplaceIdxLoopBound:\n",common::CheckerName::arrayBound,0,*common::configure);
									for(auto in:newInput){
										if(in.condition.size()!=0){
											vector<AtomicAPInt> cons2=in.condition;
											for(unsigned j=0;j<cons2.size();j++)
											{										
												common::printLog( printStmt(cons2[j].getLHS())+" op "+int2string(cons2[j].getRHS())+"\n",common::CheckerName::arrayBound,0,*common::configure);	
											}
											cons.clear();
											result.push_back(in);
										}
									}
									break;
								}
							}
							if(cons.size()!=0)
							{
								input[i].condition=cons;
								result.push_back(input[i]);	
							}
						}
						return result;
		
					}
					break;
				}
			//case Stmt::DoStmtClass:
			//	Stmt* cond=((DoStmt*)it)->getCond();
			//	if(cond!=NULL)
			//	{
			//		return checkConditionStmt(cond,input,T);
			//	}
			//	break;
			default:return input;
		}
		// if(stmt->getStmtClass() == Stmt::IfStmtClass){		
		// 	cond=((IfStmt*)stmt)->getCond();							
		// }
		// else{
		// 	return input;
		// }
		/*if(cond!=NULL)
		{	
			common::printLog( "if for while condition: "+printStmt(cond)+"\n",common::CheckerName::arrayBound,1,*common::configure);			
			vector<ArraySubscript> result;
			for(unsigned i=0;i<input.size();i++)
			{
				vector<AtomicAPInt> cons=input[i].condition;
				for(unsigned j=0;j<cons.size();j++)
				{
					if(checkConditionStmt(cond,input[i],cons[j],T))
					{
						cons.erase(cons.begin()+j);
						j--;
					}
				}
				if(cons.size()!=0)
				{
					input[i].condition=cons;
					result.push_back(input[i]);	
				}
			}
			return result;
		
		}*/
		return input;
	}
	
}

void analyseIndex(Expr* index,vector<PNode>& list)
{
	if(BinaryOperator* id = dyn_cast<BinaryOperator>(index) )
	{
		if(id->getOpcode()== clang::BinaryOperatorKind::BO_Add)
		{
			analyseIndex(id->getLHS(),list);
			analyseIndex(id->getRHS(),list);
		}
		else if(id->getOpcode()== clang::BinaryOperatorKind::BO_Sub)
		{
			analyseIndex(id->getLHS(),list);
			vector<PNode> tmp;
			analyseIndex(id->getRHS(),tmp);	
			for(PNode tt : tmp)
			{
				tt.terms=-tt.terms;
				list.push_back(tt);
			}
		}
		else if(id->getOpcode()== clang::BinaryOperatorKind::BO_Mul)
		{
			//lhs
			if(IntegerLiteral *ILE = dyn_cast<IntegerLiteral>(id->getLHS())) 
			{
				uint64_t value=ILE->getValue().getLimitedValue();
				if(value!=0)
				{
					PNode n(printStmt(id->getRHS()),static_cast<double>(value));
					list.push_back(n);
				}
				
				return;
			}
			else if(FloatingLiteral *ILE = dyn_cast<FloatingLiteral>(id->getLHS()))//-ImplicitCastExpr
			{
				double value=ILE->getValue().convertToDouble();
				if(value!=0)
				{
					PNode n(printStmt(id->getRHS()),value);
					list.push_back(n);
				}
				return;
			}
			else if(IntegerLiteral *ILE = dyn_cast<IntegerLiteral>(id->getRHS())) 
			{
				uint64_t value=ILE->getValue().getLimitedValue();
				if(value!=0)
				{
					PNode n(printStmt(id->getLHS()),static_cast<double>(value));
					list.push_back(n);
				}
				return;
			}
			else if(FloatingLiteral *ILE = dyn_cast<FloatingLiteral>(id->getRHS()))//-ImplicitCastExpr
			{
				double value=ILE->getValue().convertToDouble();
				if(value!=0)
				{
					PNode n(printStmt(id->getLHS()),value);
					list.push_back(n);
				}
				return;
			}
			else
			{
				PNode n(printStmt(index),1.0);
				list.push_back(n);
			}
		}
		else
		{
			PNode n(printStmt(index),1.0);
			list.push_back(n);
		}
	}
	else
	{
		PNode n(printStmt(index),1.0);
		list.push_back(n);
	}
}

void analyseStmt(Expr* g,Expr* l,vector<PNode>& list)
{
	//
	analyseIndex(g,list);
	vector<PNode> tmp;
	analyseIndex(l,tmp);	
	for(PNode tt : tmp)
	{
		tt.terms=-tt.terms;
		list.push_back(tt);
	}
}

bool isNum(string s)
{
	stringstream sin(s);
	double t;
	char p;
	if(!(sin>>t))
		return false;
	if(sin>>p)
		return false;
	else
		return true;
}

double str2num(string s)
{
	 stringstream ss;
	 ss<<s;

	 double i;
	 ss>>i;
	 return i;
}

bool checkComposedExpr(BinaryOperator* it,vector<PNode>& list,ArraySubscript input,AtomicAPInt con,bool flag)
{
	//////cerr<<"**************************************************"<<endl;
	//		////cerr<<printStmt(input.index)<<endl;
	//		////cerr<<printStmt(it)<<endl;

	std::vector<PNode> tmp;
	if(it->getOpcode()==clang::BinaryOperatorKind::BO_LT||it->getOpcode()==clang::BinaryOperatorKind::BO_LE)
	{
		analyseStmt(it->getRHS(),it->getLHS(),tmp);
	}
	else if(it->getOpcode()==clang::BinaryOperatorKind::BO_GT||it->getOpcode()==clang::BinaryOperatorKind::BO_GE)
	{
		analyseStmt(it->getLHS(),it->getRHS(),tmp);
	}
	//////cerr<<"analyse stmt"<<endl;
	//////cerr<<printStmt(it)<<endl;
	if(con.op==clang::BinaryOperatorKind::BO_LT)
	{
		//////cerr<<"lt size"<<endl;
		if(flag)
		{
			double val=0.0;
			for(PNode node : list)
			{
				for(unsigned i=0;i<tmp.size();i++)
				{
					
					PNode c=tmp[i];
					//////cerr<<"compare "<<node.name<<"    "<<c.name<<endl;
					// Terms t1(node.name);
					// Terms t2(c.name);
					// t1.print();
					// t2.print();
					//if(t1.isEqual(t2))
					if(node.name==c.name)
					{
						//////cerr<<"find"<<endl;
						if(val==0.0)
							val=-1*c.terms/node.terms;
						if(c.terms!=-1*node.terms*val)return false;
						tmp.erase(tmp.begin()+i);
						break;
					}
				}
			}
			//////cerr<<"finish sub"<<endl;
			if(tmp.size()==0)return false;
			double tmp_val=0.0;
			for(PNode c : tmp)
			{
				if(!isNum(c.name))
				{
					return false;
				}
				else
				{
					tmp_val+=c.terms*str2num(c.name);
				}
			}
			uint64_t value = con.rhs;

			//////cerr<<tmp_val<<"  "<<value<<endl;
			if(tmp_val<=value)return true;
			else return false;
		}
		else
		{
			double val=0.0;
			for(PNode node : list)
			{
				for(unsigned i=0;i<tmp.size();i++)
				{
					
					PNode c=tmp[i];
					//////cerr<<"compare "<<node.name<<"    "<<c.name<<endl;
					// Terms t1(node.name);
					// Terms t2(c.name);
					// if(t1.isEqual(t2))
					if(node.name==c.name)
					{
						if(val==0.0)
							val=c.terms/node.terms;
						if(c.terms!=node.terms*val)return false;
						tmp.erase(tmp.begin()+i);
						break;
					}
				}
			}
			//////cerr<<"finish sub"<<endl;
			if(tmp.size()==0)return false;
			double tmp_val=0.0;
			for(PNode c : tmp)
			{
				if(!isNum(c.name))
				{
					return false;
				}
				else
				{
					tmp_val+=-1*c.terms*str2num(c.name);
				}
			}
			uint64_t value = con.rhs;
			if(tmp_val<=value)return true;
			else return false;
		}
	}
	else if(con.op==clang::BinaryOperatorKind::BO_GE)
	{
		//////cerr<<"gt 0"<<endl;
		if(flag)
		{
			double val=0.0;
			for(PNode node : list)
			{
				for(unsigned i=0;i<tmp.size();i++)
				{
					
					PNode c=tmp[i];
					//////cerr<<"compare "<<node.name<<"    "<<c.name<<endl;
					// Terms t1(node.name);
					// Terms t2(c.name);
					// if(t1.isEqual(t2))
					if(node.name==c.name)
					{
						if(val==0.0)
							val=c.terms/node.terms;
						if(c.terms!=node.terms*val)return false;
						tmp.erase(tmp.begin()+i);
						break;
					}
				}
			}
			//////cerr<<"finish sub"<<endl;
			if(tmp.size()==0)return true;
			double tmp_val=0.0;
			for(PNode c : tmp)
			{
				if(!isNum(c.name))
				{
					return false;
				}
				else
				{
					tmp_val+=c.terms*str2num(c.name);
				}
			}
			if(tmp_val<=0)return true;
			else return false;
		}
		else
		{
			double val=0.0;
			for(PNode node : list)
			{
				for(unsigned i=0;i<tmp.size();i++)
				{
					
					PNode c=tmp[i];
					//////cerr<<"compare "<<node.name<<"    "<<c.name<<endl;
					// Terms t1(node.name);
					// Terms t2(c.name);
					// if(t1.isEqual(t2))
					if(node.name==c.name)
					{
						if(val==0.0)
							val=-1*c.terms/node.terms;
						if(c.terms!=-1*node.terms*val)return false;
						tmp.erase(tmp.begin()+i);
						break;
					}
				}
			}
			//////cerr<<"finish sub"<<endl;
			if(tmp.size()==0)return true;
			double tmp_val=0.0;
			for(PNode c : tmp)
			{
				if(!isNum(c.name))
				{
					return false;
				}
				else
				{
					tmp_val+=c.terms*str2num(c.name);
				}
			}
			//uint64_t value = con.rhs;

			//////cerr<<tmp_val<<"  "<<value<<endl;
			if(tmp_val<=0)return true;
			else return false;
		}
	}
	return false;
}

bool ArrayOutOfBoundsCheckerV4::checkConditionStmt(Expr* stmt,ArraySubscript input,AtomicAPInt con,bool flag)//true : remove this cons
{
	// cerr<<printStmt(stmt)<<"   "<<flag<<endl;
	// stmt->dump();
	// cerr<<printStmt(con.lhs)<<" op "<<con.rhs<<endl;
	common::printLog( "checkConditionStmt AtomicAPInt:"+printStmt(con.lhs)+" op "+int2string(con.rhs)+"\n",common::CheckerName::arrayBound,0,*common::configure);
	if(CallExpr* call = dyn_cast<CallExpr>(stmt->IgnoreImpCasts()->IgnoreParenCasts()) )
	{
		FunctionDecl* fun=call->getDirectCallee();
		if(fun==nullptr) return false;
		DeclarationName DeclName = fun->getNameInfo().getName();
		std::string FuncName = DeclName.getAsString();
		//cerr<<FuncName<<endl;
		if(FuncName=="__builtin_expect")
		{
			return checkConditionStmt(call->getArg(0)->IgnoreImpCasts()->IgnoreParenCasts(),input,con,flag);

		}
	}
	bool result=false;
	if(BinaryOperator * it=dyn_cast<BinaryOperator>(stmt))
	{
		
		switch(it->getOpcode())
		{
		case clang::BinaryOperatorKind::BO_LAnd:
			{
				bool temp_lhs=checkConditionStmt(it->getLHS()->IgnoreParenCasts()->IgnoreImpCasts(),input,con,flag);
				bool temp_rhs=checkConditionStmt(it->getRHS()->IgnoreParenCasts()->IgnoreImpCasts(),input,con,flag);
				if(flag)
				{
					result=temp_lhs || temp_rhs;
				}
				else
				{
					result=temp_lhs && temp_rhs;
				}

				break;
			}
		case clang::BinaryOperatorKind::BO_LOr:
			{
				bool temp_lhs=checkConditionStmt(it->getLHS()->IgnoreParenCasts()->IgnoreImpCasts(),input,con,flag);
				bool temp_rhs=checkConditionStmt(it->getRHS()->IgnoreParenCasts()->IgnoreImpCasts(),input,con,flag);
				if(flag)
				{
					result=temp_lhs && temp_rhs;
				}
				else
				{
					result=temp_lhs || temp_rhs;
				}
				break;
			}
		default:
			{
				Expr* lhs=it->getLHS()->IgnoreImpCasts()->IgnoreImpCasts()->IgnoreParenCasts()->IgnoreImpCasts();
				Expr* rhs=it->getRHS()->IgnoreImpCasts()->IgnoreImpCasts()->IgnoreParenCasts()->IgnoreImpCasts();
				if(con.op==clang::BinaryOperatorKind::BO_LT)
				{
					string index=printStmt(con.lhs->IgnoreImpCasts()->IgnoreParenCasts());
					if(flag)
					{
						uint64_t value = con.rhs;
						common::printLog( int2string(flag)+"path\n",common::CheckerName::arrayBound,0,*common::configure);
						common::printLog( "checkConditionStmt rhs:"+printStmt(rhs)+"\n",common::CheckerName::arrayBound,0,*common::configure);
						//cerr<<"*****************\n"<<printStmt(rhs)<<"    "<<value<<endl;
						//cerr<<"*****************\n"<<printStmt(lhs)<<"    "<<index<<endl;
						if(index==printStmt(lhs))
						{
							//cerr<<"Index match"<<endl;
							common::printLog( "Index match\n",common::CheckerName::arrayBound,0,*common::configure);
							if(it->getOpcode()==clang::BinaryOperatorKind::BO_LT)
							{
								result=CheckConditionExpr(rhs,"<",value);
								if(printStmt(rhs).find("sizeof")!=std::string::npos)
								{									
									common::printLog( printStmt(lhs)+"< "+printStmt(rhs)+"\n",common::CheckerName::arrayBound,0,*common::configure);									
									result=1;
								}
							}
							else if(it->getOpcode()==clang::BinaryOperatorKind::BO_LE)
							{
								result=CheckConditionExpr(rhs,"<",value);//
								if(printStmt(rhs).find("sizeof")!=std::string::npos)
								{									
									common::printLog( printStmt(lhs)+"<="+printStmt(rhs)+"\n",common::CheckerName::arrayBound,0,*common::configure);									
									result=1;
								}
							}
						}
						else if(index==printStmt(rhs))
						{
							if(it->getOpcode()==clang::BinaryOperatorKind::BO_GT)
							{
								result=CheckConditionExpr(lhs,"<",value);
								if(printStmt(lhs).find("sizeof")!=std::string::npos)
								{									
									common::printLog( printStmt(lhs)+">"+printStmt(rhs)+"\n",common::CheckerName::arrayBound,0,*common::configure);									
									result=1;
								}
							}
							else if(it->getOpcode()==clang::BinaryOperatorKind::BO_GE)
							{
								result=CheckConditionExpr(lhs,"<",value);//	
								if(printStmt(lhs).find("sizeof")!=std::string::npos)
								{									
									common::printLog( printStmt(lhs)+">="+printStmt(rhs)+"\n",common::CheckerName::arrayBound,0,*common::configure);									
									result=1;
								}
							}
						}
					}
					else
					{
						common::printLog( int2string(flag)+"path\n",common::CheckerName::arrayBound,0,*common::configure);
						uint64_t value = con.rhs;
						// cerr<<"*****************\n"<<printStmt(rhs)<<"    "<<value<<endl;
						// cerr<<"*****************\n"<<printStmt(lhs)<<"    "<<index<<endl;
						if(index==printStmt(lhs))
						{
							if(it->getOpcode()==clang::BinaryOperatorKind::BO_GE)
							{
								result=CheckConditionExpr(rhs,"<",value);
								if(printStmt(rhs).find("sizeof")!=std::string::npos)
								{									
									common::printLog( printStmt(lhs)+"<"+printStmt(rhs)+"\n",common::CheckerName::arrayBound,0,*common::configure);									
									result=1;
								}
							}
							else if(it->getOpcode()==clang::BinaryOperatorKind::BO_GT)
							{
								result=CheckConditionExpr(rhs,"<",value);//
								if(printStmt(rhs).find("sizeof")!=std::string::npos)
								{									
									common::printLog( printStmt(lhs)+"<="+printStmt(rhs)+"\n",common::CheckerName::arrayBound,0,*common::configure);									
									result=1;
								}
							}
						}
						else if(index==printStmt(rhs))
						{
							if(it->getOpcode()==clang::BinaryOperatorKind::BO_LE)
							{
								result=CheckConditionExpr(lhs,"<",value);
								if(printStmt(lhs).find("sizeof")!=std::string::npos)
								{									
									common::printLog( printStmt(lhs)+">"+printStmt(rhs)+"\n",common::CheckerName::arrayBound,0,*common::configure);									
									result=1;
								}
							}
							else if(it->getOpcode()==clang::BinaryOperatorKind::BO_LT)
							{
								result=CheckConditionExpr(lhs,"<",value);//
								if(printStmt(lhs).find("sizeof")!=std::string::npos)
								{									
									common::printLog( printStmt(lhs)+">="+printStmt(rhs)+"\n",common::CheckerName::arrayBound,0,*common::configure);									
									result=1;
								}
							}
						}
					}
				}
				else if(con.op==clang::BinaryOperatorKind::BO_GE)
				{
					string index=printStmt(con.lhs->IgnoreImpCasts()->IgnoreParenCasts());
					
					if(flag)
					{
						//it->dump();
						uint64_t value = con.rhs;
						if(index==printStmt(lhs))
						{
							if(it->getOpcode()==clang::BinaryOperatorKind::BO_GE)
							{
								
								result=CheckConditionExpr(rhs,">=",value);
							}
							else if(it->getOpcode()==clang::BinaryOperatorKind::BO_GT)
							{
								result=CheckConditionExpr(rhs,">=",value);
							}
							////////cerr<<result<<endl;
						}
						else if(index==printStmt(rhs))
						{
							if(it->getOpcode()==clang::BinaryOperatorKind::BO_LT)
							{
								result=CheckConditionExpr(rhs,">=",value);
							}
							else if(it->getOpcode()==clang::BinaryOperatorKind::BO_LE)
							{
								result=CheckConditionExpr(rhs,">=",value);
							}
						}
					}
					else
					{
						uint64_t value = con.rhs;
						if(index==printStmt(lhs))
						{
							if(it->getOpcode()==clang::BinaryOperatorKind::BO_LT)
							{
								result=CheckConditionExpr(rhs,">=",value);
							}
							else if(it->getOpcode()==clang::BinaryOperatorKind::BO_LE)
							{
								result=CheckConditionExpr(rhs,">=",value);
							}
						}
						else if(index==printStmt(rhs))
						{
							if(it->getOpcode()==clang::BinaryOperatorKind::BO_GE)
							{
								result=CheckConditionExpr(rhs,">=",value);
							}
							else if(it->getOpcode()==clang::BinaryOperatorKind::BO_GT)
							{
								result=CheckConditionExpr(rhs,">=",value);
							}
						}
					}
				}
				if(!result )
				{
					//////cerr<<"Composed expr"<<endl;
					vector<PNode> tmp;
					common::printLog( "result false, call analyseIndex:\n",common::CheckerName::arrayBound,0,*common::configure);
					analyseIndex(input.index,tmp);
					//////cerr<<"analyse index"<<endl;
					//////cerr<<printStmt(input.index)<<endl;
					common::printLog( "after analyseIndex:"+printStmt(input.index)+"\n",common::CheckerName::arrayBound,0,*common::configure);
					if(tmp.size()>1)
					{
						if(con.op==clang::BinaryOperatorKind::BO_LT)
						{
							
							result=checkComposedExpr(it,tmp,input,con,flag);
							
						}
						else if(con.op==clang::BinaryOperatorKind::BO_GE)
						{
							
							result=checkComposedExpr(it,tmp,input,con,flag);
							
						}

					}
				}
			}
		}
	}
	else if(UnaryOperator * it=dyn_cast<UnaryOperator>(stmt))
	{
		//cerr<<"UnaryOperator"<<endl;
		if(it->getOpcode()==clang::UnaryOperatorKind::UO_LNot)
		{
			//cerr<<"Not"<<endl;
			Expr* expr=it->getSubExpr()->IgnoreImpCasts()->IgnoreParenCasts();
			return checkConditionStmt(expr,input,con,!flag);
		}
	}
	return result;//*/
}


void ArrayOutOfBoundsCheckerV4::initialize()
{
	unfinished.clear();
}


////接口函数
bool ArrayOutOfBoundsCheckerV4::isTainted(FunctionDecl* Func,CFGBlock* block,Stmt* stmt,Expr* expr)
{
	//std::////cerr<<"In ArrayOutOfBoundsCheckerV4, isTainted():"<<endl;
	return taint::TaintChecker::is_tainted(Func,block,stmt,expr);
	//std::////cerr<<"Out ArrayOutOfBoundsCheckerV4, isTainted():"<<endl;
//	//std::unordered_map<ExprPosition*, TaintValue> list=TaintChecker.getTaintValueList();
//	//ExprPosition key(Func,block,stmt,expr);
//	//TaintValue value=list[key];
//	//if(value.kind==TaintKind::Tainted)
//	//	return true;
//	//else
//	//	return false;
//		if(IntegerLiteral *ILE = dyn_cast<IntegerLiteral>(expr)) return false;
//		VarDecl *tmp=getVarDecl(expr);	
//		if(tmp!=NULL){
//			string name=tmp->getQualifiedNameAsString();
//			if(name=="i") return false;
//		}
//		//else
//		return true;
}

vector<ArraySubscript> ArrayOutOfBoundsCheckerV4::LocatingTaintExpr(FunctionDecl *func,CFG * myCFG)
{
	//this function will locate the targeted array index, and get the array boundary information
	vector<ArraySubscript> result;
	mapToBlockOutLoopExpr.clear();
	mapToBlockInLoopExpr.clear();
	mapToLoopTaintedExpr.clear();
	mapToBlockTaintedLoopExpr.clear();
	mapToCheckBlock.clear();
	unfinished.clear();
	////llvm::errs()<<"before AnalyeLoopExpr\n";
	//AnalyeLoopExpr(func,myCFG);
	////llvm::errs()<<"b\n";
	//get array information,return to result
	ifAnalyeLoopExpr=false;
	result = getArraySubscript(func,myCFG);
	////llvm::errs()<<"2\n";
	mapToBlockOutLoopExpr.clear();
	mapToBlockInLoopExpr.clear();
	mapToLoopTaintedExpr.clear();
	mapToBlockTaintedLoopExpr.clear();
	mapToCheckBlock.clear();
	unfinished.clear();
	return result;
}

void ArrayOutOfBoundsCheckerV4::AnalyeLoopExpr(FunctionDecl *func,CFG * cfg)
{
	common::printLog( "begin AnalyeLoopExpr:\n",common::CheckerName::arrayBound,3,*common::configure);
	//init init value and block to visit
	for(CFG::iterator iter=cfg->begin();iter!=cfg->end();++iter){
		CFGBlock* block=*iter;
		//////std::cerr<<block->getBlockID()<<endl;
		vector<Expr*> vardecl;
		mapToBlockInLoopExpr.insert(pair<CFGBlock*, vector<Expr*>>(block,vardecl));
		vector<pair<CFGBlock*,vector<Expr*>>> BlockVar;
		

		////std::cerr<<"1\n";
		for(CFGBlock::succ_iterator succ_iter=block->succ_begin();succ_iter!=block->succ_end();++succ_iter){

			CFGBlock* succ=*succ_iter;
			if(succ==NULL)continue;
			BlockVar.push_back(pair<CFGBlock*, vector<Expr*>>(succ,vardecl));
		}
		////std::cerr<<"2\n";

		mapToBlockOutLoopExpr.insert(pair<CFGBlock*, vector<pair<CFGBlock*,vector<Expr*>>>>(block,BlockVar));
		
		vector<Expr*> taintedExpr;;
		Stmt* it=(block->getTerminator()).getStmt();
		////std::cerr<<"3\n";

		if(it!=NULL)
		{
			Expr* cond=NULL;
			if(it->getStmtClass()==Stmt::ForStmtClass)
			{
				//////std::cerr<<"6\n";
				cond=((ForStmt*)it)->getCond();
				////std::cerr<<"6\n";
				taintedExpr=checkTaintExpr(cond,block,func,false);
				////std::cerr<<"6\n";
				unfinished.push_back(block);
			}
			else if(it->getStmtClass()==Stmt::WhileStmtClass)
			{
				//////std::cerr<<"7\n";
				cond=((WhileStmt*)it)->getCond();
				////std::cerr<<"7\n";
				taintedExpr=checkTaintExpr(cond,block,func,false);
				////std::cerr<<"7\n";
				unfinished.push_back(block);
			}
		////std::cerr<<"4\n";
			
		}
		
		mapToBlockTaintedLoopExpr.insert(pair<CFGBlock*, vector<Expr*>>(block,taintedExpr));
		////std::cerr<<"5\n";

		
	}
	common::printLog( "initial loop var map\n",common::CheckerName::arrayBound,1,*common::configure);	
	////std::cerr<<"finish pre work"<<endl;
	while(unfinished.size()!=0)
	{
		
		CFGBlock* block=unfinished[0];
		//////std::cerr<<"loop\n";
		unfinished.erase(unfinished.begin());
		vector<Expr*> expr_in;
		vector<pair<CFGBlock*,vector<Expr*>>> expr_out;
		for(CFGBlock::pred_iterator piter=block->pred_begin();piter!=block->pred_end();piter++)
		{
			CFGBlock* pred=*piter;
			vector<pair<CFGBlock*,vector<Expr*>>> pred_out=mapToBlockOutLoopExpr[pred];
			vector<Expr*> temp_expr_in;
			for(unsigned i=0;i<pred_out.size();i++)
			{
				if(pred_out[i].first==block)
				{
					temp_expr_in=pred_out[i].second;
					break;
				}
			}
			if(piter==block->pred_begin())
			{
				expr_in=temp_expr_in;
			}
			else
			{
				expr_in=Union(expr_in,temp_expr_in);
			}
		}
		//in expr
		bool init = true;
		
		for(CFGBlock::succ_iterator succ_iter=block->succ_begin();succ_iter!=block->succ_end();succ_iter++)
		{
			CFGBlock* succ=*succ_iter;
			if(succ==NULL)continue;
			if(init)
			{
				init=false;
				expr_out.push_back(pair<CFGBlock*,vector<Expr*>>(succ,Union(expr_in,mapToBlockTaintedLoopExpr[block])));
			}
			else
			{
				expr_out.push_back(pair<CFGBlock*,vector<Expr*>>(succ,expr_in));
			}
		}
		
		vector<pair<CFGBlock*,vector<Expr*>>> pre_out=mapToBlockOutLoopExpr[block];
		if(!isEqual(pre_out,expr_out))
		{
			////std::cerr<<"update\n";
			for(CFGBlock::succ_iterator succ_iter=block->succ_begin();succ_iter!=block->succ_end();++succ_iter){
				CFGBlock *succ=*succ_iter;
				if(succ==NULL)continue;
				if(find(unfinished.begin(),unfinished.end(),succ)==unfinished.end())
					unfinished.push_back(succ);	
			}
			map<CFGBlock*, vector<Expr*> >::iterator expr_in_iter;
			expr_in_iter=mapToBlockInLoopExpr.find(block);
			mapToBlockInLoopExpr.erase(expr_in_iter);
			mapToBlockInLoopExpr.insert(pair<CFGBlock*, vector<Expr*> >(block,expr_in));

			map<CFGBlock*, vector<pair<CFGBlock*,vector<Expr*> > > >::iterator expr_out_iter;
			expr_out_iter=mapToBlockOutLoopExpr.find(block);
			mapToBlockOutLoopExpr.erase(expr_out_iter);
			mapToBlockOutLoopExpr.insert(pair<CFGBlock*, vector<pair<CFGBlock*,vector<Expr*> > > >(block,expr_out));
		}

	}

	//llvm::errs() <<"AnalyeLoopExpr end\n";
	//print info
	/*LangOptions LO;
	LO.CPlusPlus=1; 
	cfg->dump(LO,true);
	for(CFG::iterator iter=cfg->begin();iter!=cfg->end();++iter){
		CFGBlock* block=*iter;
		////cerr<<"BLOCK:"<<block->getBlockID()<<endl;
		////cerr<<"Loop Expr"<<endl;
		 vector<Expr*> list=mapToBlockInLoopExpr[block];
		 for(Expr* e : list)
		 {
		 	////cerr<<"\tExpr:"<<printStmt(e)<<endl;
		 	vector<Expr*> tlist=mapToLoopTaintedExpr[e];
		 	 for(Expr* te : tlist)
		 	{
		 		////cerr<<"\t\tTaintExpr:"<<printStmt(te)<<endl;
		 		////cerr<<"\t\tCheckBlock:"<<mapToCheckBlock[te]->getBlockID()<<endl;
		 	}
		 	
		 }

	}//*/
	ifAnalyeLoopExpr=true;
	common::printLog( "End AnalyeLoopExpr!\n",common::CheckerName::arrayBound,3,*common::configure);
}

bool ArrayOutOfBoundsCheckerV4::checkExprTaintedForLoop(FunctionDecl* Func,CFGBlock* block,Stmt* stmt,Expr* expr)
{
	////std::cerr<<"function checkExprTaintedForLoop\n";
	if(expr==nullptr||stmt==nullptr||block==nullptr)return false;
	if(dyn_cast<IntegerLiteral>(expr))
	{
		return false;
	}
	CFGBlock* temp=block;
	
	while(temp!=nullptr)
	{
	//err<<"in\n";
		for(CFGBlock::iterator iter=temp->begin();iter!=temp->end();iter++)
		{
			CFGElement element=*(iter);
			//////std::cerr<<"block iterator\n";
			if(element.getKind()==CFGElement::Kind::Statement)
			{
				const Stmt* it=((CFGStmt*)&element)->getStmt();
				if(it==nullptr)continue;
				//////std::cerr<<"block stmt\n";
				
				if(it==stmt)
				{
					//
					
					bool t=  isTainted(Func,temp,stmt,expr);
					////std::cerr<<"after isTainted in arrayOUtOfBoundsCheckerV4\n";
					return t;
				}
			}
		}
		
		temp=*(temp->pred_begin());
		////std::cerr<<"out\n";
		//clang::CFGTerminator cfgt=temp->getTerminator();
		//if(cfgt==NULL)break;
		
	}
	return false;
}

vector<Expr*> ArrayOutOfBoundsCheckerV4::checkTaintExpr(Expr* stmt,CFGBlock* block,FunctionDecl* func,bool flag)
{
	////std::cerr<<"checkTaintExpr\n";
	vector<Expr*> TaintedExpr;
	if(stmt==nullptr)
	{
		//////std::cerr<<"NULL\n";
		return TaintedExpr;
	}
	if(BinaryOperator * it=dyn_cast<BinaryOperator>(stmt))
	{
		////std::cerr<<"BinaryOperator\n";
		switch(it->getOpcode())
		{
		case clang::BinaryOperatorKind::BO_LAnd:
			{
				////std::cerr<<"BinaryOperator BO_LAnd\n";
				vector<Expr*> temp_lhs=checkTaintExpr(it->getLHS()->IgnoreParenCasts()->IgnoreImpCasts(),block,func,flag);
				vector<Expr*> temp_rhs=checkTaintExpr(it->getRHS()->IgnoreParenCasts()->IgnoreImpCasts(),block,func,flag);
				for(unsigned i=0;i<temp_lhs.size();i++)
				{
					if(find(TaintedExpr.begin(),TaintedExpr.end(),temp_lhs[i])==TaintedExpr.end())
					{
						TaintedExpr.push_back(temp_lhs[i]);
					}
				}
				for(unsigned i=0;i<temp_rhs.size();i++)
				{
					if(find(TaintedExpr.begin(),TaintedExpr.end(),temp_rhs[i])==TaintedExpr.end())
					{
						TaintedExpr.push_back(temp_rhs[i]);
					}
				}
				break;
			}
		case clang::BinaryOperatorKind::BO_LOr:
			{
				////std::cerr<<"BinaryOperator BO_LOr\n";
				vector<Expr*> temp_lhs=checkTaintExpr(it->getLHS()->IgnoreParenCasts()->IgnoreImpCasts(),block,func,flag);
				vector<Expr*> temp_rhs=checkTaintExpr(it->getRHS()->IgnoreParenCasts()->IgnoreImpCasts(),block,func,flag);
				for(unsigned i=0;i<temp_lhs.size();i++)
				{
					if(find(TaintedExpr.begin(),TaintedExpr.end(),temp_lhs[i])==TaintedExpr.end())
					{
						TaintedExpr.push_back(temp_lhs[i]);
					}
				}
				for(unsigned i=0;i<temp_rhs.size();i++)
				{
					if(find(TaintedExpr.begin(),TaintedExpr.end(),temp_rhs[i])==TaintedExpr.end())
					{
						TaintedExpr.push_back(temp_rhs[i]);
					}
				}
				break;
			}
		default:
			{
				////std::cerr<<"check\n";
				Expr* lhs=it->getLHS();
				Expr* rhs=it->getRHS();
				vector<Expr*> texpr;
				if(flag)
				{
					////std::cerr<<"true\n";
					if(it->getOpcode()==clang::BinaryOperatorKind::BO_LE||it->getOpcode()==clang::BinaryOperatorKind::BO_LT)
					{
						////std::cerr<<"checkExprTaintedForLoop\n";
						if(checkExprTaintedForLoop(func,block,stmt,lhs))
						{
							if(dyn_cast<IntegerLiteral>(rhs))
							{
							}
							else
							{
								////std::cerr<<"1\n";
								TaintedExpr.push_back(rhs);
								texpr.push_back(lhs);
								mapToLoopTaintedExpr.insert(pair<Expr*,vector<Expr*>>(rhs,texpr));
								mapToCheckBlock.insert(pair<Expr*,CFGBlock*>(lhs,block));
								////std::cerr<<"2\n";
							}
						}
					}
					else if(it->getOpcode()==clang::BinaryOperatorKind::BO_GE||it->getOpcode()==clang::BinaryOperatorKind::BO_GT)
					{
						////std::cerr<<"checkExprTaintedForLoop\n";
						if(checkExprTaintedForLoop(func,block,stmt,rhs))
						{
							if(dyn_cast<IntegerLiteral>(lhs))
							{
							}
							else
							{
								////std::cerr<<"1\n";
								TaintedExpr.push_back(lhs);
								texpr.push_back(rhs);
								mapToLoopTaintedExpr.insert(pair<Expr*,vector<Expr*>>(lhs,texpr));
								mapToCheckBlock.insert(pair<Expr*,CFGBlock*>(rhs,block));
								////std::cerr<<"2\n";
								
							}
						}
					}
				}
				else
				{
					////std::cerr<<"false\n";
					if(it->getOpcode()==clang::BinaryOperatorKind::BO_LE||it->getOpcode()==clang::BinaryOperatorKind::BO_LT)
					{
						////std::cerr<<"checkExprTaintedForLoop  f1\n";
						if(checkExprTaintedForLoop(func,block,stmt,rhs))
						{
							if(dyn_cast<IntegerLiteral>(lhs))
							{
							}
							else
							{
							////std::cerr<<"1\n";
								TaintedExpr.push_back(lhs);
								texpr.push_back(rhs);
								mapToLoopTaintedExpr.insert(pair<Expr*,vector<Expr*>>(lhs,texpr));
								mapToCheckBlock.insert(pair<Expr*,CFGBlock*>(rhs,block));
								////std::cerr<<"2\n";
								
							}
						}
					}
					else if(it->getOpcode()==clang::BinaryOperatorKind::BO_GE||it->getOpcode()==clang::BinaryOperatorKind::BO_GT)
					{
						////std::cerr<<"checkExprTaintedForLoop  f2\n";
						if(checkExprTaintedForLoop(func,block,stmt,lhs))
						{
							if(dyn_cast<IntegerLiteral>(rhs))
							{
							}
							else
							{
								////std::cerr<<"1\n";
								TaintedExpr.push_back(rhs);
								texpr.push_back(lhs);
								mapToLoopTaintedExpr.insert(pair<Expr*,vector<Expr*>>(rhs,texpr));
								mapToCheckBlock.insert(pair<Expr*,CFGBlock*>(lhs,block));
								////std::cerr<<"2\n";
							}
						}
					}
				}
				break;
			}
		}
	}
	else if(UnaryOperator * it=dyn_cast<UnaryOperator>(stmt))
	{
		////std::cerr<<"UnaryOperator\n";
		if(it->getOpcode()==clang::UnaryOperatorKind::UO_LNot)
		{
			Expr* expr=it->getSubExpr()->IgnoreImpCasts()->IgnoreParenCasts();
			////std::cerr<<"finish\n";
			return checkTaintExpr(expr,block,func,!flag);
		}
	}
	////std::cerr<<"finish\n";
	return TaintedExpr;
}
std::string ArrayOutOfBoundsCheckerV4::replace_all(string str, const string old_value, const string new_value)
{     
    while(true){
		string::size_type pos(0);     
        if((pos=str.find(old_value))!=string::npos)     
            str.replace(pos,old_value.length(),new_value);     
        else 
			break;     
    }     
    return str;
} 
void ArrayOutOfBoundsCheckerV4::reportWarning(ArraySubscript as){	
	string loc=as.location;
	string originalLoc=as.originalLocation;
	common::printLog( "reportWarning,asID:"+int2string(as.ID)+"\n",common::CheckerName::arrayBound,0,*common::configure);
	//if is not loopboundchecking, if as.ID not found in reportedID, we can report it, else found and reported, return
	if(!as.isLoopBoundChecking){
		if(find(reportedID.begin(),reportedID.end(),as.ID)==reportedID.end())
		{
			common::printLog( "Not find in reportedID,asID:"+int2string(as.ID)+"\n",common::CheckerName::arrayBound,0,*common::configure);
			common::printLog( "as originalLocation:"+originalLoc+"\n",common::CheckerName::arrayBound,0,*common::configure);
			reportedID.push_back(as.ID);
		}
		else
			return;
	}
	else{
		

	}
	////////cerr<<"reportWarnings:"<<reportWarnings;
	warningCount++;
	
	string func=as.func->getNameAsString();

	//string originalfile,originalline;
	//string originalLoc0=originalLoc;
	//if(originalLoc.find("Spelling")!=std::string::npos){
	//	int sp=originalLoc.find("Spelling");
	//	common::printLog( "Spelling Position"+int2string(sp)+"\n",common::CheckerName::arrayBound,0,*common::configure);
	//	common::printLog( "Found Spelling\n",common::CheckerName::arrayBound,0,*common::configure);
	//	originalLoc0.assign(originalLoc.c_str(),sp-2);
	//	common::printLog( "Remove Spelling"+originalLoc0+"\n",common::CheckerName::arrayBound,0,*common::configure);
	//}
	//int opos = originalLoc0.find_last_of(':');
	//string ostr;
	//ostr.assign(originalLoc0.c_str(),opos);
	////.....test.c:20
	//int opos2=ostr.find_last_of(':');				
	//originalfile.assign(ostr.c_str(),opos2);
	//originalline = ostr.substr(opos2+1);	

	string file,desc="",line,idxCnt;
	string loc0=loc;
	if(loc.find("Spelling")!=std::string::npos){
		int sp=loc.find("Spelling");
		common::printLog( "Spelling Position"+int2string(sp)+"\n",common::CheckerName::arrayBound,0,*common::configure);
		common::printLog( "Found Spelling\n",common::CheckerName::arrayBound,0,*common::configure);
		loc0.assign(loc.c_str(),sp-2);
		common::printLog( "Remove Spelling"+loc0+"\n",common::CheckerName::arrayBound,0,*common::configure);
	}
	reportWarnings+=loc0;
	reportWarnings+="\n";
	int pos = loc0.find_last_of(':');
	string str;
	str.assign(loc0.c_str(),pos);
	//.....test.c:20
	int pos2=str.find_last_of(':');				
	file.assign(str.c_str(),pos2);
	line = str.substr(pos2+1);	
	idxCnt = int2string(as.indexCnt+1);
	//if(as.isLoopBoundChecking) desc+="\n\t\tArray's Original Location line "+originalline+"\n\t\t"+originalfile;
	desc+="\n\t\tArray expression: "+as.arrayExpr+"\n\t\tneeds bound checking:\n";
	for(auto con:as.condition){
		desc+="\t\t"+getInfo(con);
		desc+=";\n";
	}	
	////////cerr<<desc<<endl;
	string checkerId;
	if(as.isLoopBoundChecking){
		checkerId="MOLINT.TAINTED.LOOP.BOUND";		
		desc+="\t\tArray Index "+printStmt(as.arrayIdx)+" is a loop variable,\n\t\tits corresponding loop bound is "+printStmt(as.index) ;
		desc+="\n";
	}else{
		checkerId="MOLINT.TAINTED.ARRAY.INDEX";
	}
	desc+="\t\t";
	
	set<vector<string> > tmp_path=mapToPath[as.ID];
	if(tmp_path.size()>0)
	{
		desc+="stack:\n\t\t";
		for(vector<string> path : tmp_path)
		{
			for(int i=path.size()-1;i>=0;i--)
			{
				desc+=path[i];
				if(i!=0)desc+="->";
			}
			desc+="\n\t\t";
		}
	}
	else
		desc+="in Current Function\n\t\t";
	file=replace_all(file, "\\", "/");
	writingToXML(as.isLoopBoundChecking,file, func, desc, line, checkerId, idxCnt, as.arrayExpr);
}
void ArrayOutOfBoundsCheckerV4::reportWarningSameID(vector<ArraySubscript> as){
	//this function will report the warnings which have the same ID
	//actually, this function is designed for the case that for(i<m&&i<n), in a loop bound checking,
	//we save m<size and n<size, they have the same ID as i<size.
	//at the end, we just report them togethor.
	if(as.size()==0) return;
	string loc=as[0].location;
	string originalLoc=as[0].originalLocation;
	common::printLog( "reportWarningSameID,asID:"+int2string(as[0].ID)+"\n",common::CheckerName::arrayBound,0,*common::configure);
	//if is not loopboundchecking, if as.ID not found in reportedID, we can report it, else found and reported, return
	warningCount++;
	
	string func=as[0].func->getNameAsString();	

	string file,desc="",line,idxCnt;
	string loc0=loc;
	if(loc.find("Spelling")!=std::string::npos){
		int sp=loc.find("Spelling");
		common::printLog( "Spelling Position"+int2string(sp)+"\n",common::CheckerName::arrayBound,0,*common::configure);
		common::printLog( "Found Spelling\n",common::CheckerName::arrayBound,0,*common::configure);
		loc0.assign(loc.c_str(),sp-2);
		common::printLog( "Remove Spelling"+loc0+"\n",common::CheckerName::arrayBound,0,*common::configure);
	}
	reportWarnings+=loc0;
	reportWarnings+="\n";
	int pos = loc0.find_last_of(':');
	string str;
	str.assign(loc0.c_str(),pos);
	//.....test.c:20
	int pos2=str.find_last_of(':');				
	file.assign(str.c_str(),pos2);
	line = str.substr(pos2+1);	
	idxCnt = int2string(as[0].indexCnt+1);
	//if(as.isLoopBoundChecking) desc+="\n\t\tArray's Original Location line "+originalline+"\n\t\t"+originalfile;
	desc+="\n\t\tArray expression: "+as[0].arrayExpr+"\n\t\tneeds bound checking:\n";
	for(auto aas:as){
		for(auto con:aas.condition){
			desc+="\t\t"+getInfo(con);
			desc+=";\n";
		}	
	}
	////////cerr<<desc<<endl;
	string checkerId;
	if(as[0].isLoopBoundChecking){
		checkerId="MOLINT.TAINTED.LOOP.BOUND";	
		string loopbound="";
		for(auto aas:as){loopbound=loopbound+", "+printStmt(aas.index);}
		desc+="\t\tArray Index "+printStmt(as[0].arrayIdx)+" is a loop variable,\n\t\tits corresponding loop bound is "+loopbound;
		desc+="\n";
	}else{
		checkerId="MOLINT.TAINTED.ARRAY.INDEX";
	}
	desc+="\t\t";
	
	set<vector<string> > tmp_path=mapToPath[as[0].ID];
	if(tmp_path.size()>0)
	{
		desc+="stack:\n\t\t";
		for(vector<string> path : tmp_path)
		{
			for(int i=path.size()-1;i>=0;i--)
			{
				desc+=path[i];
				if(i!=0)desc+="->";
			}
			desc+="\n\t\t";
		}
	}
	else
		desc+="in Current Function\n\t\t";
	file=replace_all(file, "\\", "/");
	writingToXML(as[0].isLoopBoundChecking,file, func, desc, line, checkerId, idxCnt, as[0].arrayExpr);
}
const vector<uint64_t> ArrayOutOfBoundsCheckerV4::getArraySize(Expr* ex){
	//ex->dump();
	vector<uint64_t> tmp;
	Expr *exprTmp=ex->IgnoreParenCasts()->IgnoreImpCasts();
	/*while(ImplicitCastExpr * it = dyn_cast<ImplicitCastExpr>(exprTmp)){
		exprTmp=it->IgnoreImpCasts();
	}*/
	////////cerr<<"getArraySize"<<endl;
	//exprTmp->dump();
	ValueDecl *valueDecl;
	if(DeclRefExpr* declRef=dyn_cast<DeclRefExpr>(exprTmp)){
			//declRef->dump();
			valueDecl = declRef->getDecl();	
			//valueDecl->dump();
					
	}
	else if(MemberExpr *mem = dyn_cast<MemberExpr>(exprTmp)){
		//mem->dump();		
		valueDecl=mem->getMemberDecl();
		//valueDecl->dump();			
	}
	else{
		return tmp;
	}
	if(!((VarDecl*)valueDecl)->hasExternalStorage()){
		QualType qt = valueDecl->getType ();					
		//deal with a[][][]	to get array size													
		while(const ConstantArrayType *CAT = dyn_cast<ConstantArrayType>(qt)){													
			//qt->dump();
			qt=CAT->getElementType();
			const llvm::APInt & size = CAT->getSize();	
			uint64_t v = size.getLimitedValue();
			////////cerr<<"size:"<<getInfo(size)<<endl;
			tmp.push_back(v);												
		}	
	}else{
		////////cerr<<"used Extern!!!!!!!!!!!!!!"<<endl;
	}
				
	return tmp;
}
VarDecl* ArrayOutOfBoundsCheckerV4::getVarDecl(Expr* ex){
	VarDecl* tmp=NULL;
	Expr *exprTmp=ex->IgnoreParenCasts()->IgnoreImpCasts();
	/*while(ImplicitCastExpr * it = dyn_cast<ImplicitCastExpr>(exprTmp)){
		exprTmp=it->IgnoreImpCasts();
	}*/
	if(DeclRefExpr* declRef=dyn_cast<DeclRefExpr>(exprTmp)){
		if (VarDecl *VD = dyn_cast<VarDecl>(declRef->getDecl())){
			tmp=VD;
		}
	}
	return tmp;
}
bool ArrayOutOfBoundsCheckerV4::isUnsigned(Expr *ex){
	//ex->dump();
	if(ConditionalOperator *co=dyn_cast<ConditionalOperator>(ex->IgnoreImpCasts()->IgnoreParenCasts()))
	{
		Expr* lhs=co->getLHS()->IgnoreImpCasts()->IgnoreImpCasts()->IgnoreParenCasts();
		//Expr* rhs=co->getRHS()->IgnoreImpCasts()->IgnoreImpCasts()->IgnoreParenCasts();
		if(dyn_cast<IntegerLiteral>(lhs)) 
		{
			return isUnsigned(co->getRHS());
		}
		else if(dyn_cast<FloatingLiteral>(lhs))//-ImplicitCastExpr
		{
			return isUnsigned(co->getRHS());
		}
		else
		{
			//cerr<<printStmt(co->getLHS())<<endl;
			return isUnsigned(lhs);
		}
	}
	
	Expr *exprTmp=ex->IgnoreParenCasts()->IgnoreImpCasts();
	/*while(ImplicitCastExpr * it = dyn_cast<ImplicitCastExpr>(exprTmp)){
		exprTmp=it->IgnoreImpCasts();
	}*/
	QualType qt;
	if(DeclRefExpr *declRef = dyn_cast<DeclRefExpr>(exprTmp)){
		ValueDecl *valueDecl = declRef->getDecl();
		qt = valueDecl->getType ();		
	}
	else if(MemberExpr *mem = dyn_cast<MemberExpr>(exprTmp)){
		//mem->dump();		
		ValueDecl *valueDecl=mem->getMemberDecl();
		//valueDecl->dump();
		qt = valueDecl->getType ();	
	}
	else if(ArraySubscriptExpr *ASE = dyn_cast<ArraySubscriptExpr>(exprTmp)){
		//ASE->dump();
		qt = ASE->getType ();		
	}
	else return false;	
	////////cerr<<"QualType;"<<endl;
	//qt->dump();	
	if(const BuiltinType *bt = dyn_cast<BuiltinType>(qt)){
		if(bt->isSignedInteger()){/*//////cerr<<"SignedInteger"<<endl;*/return false;}
		if(bt->isUnsignedInteger()){/*//////cerr<<"UnsignedInteger"<<endl;*/return true;}
	}
	if(const TypedefType *tt = dyn_cast<TypedefType>(qt)){
		const QualType t=tt->desugar();
		const QualType c = t.getCanonicalType();
		//c->dump();
		//std::string str=c.getAsString();
		if(const BuiltinType *bt = dyn_cast<BuiltinType>(c)){
			if(bt->isSignedInteger()){/*//////cerr<<"SignedInteger"<<endl;*/return false;}
			if(bt->isUnsignedInteger()){/*//////cerr<<"UnsignedInteger"<<endl;*/return true;}
		} 

	}
	return false;
}
int ArrayOutOfBoundsCheckerV4::writingToXML(bool isLoopBoundChecking, string fileName, string funName, string descr, string locLine,string checkerId, string indexCnt, string arrayExpr)
{
	pugi::xml_node node;
	if(isLoopBoundChecking)
	{
		string report=fileName+funName+descr+locLine+checkerId+indexCnt+arrayExpr;
		if(find(loopReport.begin(),loopReport.end(),report)==loopReport.end()){
			common::printLog( "Not find in loopReport\n",common::CheckerName::arrayBound,0,*common::configure);
			loopReport.push_back(report);
		}
		else{
			common::printLog( "find in loopReport,return\n",common::CheckerName::arrayBound,0,*common::configure);
			return 0;
		}
		node= doc_loopbound.append_child("error");
		doc_loopbound_empty = false;
	}
	else{
		node= doc_arraybound.append_child("error");
		doc_arraybound_empty = false;
	}

    pugi::xml_node checker = node.append_child("checker");
	checker.append_child(pugi::node_pcdata).set_value(checkerId.c_str());

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

	//pugi::xml_node arrayExpression = event.append_child("arrayExpression");
	//arrayExpression.append_child(pugi::node_pcdata).set_value(arrayExpr.c_str());


	//pugi::xml_node arrayIndex = event.append_child("arrayIndex");
    //arrayIndex.append_child(pugi::node_pcdata).set_value(indexCnt.c_str());

	pugi::xml_node extra = node.append_child("extra");
    extra.append_child(pugi::node_pcdata).set_value("none");

	pugi::xml_node subcategory = node.append_child("subcategory");
    subcategory.append_child(pugi::node_pcdata).set_value("none");

	return 0;
}
std::unordered_map<Expr*, vector<Expr*>> ArrayOutOfBoundsCheckerV4::getIdx(Expr *ex){
	std::unordered_map<Expr*, vector<Expr*>> tmp;	
	vector<Expr*> tmpIdx;
	Expr *root=ex;
	/*NOTE:
	m[i][j][k],getBase will get m[i][j];getIdx will get k,
	we get index:k,j,i in loop, 
	when ASE's left children is DeclRefExpr, we know there's not ArraySubscriptExpr in its children,so we set root=NULL, end the loop;
	In the last loop,getBase will get m;
	* */
	//root->dump();
	while(root!=NULL){
		//root->dump();
		if(ArraySubscriptExpr *ASE = dyn_cast<ArraySubscriptExpr>(root->IgnoreParenCasts()->IgnoreImpCasts())){
			//ASE->dump();
			////////cerr<<"hi getIdx"<<endl;
			Expr *idx = ASE->getIdx()->IgnoreParenCasts()->IgnoreImpCasts();	
			//idx->dump();		
			tmpIdx.push_back(idx);	
			root=ASE->getBase()->IgnoreParenCasts()->IgnoreImpCasts();			
		}else{
			reverse(tmpIdx.begin(),tmpIdx.end());
			tmp.insert(std::make_pair(root,tmpIdx));
			root=NULL;
		}
	}
	return tmp;
}
vector<ArraySubscript> ArrayOutOfBoundsCheckerV4::getArraySubscriptInExpr(Expr * ex,FunctionDecl *f,CFGBlock* block,Stmt* stmt,CFG * myCFG){
	//this function can get array subscript information of a statement.	
	common::printLog( "getArraySubscriptInExpr begin:\n",common::CheckerName::arrayBound,3,*common::configure);
	vector<ArraySubscript> tmpAS;
	std::unordered_map<Expr*, vector<Expr*>> base_Idx = getIdx(ex);
	if(base_Idx.size()>0){
		for(auto content:base_Idx){
			Expr * base = content.first;
//#ifdef _DEBUG
//			////cerr<<"Base:"<<endl;
//			base->dump();
//#endif
			const vector<uint64_t> arrSize=getArraySize(base);			
			//if use extern,arrSize will be null.			
			
			common::printLog( "array size number:"+int2string(arrSize.size())+"\n",common::CheckerName::arrayBound,1,*common::configure);
			//for(auto s:arrSize) cerr<<s<<endl;
			common::printLog( "index number:"+int2string(content.second.size())+"\n",common::CheckerName::arrayBound,1,*common::configure);
			//for(auto c:content.second) cerr<<printStmt(c)<<endl;
			if(arrSize.size()!=content.second.size()){
				common::printLog( "array size number not equal index number!\n",common::CheckerName::arrayBound,1,*common::configure);
				return tmpAS;
			}
			if(arrSize.size()>0){
				int i=0;
				if(content.second.size()>0){
					//unsigned idxcnt=0;
					for(auto idx:content.second){
						ArraySubscript as;
						if(dyn_cast<IntegerLiteral>(idx)) {
							common::printLog( "index is IntegerLiteral:"+printStmt(idx)+",continue\n",common::CheckerName::arrayBound,0,*common::configure);
							i++;
							continue;
						}
						//idx->dump();
						if(SimpleExprCheckOnly){
							if(dyn_cast<BinaryOperator>(idx)){
								common::printLog( "SimpleExprCheckOnly,BinaryOperator:"+printStmt(idx)+",continue\n",common::CheckerName::arrayBound,0,*common::configure);
								i++;	
								continue;
							}
							else if(dyn_cast<UnaryOperator>(idx)){
								common::printLog( "SimpleExprCheckOnly,UnaryOperator:"+printStmt(idx)+",continue\n",common::CheckerName::arrayBound,0,*common::configure);
								i++;	
								continue;

							}
							else if(dyn_cast<CallExpr>(idx)){
								common::printLog( "SimpleExprCheckOnly,callexpr:"+printStmt(idx)+",continue\n",common::CheckerName::arrayBound,0,*common::configure);
								i++;	
								continue;

							}
							else if(dyn_cast<ConditionalOperator>(idx)){
								common::printLog( "SimpleExprCheckOnly,ConditionalOperator:"+printStmt(idx)+",continue\n",common::CheckerName::arrayBound,0,*common::configure);
								i++;	
								continue;

							}
							
						}
						if(IndexIgnoreConditionalOperator == true){
							//config.txt IndexIgnoreConditionalOperator is true, if index is a conditionaloperator, we will ignore it
							if(dyn_cast<ConditionalOperator>(idx)){
								common::printLog( "IndexIgnoreConditionalOperator and index is ConditionalOperator:"+printStmt(idx)+",continue\n",common::CheckerName::arrayBound,0,*common::configure);
								i++;
								continue;
							}
						}
						if(isTainted(f,block,stmt,idx))
						{
							vector<AtomicAPInt> tmpAA;
							common::printLog( "index is tainted:"+printStmt(idx)+"\n",common::CheckerName::arrayBound,1,*common::configure);							
							uint64_t apInt=arrSize[i];	
							AtomicAPInt tempoperator;
							tempoperator.op=clang::BinaryOperatorKind(BO_LT);
							tempoperator.lhs=idx;
							tempoperator.rhs=apInt;							
							tmpAA.push_back(tempoperator);
							common::printLog( "array bound:"+int2string(apInt)+"\n",common::CheckerName::arrayBound,1,*common::configure);
							//we need to determine whether there needs a checking like idx>=0. If idx is unsigned, then no; If idx is signed, then yes.
							if(!isUnsigned(idx)){
								common::printLog( "index is  not unsigned:"+printStmt(idx)+"\n",common::CheckerName::arrayBound,1,*common::configure);
								AtomicAPInt tempoperator;
								tempoperator.op=clang::BinaryOperatorKind(BO_GE);
								tempoperator.lhs=idx;
								tempoperator.rhs=0;
								tmpAA.push_back(tempoperator);
							}
							SourceManager *sm;
							sm = &(f->getASTContext().getSourceManager()); 
							string loc = idx->getLocStart().printToString(*sm);	
							common::printLog( "get index location:"+loc+"\n",common::CheckerName::arrayBound,1,*common::configure);							
							as.func=f;
							as.block=block;
							as.stmt=stmt;
							as.index=idx;
							as.location=loc;
							as.originalLocation=loc;
							as.condition=tmpAA;
							as.isLoopBoundChecking=false;							
							as.indexCnt=i;
							as.arrayName=printStmt(base);
							as.arrayExpr=printStmt(ex);
							as.arrayIdx=idx;
							if(ConditionalOperator *co=dyn_cast<ConditionalOperator>(idx->IgnoreImpCasts()->IgnoreParenCasts()))
							{
								 
								// //////cerr<<printStmt(rhs)<<endl;
								Expr* cc=co->getCond();
								
								Expr* rhs=NULL;
								//cerr<<"ConditionalOperator"<<endl;
								for(unsigned j=0;j<as.condition.size();j++)
								{
									bool flag=false;
									Expr* Indx=as.index;
									as.changeIndex(co->getLHS()->IgnoreImpCasts()->IgnoreImpCasts()->IgnoreParenCasts());
									if(checkConditionStmt(cc,as,as.condition[j],true))
									{
										//cerr<<"1"<<endl;
										rhs=co->getRHS()->IgnoreImpCasts()->IgnoreImpCasts()->IgnoreParenCasts();
										flag=true;
									}
									else {
										as.changeIndex(co->getRHS()->IgnoreImpCasts()->IgnoreImpCasts()->IgnoreParenCasts());
										if(checkConditionStmt(cc,as,as.condition[j],false))
										{
											//cerr<<"2"<<endl;
											rhs=co->getLHS()->IgnoreImpCasts()->IgnoreImpCasts()->IgnoreParenCasts();
											flag=true;
										}
									}
									as.changeIndex(Indx);
									if(rhs!=NULL)
									//cerr<<printStmt(rhs)<<endl;
									if(flag)
									{
										if(IntegerLiteral *ILE = dyn_cast<IntegerLiteral>(rhs)) 
										{
											
											uint64_t value=ILE->getValue().getLimitedValue();
											//////////cerr<<value<<endl;
											bool ff=true;
											
											if(as.condition[j].op==clang::BinaryOperatorKind::BO_LT)
											{
												if(value>as.condition[j].rhs)ff=false;
											}
											
											
											if(ff)
											{
												as.condition.erase(as.condition.begin()+j);
												//return 1;
											}
											
										}
										else if(FloatingLiteral *ILE = dyn_cast<FloatingLiteral>(rhs))//-ImplicitCastExpr
										{
											double value=ILE->getValue().convertToDouble();
											//////////cerr<<value<<endl;
											bool ff=true;
											if(as.condition[j].op==clang::BinaryOperatorKind::BO_LT)
											{
												if(value>as.condition[j].rhs)ff=false;
											}
											
											
											if(ff)
											{
												as.condition.erase(as.condition.begin()+j);
												//return 1;
											}
										}
										else if(BinaryOperator *bb=dyn_cast<BinaryOperator>(rhs))
										{
											if(bb->getOpcode()==clang::BinaryOperatorKind::BO_Rem)
											{
												//////cerr<<"Rem after assign"<<endl;
												Expr* rr=bb->getRHS()->IgnoreImpCasts()->IgnoreParenCasts();
												if(IntegerLiteral *ILE = dyn_cast<IntegerLiteral>(rr)) 
												{
													
													uint64_t value=ILE->getValue().getLimitedValue();
													
													if(as.condition[j].op==clang::BinaryOperatorKind::BO_LT)
													{
														if(value<=as.condition[j].rhs)
														{
															
															as.condition.erase(as.condition.begin()+j);
														}
													}
													
													
												}
												//
											}
										}
									}
								}
								
								if(as.condition.size()==0)
									i++;
									continue;
							}
							tmpAS.push_back(as);
						}
						/*else{
							common::printLog( "index is not tainted:"+printStmt(idx)+"\n",common::CheckerName::arrayBound,1,*common::configure);
						}*/
						else{	
							common::printLog( "index is not tainted,is it loop variable? index is:"+printStmt(idx)+"\n",common::CheckerName::arrayBound,1,*common::configure);
							if(ifAnalyeLoopExpr==false) {
								common::printLog( "ifAnalyeLoopExpr false,call AnalyeLoopExpr\n",common::CheckerName::arrayBound,1,*common::configure);
								AnalyeLoopExpr(f,myCFG);
							}
							vector<AtomicAPInt> tmpBounds;							
							common::printLog( "mapToLoopTaintedExpr size:"+int2string(mapToLoopTaintedExpr.size())+"\n",common::CheckerName::arrayBound,1,*common::configure);
							//VarDecl *idxdecl=getVarDecl(idx);
							//if(idxdecl==NULL) continue;
							vector<Expr*> loopBounds;
							std::vector<Expr*> vars=mapToBlockInLoopExpr[block];
							//////cerr<<vars.size()<<endl;
							for(auto ex : vars){
								//////cerr<<"index:"<<idxdecl->getNameAsString()<<"    vars:"<<printStmt(ex)<<endl;
								//if(printStmt(ex)==idxdecl->getNameAsString())
								if(printStmt(ex->IgnoreParenCasts()->IgnoreImpCasts())==printStmt(idx))
								{
									loopBounds=mapToLoopTaintedExpr[ex];
									break;
								}
								// VarDecl *tmp=getVarDecl(ex.first);
								// //tmp->dump();
								// if(tmp==NULL) continue;
								// if(idxdecl==tmp){
								// 	common::printLog( "idxdecl==tmp,idx is a loop variable\n",common::CheckerName::arrayBound,1,*common::configure);
								// 	loopBounds=ex.second;
								// }
							}
							if(loopBounds.size()>0){
								//////cerr<<"loopbound size>0"<<endl;
								//for(auto loopvar:loopBounds){
								auto loopvar=loopBounds[0];
								
								{
									//now that loop variable is not tainted, 
									//we find the index is loop variable,
									//we also push the index into our array boundary information
									//and perform index<size in the loop
									//and perform loopbound<size out the loop
									uint64_t apInt=arrSize[i];										
									AtomicAPInt tempoperator;
									tempoperator.op=clang::BinaryOperatorKind(BO_LT);
									tempoperator.lhs=idx;
									tempoperator.rhs=apInt;
									
									tmpBounds.push_back(tempoperator);
									// if(!isUnsigned((Expr*)loopvar)){
									// 	////////cerr<<"loopbound is not unsigned"<<endl;
									// 	AtomicAPInt tempoperator;
									// 	tempoperator.op=clang::BinaryOperatorKind(BO_GE);
									// 	tempoperator.lhs=(Expr*)loopvar;
									// 	tempoperator.rhs=0;
									// 	tmpBounds.push_back(tempoperator);
									// }
									map<Expr*,CFGBlock*>::iterator itrblock;
									itrblock=mapToCheckBlock.find(loopvar);									
									if(itrblock!=mapToCheckBlock.end()){
										as.loopblock=itrblock->second;
									}
									else {
										common::printLog( "loopvar,mapToCheckBlock not find loopvar!\n",common::CheckerName::arrayBound,1,*common::configure);
										continue;
									}
																		
									SourceManager *sm;
									sm = &(f->getASTContext().getSourceManager()); 
									string loc = idx->getLocStart().printToString(*sm);	
									common::printLog( "get index location:"+loc+"\n",common::CheckerName::arrayBound,1,*common::configure);							
									as.func=f;
									as.block=block;
									as.stmt=stmt;
									as.index=idx;
									as.location=loc;
									as.originalLocation=loc;
									as.condition=tmpBounds;
									as.isLoopBoundChecking=false;
									as.indexCnt=i;
									as.arrayName=printStmt(base);
									as.arrayExpr=printStmt(ex);
									as.arrayIdx=idx;

									//loopvar->dump();
									//////cerr<<"**********************"<<endl;											
									//uint64_t apInt=arrSize[i];										
									//AtomicAPInt tempoperator;
									//tempoperator.op=clang::BinaryOperatorKind(BO_LT);
									//tempoperator.lhs=(Expr*)loopvar;
									//tempoperator.rhs=apInt;
									//
									//tmpBounds.push_back(tempoperator);
									//// if(!isUnsigned((Expr*)loopvar)){
									//// 	////////cerr<<"loopbound is not unsigned"<<endl;
									//// 	AtomicAPInt tempoperator;
									//// 	tempoperator.op=clang::BinaryOperatorKind(BO_GE);
									//// 	tempoperator.lhs=(Expr*)loopvar;
									//// 	tempoperator.rhs=0;
									//// 	tmpBounds.push_back(tempoperator);
									//// }
									//map<Expr*,CFGBlock*>::iterator itrblock;
									//itrblock=mapToCheckBlock.find(loopvar);									
									//if(itrblock!=mapToCheckBlock.end()){
									//	as.block=itrblock->second;
									//}
									//else {
									//	common::printLog( "loopvar,mapToCheckBlock not find loopvar!\n",common::CheckerName::arrayBound,1,*common::configure);
									//	continue;
									//}
									//SourceManager *sm;
									//sm = &(f->getASTContext().getSourceManager()); 
									//string loc = idx->getLocStart().printToString(*sm);
									//as.func=f;
									//as.stmt=as.block->getTerminatorCondition();
									//common::printLog( "loopvar,as.stmt=as.block->getTerminatorCondition() is:"+printStmt(as.stmt)+"\n",common::CheckerName::arrayBound,1,*common::configure);
									//as.index=loopvar;
									//as.location=loc;
									//as.condition=tmpBounds;
									//as.isLoopBoundChecking=true;
									//as.indexCnt=i;
									//as.arrayExpr=printStmt(ex);
									//as.arrayIdx=idx;
									//cerr<<"Loop Var"<<endl;
									//Expr* tmp=loopvar->IgnoreImpCasts()->IgnoreImpCasts()->IgnoreParenCasts();
									//tmp->dump();
									//if(ConditionalOperator *co=dyn_cast<ConditionalOperator>(loopvar->IgnoreImpCasts()->IgnoreParenCasts()))
									//{
									//	 
									//	// //////cerr<<printStmt(rhs)<<endl;
									//	Expr* cc=co->getCond();
									//	
									//	Expr* rhs=NULL;
									//	//cerr<<"ConditionalOperator"<<endl;
									//	for(unsigned j=0;j<as.condition.size();j++)
									//	{
									//		bool flag=false;
									//		Expr* Indx=as.index;
									//		as.changeIndex(co->getLHS()->IgnoreImpCasts()->IgnoreImpCasts()->IgnoreParenCasts());
									//		if(checkConditionStmt(cc,as,as.condition[j],true))
									//		{
									//			//cerr<<"1"<<endl;
									//			rhs=co->getRHS()->IgnoreImpCasts()->IgnoreImpCasts()->IgnoreParenCasts();
									//			flag=true;
									//		}
									//		else {
									//			as.changeIndex(co->getRHS()->IgnoreImpCasts()->IgnoreImpCasts()->IgnoreParenCasts());
									//			if(checkConditionStmt(cc,as,as.condition[j],false))
									//			{
									//				//cerr<<"2"<<endl;
									//				rhs=co->getLHS()->IgnoreImpCasts()->IgnoreImpCasts()->IgnoreParenCasts();
									//				flag=true;
									//			}
									//		}
									//		as.changeIndex(Indx);
									//		if(rhs!=NULL)
									//		//cerr<<printStmt(rhs)<<endl;
									//		if(flag)
									//		{
									//			if(IntegerLiteral *ILE = dyn_cast<IntegerLiteral>(rhs)) 
									//			{
									//				
									//				uint64_t value=ILE->getValue().getLimitedValue();
									//				//////////cerr<<value<<endl;
									//				bool ff=true;
									//				
									//				if(as.condition[j].op==clang::BinaryOperatorKind::BO_LT)
									//				{
									//					if(value>as.condition[j].rhs)ff=false;
									//				}
									//				
									//				
									//				if(ff)
									//				{
									//					as.condition.erase(as.condition.begin()+j);
									//					//return 1;
									//				}
									//				
									//			}
									//			else if(FloatingLiteral *ILE = dyn_cast<FloatingLiteral>(rhs))//-ImplicitCastExpr
									//			{
									//				double value=ILE->getValue().convertToDouble();
									//				//////////cerr<<value<<endl;
									//				bool ff=true;
									//				if(as.condition[j].op==clang::BinaryOperatorKind::BO_LT)
									//				{
									//					if(value>as.condition[j].rhs)ff=false;
									//				}
									//				
									//				
									//				if(ff)
									//				{
									//					as.condition.erase(as.condition.begin()+j);
									//					//return 1;
									//				}
									//			}
									//			else if(BinaryOperator *bb=dyn_cast<BinaryOperator>(rhs))
									//			{
									//				if(bb->getOpcode()==clang::BinaryOperatorKind::BO_Rem)
									//				{
									//					//////cerr<<"Rem after assign"<<endl;
									//					Expr* rr=bb->getRHS()->IgnoreImpCasts();
									//					if(IntegerLiteral *ILE = dyn_cast<IntegerLiteral>(rr)) 
									//					{
									//						
									//						uint64_t value=ILE->getValue().getLimitedValue();
									//						
									//						if(as.condition[j].op==clang::BinaryOperatorKind::BO_LT)
									//						{
									//							if(value<=as.condition[j].rhs)
									//							{
									//								
									//								as.condition.erase(as.condition.begin()+j);
									//							}
									//						}
									//						
									//						
									//					}
									//					//
									//				}
									//			}
									//		}
									//	}
									//	
									//	if(as.condition.size()==0)
									//		continue;
									//}
									//cerr<<"generate Script:"<<printStmt(as.index)<<endl;
									tmpAS.push_back(as);
									
								}
							}	
							else{
								common::printLog( "there's no loopvar,loopbound, index is not loopvar",common::CheckerName::arrayBound,1,*common::configure);
							}
						}
						
						i++;
					}
				}
			}
		}
	}
	common::printLog( "getArraySubscriptInExpr end!\n",common::CheckerName::arrayBound,3,*common::configure);
	return tmpAS;
}

vector<ArraySubscript> ArrayOutOfBoundsCheckerV4::getArraySubscript(FunctionDecl *func,CFG * myCFG){
	//this function will locate the targeted array index, and get the array boundary information
	common::printLog( "get array information begin:\n",common::CheckerName::arrayBound,4,*common::configure);
	vector<ArraySubscript> tmpAS;
	for (CFGBlock *block : *myCFG) {
//			block->dump();
		for (auto& element : *block) {
			if (element.getKind() == CFGStmt::Statement) {
				Stmt *stmt = const_cast<Stmt*>(element.castAs<CFGStmt>().getStmt());
				//stmt->dump();
				FindArraySubscriptExpr visitor;
				if(stmt!=nullptr){
					visitor.TraverseStmt(stmt);
					const auto &arrayExprs = visitor.getArrayExprs();
					if(!arrayExprs.empty()){
						//#if __DEBUG							
						//	////llvm::errs() <<"arrayExpr not empty:--------------------------\n first:\n ";
						//#endif
						
						for(auto& arrayExpr:arrayExprs){  
							//arrayPositions.push_back(ExprPosition(fd,block,stmt,arrayExpr));  
							common::printLog( "arrayExpr:"+printStmt(arrayExpr)+"\n",common::CheckerName::arrayBound,0,*common::configure);
							vector<ArraySubscript> tmp = getArraySubscriptInExpr(arrayExpr,func,block,stmt,myCFG);

							if(tmp.size()>0){
								vector<ArraySubscript>::iterator itrblock;
								for(auto & as:tmp){
									bool iffind=false;
									for(auto& ass:tmpAS){
										//if(equal(ass,as)){
									//common::printLog( "Remove the same array bound check checking on the same line\n",common::CheckerName::arrayBound,1,*common::configure);										
									//string assloc;
									//assloc.assign(ass.location.c_str(),ass.location.find_last_of(':'));
									//common::printLog( "Already push ass at line: "+assloc+", "+ass.arrayExpr+"\n",common::CheckerName::arrayBound,1,*common::configure);
									//string asloc;
									//asloc.assign(as.location.c_str(),as.location.find_last_of(':'));
									//common::printLog( "Need to push as at line: "+asloc+", "+as.arrayExpr+"\n",common::CheckerName::arrayBound,1,*common::configure);
										//if(ass.arrayName==as.arrayName&&printStmt(ass.index)==printStmt(as.index)&&ass.indexCnt==as.indexCnt&&ass.isLoopBoundChecking==as.isLoopBoundChecking&&ass.location==as.location&&ass.arrayExpr==as.arrayExpr){
										//if(ass.arrayName==as.arrayName&&printStmt(ass.index)==printStmt(as.index)&&ass.indexCnt==as.indexCnt&&assloc==asloc){
										if(ass.arrayName==as.arrayName&&printStmt(ass.index)==printStmt(as.index)&&ass.indexCnt==as.indexCnt){
											iffind=true;
											common::printLog( "as is found in tmpAS, just throw away\n",common::CheckerName::arrayBound,1,*common::configure);
											break;
										}
									}
									if(!iffind){
										common::printLog( "as is not found in tmpAS,push to tmpAS\n",common::CheckerName::arrayBound,1,*common::configure);
										//as  is not found in tmpAS
										tmpAS.push_back(as);
									}
									
								}
								//tmpAS.insert(tmpAS.begin(),tmp.begin(),tmp.end());
							}
						}
					}
				}
			}
		}
	}
//#ifdef __DEBUG 
//	//////cerr<<"tmpAS size:"<<tmpAS.size()<<endl;
//	for(auto tt:tmpAS){
//		////////cerr<<"func:"<<printStmt((Stmt*)tt.func)<<endl;
//		//////cerr<<"index:"<<printStmt((Stmt*)tt.index)<<endl;
//		//////cerr<<"location:"<<tt.location<<endl;
//	}
//#endif
	common::printLog( "get array information end!\n",common::CheckerName::arrayBound,4,*common::configure);
	return tmpAS;
}
//vector<ArraySubscript> ArrayOutOfBoundsCheckerV4::getArraySubscript(std::shared_ptr<clang::CFG> cfg,FunctionDecl *f){
//	//this function can get array subscript information in a function.
//	vector<ArraySubscript> tmpAS;	
//	//std:://////cerr<<"------------get array information begin---------------------"<<std::endl;		
//	for(CFG::iterator iter=cfg->begin();iter!=cfg->end();++iter){			
//		CFGBlock* block=*iter;
//		for(CFGBlock::iterator iterblock=block->begin();iterblock!=block->end();++iterblock)
//		{
//			CFGElement element=(*iterblock);
//			if(element.getKind()==CFGElement::Kind::Statement){
//				const Stmt* it=((CFGStmt*)&element)->getStmt();
//				switch (it->getStmtClass()) {
//					default:
//						////////cerr<<"default;";
//						break;
//					case Stmt::CompoundStmtClass:
//						////////cerr<<"CompoundStmt Stmt;";
//						//process...	
//						break;
//					case Stmt::IfStmtClass:
//						////////cerr<<"If Stmt;";
//						//process...	
//						break;
//					case Stmt::UnaryOperatorClass:{
//						////////cerr<<"UnaryOperator Stmt;"<<endl;
//						const UnaryOperator *uo = cast<UnaryOperator>(it);
//						//checkStmt((Expr*)uo,f,block_in);
//						vector<ArraySubscript> tmp=getArraySubscriptInExpr((Expr*)uo,f,block,(Stmt*)it);
//						if(tmp.size()>0){
//							tmpAS.insert(tmpAS.begin(),tmp.begin(),tmp.end());
//						}
//						break;
//					}
//					case Stmt::BinaryOperatorClass:{	
//						////////cerr<<"BinaryOperator Stmt;"<<endl;
//						//process...
//						const BinaryOperator *bo = cast<BinaryOperator>(it);
//						//checkStmt((Expr*)bo,f,block_in);	
//						vector<ArraySubscript> tmp=getArraySubscriptInExpr((Expr*)bo,f,block,(Stmt*)it);
//						////////cerr<<"tmp size:"<<tmp.size()<<endl;
//						if(tmp.size()>0){
//							tmpAS.insert(tmpAS.begin(),tmp.begin(),tmp.end());
//						}
//						break;
//					}
//					case Stmt::WhileStmtClass:
//						////////cerr<<"While Stmt;";
//						break;
//					case Stmt::ForStmtClass:
//						////////cerr<<"For Stmt;";
//						break;
//					case Stmt::CallExprClass:{
//						////////cerr<<"CallExpr;"<<endl;
//						const CallExpr *callexpr = cast<CallExpr>(it);
//						for(unsigned i=0;i<callexpr->getNumArgs();i++){
//							const Expr *arg = callexpr->getArg(i);
//							//checkStmt((Expr*)arg,f,block_in);
//							vector<ArraySubscript> tmp=getArraySubscriptInExpr((Expr*)arg,f,block,(Stmt*)it);
//							if(tmp.size()>0){
//								tmpAS.insert(tmpAS.begin(),tmp.begin(),tmp.end());
//							}
//						}
//						break;
//					}
//				}
//			}
//		}
//	}
//	//std:://////cerr<<"------------get array information end -----------------------"<<std::endl;
//	return tmpAS;
//}

template <typename T>
std::string int2string(const T& i)
{
	std::ostringstream oss;
	oss<<i;
	return oss.str();
	
}
string getInfo(Expr* expr)
{	
	LangOptions L0;
	L0.CPlusPlus=1;
	std::string buffer1;
	llvm::raw_string_ostream strout1(buffer1);
	expr->printPretty(strout1,nullptr,PrintingPolicy(L0));
	return ""+strout1.str()+"";
}
string getInfo(uint64_t expr)
{				
	string result=int2string(expr);
	return result;
}

string getInfo(AtomicAPInt expr)
{
	string result="";
	int op=expr.op;
		switch(op)
		{
			case 9://LT
			{
			result=getInfo(expr.getLHS())+" < "+getInfo(expr.getRHS());
			break;
			}
			case 10://GT
			{
			result=getInfo(expr.getLHS())+" > "+getInfo(expr.getRHS());
			break;
			}
			case 11://LE
			{
				result=getInfo(expr.getLHS())+" <= "+getInfo(expr.getRHS());
			break;
			}
			case 12://GE
			{
				result=getInfo(expr.getLHS())+" >= "+getInfo(expr.getRHS());
			break;
			}
			case 13://EQ
			{
				result=getInfo(expr.getLHS())+" == "+getInfo(expr.getRHS());
			break;
			}
			case 14://NE
			{
				result=getInfo(expr.getLHS())+" != "+getInfo(expr.getRHS());
			break;
			}
		}
	return result;
}
void ArraySubscript::print(){
	cerr<<"ArraySubscript location: "<<location<<endl;
	cerr<<"ArraySubscript Index: "<<printStmt(index)<<endl;
	cerr<<"need condition: "<<endl;
	for(auto c:condition){
		cerr<<getInfo(c)<<endl;
	}
}

int ArrayOutOfBoundsCheckerV4::throughStmt(Stmt* it,ArraySubscript& con,CFGBlock* block)//0:next  1:ok   -1:error
{
	if(it->getStmtClass()==Stmt::CompoundAssignOperatorClass)//+=,-=,*=,/=,%=
	{
		const CompoundAssignOperator *co = cast<CompoundAssignOperator>(it);
		common::printLog( "CompoundAssignOperatorClass\n",common::CheckerName::arrayBound,0,*common::configure);	
		//////cerr<<"CompoundAssignOperatorClass"<<endl;
		if(co->getOpcode()== clang::BinaryOperatorKind::BO_RemAssign )
		{
			//////cerr<<"RemAssign"<<endl;
			//co->dump();
			Expr* lhs=co->getLHS()->IgnoreParenCasts()->IgnoreImpCasts();
			Expr* rhs=co->getRHS()->IgnoreParenCasts()->IgnoreImpCasts();

			if(printStmt(lhs)==printStmt(con.index->IgnoreImpCasts()->IgnoreParenCasts()))
			{
				
				if(IntegerLiteral *ILE = dyn_cast<IntegerLiteral>(rhs)) 
				{
					
					uint64_t value=ILE->getValue().getLimitedValue();
					common::printLog( "IntegerLiteral:"+int2string(value)+"\n",common::CheckerName::arrayBound,0,*common::configure);	
							
					for(unsigned j=0;j<con.condition.size();j++)
					{
						AtomicAPInt temp=con.condition[j];
						if(temp.op==clang::BinaryOperatorKind::BO_LT)
						{
							if(value>temp.rhs)
								return 0;
							else
							{
								//con.condition.erase(con.condition.begin()+j);
								if(con.condition.size()==1)
									return 1;
								else
								{
									con.condition.erase(con.condition.begin()+j);
									return 0;
								}
							}

						}
						
					}
				}
				else if(printStmt(rhs).find("sizeof")!=std::string::npos)
				{
					//int a[5][6];
					//cerr<<"test sizeof:"<<sizeof(a)<<endl;
					common::printLog( printStmt(lhs)+"%= "+printStmt(rhs)+"\n",common::CheckerName::arrayBound,0,*common::configure);	
					//rhs->dump();
					//con.condition.clear();
					return 1;
				}
				else if(DeclRefExpr * ref=dyn_cast<DeclRefExpr>(rhs)){
					
					if(EnumConstantDecl * EC=dyn_cast<EnumConstantDecl>(ref->getDecl())){
						uint64_t value=EC->getInitVal().getLimitedValue ();
						common::printLog( "EnumConstantDecl:"+int2string(value)+"\n",common::CheckerName::arrayBound,0,*common::configure);	
							
						for(unsigned j=0;j<con.condition.size();j++)
						{
							AtomicAPInt temp=con.condition[j];
							if(temp.op==clang::BinaryOperatorKind::BO_LT)
							{
								if(value>temp.rhs)
									return 0;
								else
								{
									//con.condition.erase(con.condition.begin()+j);
									if(con.condition.size()==1)
										return 1;
									else
									{
										con.condition.erase(con.condition.begin()+j);
										return 0;
									}
								}

							}
							
						}
					}
				}
			}
		}
	}
	else if(it->getStmtClass()==Stmt::BinaryOperatorClass)//
	{
		const BinaryOperator *bo = cast<BinaryOperator>(it);
		if(bo->getOpcode()== clang::BinaryOperatorKind::BO_Assign )
		{
			Expr* lhs=bo->getLHS()->IgnoreImpCasts()->IgnoreParenCasts();
			Expr* rhs=bo->getRHS()->IgnoreImpCasts()->IgnoreParenCasts();
			////cerr<<"!!!"<<endl;
			//cout<<"Assign Stmt"<<endl;
			//cout<<printStmt(lhs)<<"  "<<printStmt(con.index->IgnoreImpCasts()->IgnoreParenCasts())<<endl;
			//bo->dump();
			
			if(printStmt(lhs)==printStmt(con.index->IgnoreImpCasts()->IgnoreParenCasts())){
				//////cerr<<"Equal ihs"<<endl;
				//cerr<<printStmt(it)<<endl;
				common::printLog("Assign operator: "+printStmt(rhs)+" to "+printStmt(lhs)+"\n",common::CheckerName::arrayBound,0,*common::configure);
				if(DebugMode)
						rhs->dump();
				if(ConditionalOperator *co=dyn_cast<ConditionalOperator>(rhs))
				{
					common::printLog("Encountering conditionaloperator:\nlhs:"+printStmt(lhs)+"\ncon.index:"+printStmt(con.index)+"\n",common::CheckerName::arrayBound,0,*common::configure);
					//////cerr<<"ConditionalOperator"<<endl;

					//////cerr<<printStmt(rhs)<<endl;
					common::printLog( "=ConditionalOperator:"+printStmt(lhs)+"= "+printStmt(rhs)+"\n",common::CheckerName::arrayBound,0,*common::configure);
					Expr* cc=co->getCond()->IgnoreImpCasts()->IgnoreParenCasts();
					//cc->dump();
					for(unsigned j=0;j<con.condition.size();j++)
					{
						AtomicAPInt temp=con.condition[j];
						//////cerr<<printStmt(temp.lhs)<<" op "<<temp.rhs<<endl;
						//////cerr<<"Cond:"<<printStmt(cc)<<endl;
						bool flag=false;
						Expr* Indx=con.index;
						con.changeIndex(co->getLHS()->IgnoreImpCasts()->IgnoreParenCasts());
						if(checkConditionStmt(cc,con,con.condition[j],true))
						{
							////cerr<<"1"<<endl;
							rhs=co->getRHS()->IgnoreImpCasts()->IgnoreParenCasts();
							common::printLog( "true ConditionalOperator, co rhs:"+printStmt(rhs)+"\n",common::CheckerName::arrayBound,0,*common::configure);
							flag=true;
						}
						else {
							con.changeIndex(co->getRHS()->IgnoreImpCasts()->IgnoreParenCasts());
							if(checkConditionStmt(cc,con,con.condition[j],false))
							{
								////cerr<<"2"<<endl;
								rhs=co->getLHS()->IgnoreImpCasts()->IgnoreParenCasts();
								flag=true;
								common::printLog( "false ConditionalOperator, co rhs:"+printStmt(rhs)+"\n",common::CheckerName::arrayBound,0,*common::configure);
							}
						}
						con.changeIndex(Indx);
						if(flag)
						{
							if(IntegerLiteral *ILE = dyn_cast<IntegerLiteral>(rhs)) 
							{
								
								uint64_t value=ILE->getValue().getLimitedValue();
								common::printLog( "IntegerLiteral:"+int2string(value)+"\n",common::CheckerName::arrayBound,0,*common::configure);
								bool ff=true;
								
								if(temp.op==clang::BinaryOperatorKind::BO_LT)
								{
									//for(i<n) a[i], need n<=size
									//for(i<=n)a[i], need n<=size
									//if(value>=temp.rhs)ff=false;
									if(value>=temp.rhs)ff=false;
								}
								if(temp.op==clang::BinaryOperatorKind::BO_GE)
								{
									if(value<temp.rhs)ff=false;
								}
								
								if(ff)
								{
									con.condition.erase(con.condition.begin()+j);
									//return 1;
								}
								
							}
							else if(FloatingLiteral *ILE = dyn_cast<FloatingLiteral>(rhs))//-ImplicitCastExpr
							{
								double value=ILE->getValue().convertToDouble();
								//////////cerr<<value<<endl;
								bool ff=true;
								if(temp.op==clang::BinaryOperatorKind::BO_LT)
								{
									if(value>=temp.rhs)ff=false;
								}
								if(temp.op==clang::BinaryOperatorKind::BO_GE)
								{
									if(value<temp.rhs)ff=false;
								}
								
								if(ff)
								{
									con.condition.erase(con.condition.begin()+j);
									//return 1;
								}
							}
							else if(BinaryOperator *bb=dyn_cast<BinaryOperator>(rhs))
							{
								if(bb->getOpcode()==clang::BinaryOperatorKind::BO_Rem)
								{
									//////cerr<<"Rem after assign"<<endl;
									Expr* rr=bb->getRHS()->IgnoreImpCasts()->IgnoreParenCasts();
									if(IntegerLiteral *ILE = dyn_cast<IntegerLiteral>(rr)) 
									{
										
										uint64_t value=ILE->getValue().getLimitedValue();
										
										if(temp.op==clang::BinaryOperatorKind::BO_LT)
										{
											if(value<=temp.rhs)
											{
												
												con.condition.erase(con.condition.begin()+j);
											}

										}
										
										
									}
									//
								}
								else if(bb->getOpcode()==clang::BinaryOperatorKind::BO_And)
								{
									Expr* rr=bb->getRHS()->IgnoreImpCasts()->IgnoreParenCasts();
									if(IntegerLiteral *ILE = dyn_cast<IntegerLiteral>(rr)) 
									{
										
										uint64_t value=ILE->getValue().getLimitedValue();
										
										if(temp.op==clang::BinaryOperatorKind::BO_LT)
										{
											if(value<=temp.rhs)
											{
												
												con.condition.erase(con.condition.begin()+j);
											}

										}
										
										
									}
								}
							}
						}

					}
					if(con.condition.size()>0)return 0;
					else return 1;
					
				}
				//bo->dump();
				if(IntegerLiteral *ILE = dyn_cast<IntegerLiteral>(rhs)) 
				{
					
					uint64_t value=ILE->getValue().getLimitedValue();
					//////////cerr<<value<<endl;
					bool flag=true;
					for(unsigned j=0;j<con.condition.size();j++)
					{
						AtomicAPInt temp=con.condition[j];
						if(temp.op==clang::BinaryOperatorKind::BO_LT)
						{
							if(value>=temp.rhs)flag=false;
						}
						if(temp.op==clang::BinaryOperatorKind::BO_GE)
						{
							if(value<temp.rhs)flag=false;
						}
					}
					if(flag)
					{
						//list.erase(list.begin()+i);
						//i--;
						//while(!s.empty()) s.pop();
						return 1;
					}
					else
					{
						

						return -1;
						//break;
					}
				}
				else if(printStmt(rhs).find("sizeof")!=std::string::npos)
				{
					common::printLog( printStmt(lhs)+"= "+printStmt(rhs)+"\n",common::CheckerName::arrayBound,0,*common::configure);	
					//rhs->dump();
					//con.condition.clear();
					return 1;
				}
				else if(UnaryOperator *ILE = dyn_cast<UnaryOperator>(rhs)) 
				{
					if(ILE->getOpcode()==clang::UnaryOperatorKind::UO_Minus)
					{
						return -1;
					}
				}
				else if(FloatingLiteral *ILE = dyn_cast<FloatingLiteral>(rhs))//-ImplicitCastExpr
				{
					double value=ILE->getValue().convertToDouble();
					//////////cerr<<value<<endl;
					bool flag=true;
					for(unsigned j=0;j<con.condition.size();j++)
					{
						AtomicAPInt temp=con.condition[j];
						if(temp.op==clang::BinaryOperatorKind::BO_LT)
						{
							if(value>=temp.rhs)flag=false;
						}
						if(temp.op==clang::BinaryOperatorKind::BO_GE)
						{
							if(value<temp.rhs)flag=false;
						}
					}
					if(flag)
					{
						return 1;
					}
					else
					{
						return -1;
					}
				}
				else if(DeclRefExpr * ref=dyn_cast<DeclRefExpr>(rhs)){
					
					if(EnumConstantDecl * EC=dyn_cast<EnumConstantDecl>(ref->getDecl())){
						uint64_t value=EC->getInitVal().getLimitedValue ();
						common::printLog( "EnumConstantDecl:"+int2string(value)+"\n",common::CheckerName::arrayBound,0,*common::configure);	
							
						for(unsigned j=0;j<con.condition.size();j++)
						{
							AtomicAPInt temp=con.condition[j];
							if(temp.op==clang::BinaryOperatorKind::BO_LT)
							{
								if(value>temp.rhs)
									return 0;
								else
								{
									//con.condition.erase(con.condition.begin()+j);
									if(con.condition.size()==1)
										return 1;
									else
									{
										con.condition.erase(con.condition.begin()+j);
										return 0;
									}
								}

							}
							
						}
					}
					else
					{
						if(SimpleExprCheckOnly){
							if(dyn_cast<BinaryOperator>(rhs)){
								common::printLog( "SimpleExprCheckOnly,BinaryOperator:"+printStmt(rhs)+",continue\n",common::CheckerName::arrayBound,0,*common::configure);								
								return 1;
							}
							else if(dyn_cast<UnaryOperator>(rhs)){
								common::printLog( "SimpleExprCheckOnly,UnaryOperator:"+printStmt(rhs)+",continue\n",common::CheckerName::arrayBound,0,*common::configure);
								return 1;

							}
							else if(dyn_cast<CallExpr>(rhs)){
								common::printLog( "SimpleExprCheckOnly,callexpr:"+printStmt(rhs)+",continue\n",common::CheckerName::arrayBound,0,*common::configure);
								return 1;

							}						
							
						}
						common::printLog( "change index to "+printStmt(rhs),common::CheckerName::arrayBound,0,*common::configure);
						con.changeIndex(rhs);
						return 0;
					}
				}
				else if(BinaryOperator *bb=dyn_cast<BinaryOperator>(rhs))
				{
					if(bb->getOpcode()==clang::BinaryOperatorKind::BO_Rem)
					{
						//////cerr<<"Rem after assign"<<endl;
						Expr* rr=bb->getRHS()->IgnoreImpCasts()->IgnoreParenCasts();
						if(IntegerLiteral *ILE = dyn_cast<IntegerLiteral>(rr)) 
						{
							
							uint64_t value=ILE->getValue().getLimitedValue();
							for(unsigned j=0;j<con.condition.size();j++)
							{
								AtomicAPInt temp=con.condition[j];
								if(temp.op==clang::BinaryOperatorKind::BO_LT)
								{
									if(value>temp.rhs)
										return 0;
									else
									{
										//con.condition.erase(con.condition.begin()+j);
										if(con.condition.size()==1)
											return 1;
										else
										{
											con.condition.erase(con.condition.begin()+j);
											return 0;
										}
									}

								}
								
							}
						}
						//
					}
					else if(bb->getOpcode()==clang::BinaryOperatorKind::BO_And)
					{
						//cerr<<"And Operator"<<endl;
						Expr* rr=bb->getRHS()->IgnoreImpCasts()->IgnoreParenCasts();
						if(IntegerLiteral *ILE = dyn_cast<IntegerLiteral>(rr)) 
						{
							
							uint64_t value=ILE->getValue().getLimitedValue();
							for(unsigned j=0;j<con.condition.size();j++)
							{
								AtomicAPInt temp=con.condition[j];
								if(temp.op==clang::BinaryOperatorKind::BO_LT)
								{
									if(value>temp.rhs)
										return 0;
									else
									{
										//con.condition.erase(con.condition.begin()+j);
										if(con.condition.size()==1)
											return 1;
										else
										{
											con.condition.erase(con.condition.begin()+j);
											return 0;
										}
									}

								}
								
							}
						}
					}
					else
					{
						con.changeIndex(rhs);
						return 0;
					}
				}
				else
				{
					
					auto rhs_value = calculateExpr(rhs);
					if(rhs_value.find(true) != rhs_value.end()){
						int value=rhs_value[true] ;
						bool flag=true;
						for(unsigned j=0;j<con.condition.size();j++)
						{
							AtomicAPInt temp=con.condition[j];
							if(temp.op==clang::BinaryOperatorKind::BO_LT)
							{
								if(value>=temp.rhs)flag=false;
							}
							if(temp.op==clang::BinaryOperatorKind::BO_GE)
							{
								if(value<temp.rhs)flag=false;
							}
						}
						if(flag)
						{
							//list.erase(list.begin()+i);
							//i--;
							//while(!s.empty()) s.pop();
							return 1;
						}
						else
						{
							

							return -1;
							//break;
						}
					}
					else
					{
						if(SimpleExprCheckOnly){
							if(dyn_cast<BinaryOperator>(rhs)){
								common::printLog( "SimpleExprCheckOnly,BinaryOperator:"+printStmt(rhs)+",continue\n",common::CheckerName::arrayBound,0,*common::configure);								
								return 1;
							}
							else if(dyn_cast<UnaryOperator>(rhs)){
								common::printLog( "SimpleExprCheckOnly,UnaryOperator:"+printStmt(rhs)+",continue\n",common::CheckerName::arrayBound,0,*common::configure);
								return 1;

							}
							else if(dyn_cast<CallExpr>(rhs)){
								common::printLog( "SimpleExprCheckOnly,callexpr:"+printStmt(rhs)+",continue\n",common::CheckerName::arrayBound,0,*common::configure);
								return 1;

							}						
							
						}
						common::printLog( "change index to "+printStmt(rhs),common::CheckerName::arrayBound,0,*common::configure);
						con.changeIndex(rhs);
						return 0;
					}
				}
			}
			else if(printStmt(rhs)==printStmt(con.index->IgnoreImpCasts()->IgnoreParenCasts()))
			{
				con.changeIndex(lhs);
				CFGBlock* tmp=block;
				while(tmp!=nullptr&&tmp->getTerminator()!=nullptr
					&&tmp->getTerminator()->getStmtClass()!=Stmt::IfStmtClass
					&&tmp->getTerminator()->getStmtClass()!=Stmt::ForStmtClass&&tmp->getTerminator()->getStmtClass()!=Stmt::WhileStmtClass
					&&tmp->getTerminator()->getStmtClass()!=Stmt::DoStmtClass)
				{
					if(tmp->succ_begin()!=nullptr)
						tmp=*(tmp->succ_begin());
					else
						break;
				}
				if(tmp!=nullptr&&tmp->getTerminator() != nullptr&&tmp->getTerminator()->getStmtClass()==Stmt::IfStmtClass)
				{
					Expr* cond=((IfStmt*)tmp->getTerminator().getStmt())->getCond();	
					bool T=true;
					//cout<<"ID:"<<con.ID<<endl;
					for(CFGBlock::succ_iterator succ_iter=block->succ_begin();succ_iter!=block->succ_end();++succ_iter){
						CFGBlock* succ=*succ_iter;
						//succ->dump();
						//common::printLog( "succ block ID"+int2string(succ->getBlockID())+"\n",common::CheckerName::arrayBound,1,*common::configure);
						if(succ==NULL)continue;
						std::vector<ArraySubscript> tmp=mapToBlockOut[succ];
						//cout<<"size:"<<tmp.size()<<endl;
						for(ArraySubscript s : tmp)
						{
							//cout<<"SID:"<<s.ID<<endl;
							//s.print();
							if(s.ID == con.ID)
							{
								T=succ_iter==block->succ_begin();
								break;
							}
						}					
					}
					//cout<<T<<endl;
					vector<AtomicAPInt> cons=con.condition;
					for(unsigned j=0;j<cons.size();j++)
					{
						common::printLog( "cons j: "+int2string(j)+"\n",common::CheckerName::arrayBound,0,*common::configure);
						int r;
						if(SimpleCheckEnable){r=checkConditionStmtHasIdx(block,tmp->getTerminator(),cond,con,cons[j],T,false);}
						else{r=checkConditionStmt(cond,con,cons[j],T);}
						//cout<<r<<endl;
						if(r)
						{
							common::printLog( "checkConditionStmt true,find if check: "+printStmt(cond)+", "+int2string(T)+"\n",common::CheckerName::arrayBound,0,*common::configure);
							cons.erase(cons.begin()+j);
							j--;
						}
					}
					if(cons.size()==0)
					{
						//cout<<"removed"<<endl;
						//con.resetIndex();
						return 1;	
					}
				}
				con.resetIndex();
			}
		}
	}
	else if(DeclStmt *decl = dyn_cast<DeclStmt>(it))
	{
		if(VarDecl* var = dyn_cast<VarDecl>(decl->getSingleDecl()))
		{
			if(var->getInit() != NULL)
			{
				//var->getInit()->dump();
			}
		}
	}
	else if(it->getStmtClass()==Stmt::CallExprClass)//Call
	{

	}
	return 0;
}

//bool ArrayOutOfBoundsCheckerV4::CheckConditionExpr(Expr* expr,string op,uint64_t cons) //true : remove this cons
//{
//	//cerr<<"op"<<op<<endl;
//	common::printLog( "CheckConditionExpr expr:"+printStmt(expr)+"\n",common::CheckerName::arrayBound,0,*common::configure);
//	if(IntegerLiteral *ILE = dyn_cast<IntegerLiteral>(expr)) 
//	{
//		
//		uint64_t value=ILE->getValue().getLimitedValue();
//		//cerr<<value<<endl;
//		//if(value<0)return false;
//		if(op=="<")
//		{
//			return value<=cons;
//		}
//		else if(op==">=")
//		{
//			return value>=cons;
//		}
//
//	}
//	else if(UnaryOperator *ILE = dyn_cast<UnaryOperator>(expr)) 
//	{
//		common::printLog( "CheckConditionExpr UnaryOperator:"+printStmt(expr)+"\n",common::CheckerName::arrayBound,0,*common::configure);
//		return false;
//	}
//	else if(FloatingLiteral *ILE = dyn_cast<FloatingLiteral>(expr))//-ImplicitCastExpr
//	{
//		int value=(int)ILE->getValue().convertToDouble();
//		//if(value<0)return false;
//		if(op=="<")
//		{
//			return value<=cons;
//		}
//		else if(op==">=")
//		{
//			return value>=cons;
//		}
//	}
//	else 
//	{
//		common::printLog( "CheckConditionExpr:"+printStmt(expr)+"\n",common::CheckerName::arrayBound,0,*common::configure);
//		auto rhs_value = calculateExpr(expr);
//		if(rhs_value.find(true) != rhs_value.end()){
//			common::printLog( "calculateExpr find true\n",common::CheckerName::arrayBound,0,*common::configure);
//			int value=rhs_value[true] ;
//			//cerr<<value<<endl;
//			if(op=="<")
//			{
//				return value<=cons;
//			}
//			else if(op==">=")
//			{
//				return value>=cons;
//			}
//		}
//	}
//	return false;
//}
bool ArrayOutOfBoundsCheckerV4::CheckConditionExpr(Expr* expr,string op,uint64_t cons) //true : remove this cons
{
	//cerr<<"op"<<op<<endl;
	common::printLog( "CheckConditionExpr expr:"+printStmt(expr)+"\n",common::CheckerName::arrayBound,0,*common::configure);
	if(IntegerLiteral *ILE = dyn_cast<IntegerLiteral>(expr)) 
	{
		
		uint64_t value=ILE->getValue().getLimitedValue();
		//cerr<<value<<endl;
		//if(value<0)return false;
		if(op=="<")
		{
			return value<=cons;
		}
		else if(op==">=")
		{
			return value>=cons;
		}

	}
	else if(dyn_cast<UnaryOperator>(expr)) 
	{
		common::printLog( "CheckConditionExpr UnaryOperator:"+printStmt(expr)+"\n",common::CheckerName::arrayBound,0,*common::configure);
		return false;
	}
	else if(FloatingLiteral *ILE = dyn_cast<FloatingLiteral>(expr))//-ImplicitCastExpr
	{
		int value=(int)ILE->getValue().convertToDouble();
		//if(value<0)return false;
		if(op=="<")
		{
			return value<=cons;
		}
		else if(op==">=")
		{
			return value>=cons;
		}
	}
	else if(DeclRefExpr * ref=dyn_cast<DeclRefExpr>(expr)){
					
		if(EnumConstantDecl * EC=dyn_cast<EnumConstantDecl>(ref->getDecl())){
			uint64_t value=EC->getInitVal().getLimitedValue ();
			if(op=="<")
			{
				return value<=cons;
			}
			else if(op==">=")
			{
				return value>=cons;
			}
		}
	}
	else 
	{
		common::printLog( "CheckConditionExpr:"+printStmt(expr)+"\n",common::CheckerName::arrayBound,0,*common::configure);
		auto rhs_value = calculateExpr(expr);
		if(rhs_value.find(true) != rhs_value.end()){
			common::printLog( "calculateExpr find true\n",common::CheckerName::arrayBound,0,*common::configure);
			int value=rhs_value[true] ;
			//cerr<<value<<endl;
			if(op=="<")
			{
				return value<=cons;
			}
			else if(op==">=")
			{
				return value>=cons;
			}
		}
	}
	return false;
}
//bool  ArrayOutOfBoundsCheckerV4::checkConditionStmtUseZ3(Expr* stmt,ArraySubscript input,AtomicAPInt con,bool flag){
//	//if(implyFlag(stmt,con.getLHS(),con.op,con.getRHS(),flag)) return true;
//	//else return false;
//	return false;
//}
//vector<ArraySubscript> ArrayOutOfBoundsCheckerV4::checkConditionStmtUseZ3(Expr* stmt,vector<ArraySubscript> input,bool flag){
//	vector<ArraySubscript> result;
//	//if flag is false,it's if's false branch,stmt use !stmt
//	////////cerr<<"checkConditionStmtUseZ3"<<endl;
//	////////cerr<<"stmt:"<<endl;stmt->dump();
//	////////cerr<<"input size:"<<input.size()<<endl;
//	for(unsigned i=0;i<input.size();i++){
//		bool notEmpty=false;
//		////////cerr<<"index is:"<<endl;input[i].index->dump();
//		//for(vector<AtomicAPInt>::iterator itr=input[i].condition.begin();itr!=input[i].condition.end();itr++){		
//		for(unsigned j=0;j<input[i].condition.size();j++){
//		////////cerr<<"AtomicAPInt:"<<getInfo(*itr)<<endl;
//			if(implyFlag(stmt,input[i].condition[j],flag)){
//				input[i].condition.erase(input[i].condition.begin()+j);
//			}else{
//				//////cerr<<"notEmpty=true"<<endl;
//				notEmpty=true;
//			}
//		}
//		if(notEmpty){
//			result.push_back(input[i]);
//			//////cerr<<"checkConditionStmtUseZ3 result size:"<<result.size()<<endl;
//		}
//	}
//
//	return result;
//}

bool ArrayOutOfBoundsCheckerV4::checkConditionStmtHasIdx(CFGBlock* block,Stmt* stmt,Expr* cond,ArraySubscript input,AtomicAPInt con,bool flag,bool checkTaint){
	//if checkTaint is true, we check whether index<untainted, namely, if index<tainted, return false
	//if checkTaint is false, we check whether index<**.
	bool result=false;
	Expr *idx=con.getLHS();
	common::printLog( "In checkConditionStmtHasIdx:\n",common::CheckerName::arrayBound,0,*common::configure);
	common::printLog( "condition is:"+printStmt(cond)+"\n",common::CheckerName::arrayBound,0,*common::configure);
	common::printLog( "input.stmt:"+printStmt(input.stmt)+"\n",common::CheckerName::arrayBound,0,*common::configure);
	if(CallExpr* call = dyn_cast<CallExpr>(cond->IgnoreImpCasts()->IgnoreParenCasts()) )
	{
		FunctionDecl* fun=call->getDirectCallee();
		if(fun==nullptr) return false;
		DeclarationName DeclName = fun->getNameInfo().getName();
		std::string FuncName = DeclName.getAsString();
		//cerr<<FuncName<<endl;
		if(FuncName=="__builtin_expect")
		{
			return checkConditionStmtHasIdx(block,stmt,call->getArg(0)->IgnoreImpCasts()->IgnoreParenCasts(),input,con,flag,checkTaint);

		}
	}
	if(BinaryOperator * it=dyn_cast<BinaryOperator>(cond)){	
		switch(it->getOpcode())
		{
		case clang::BinaryOperatorKind::BO_LAnd:
			{
				bool temp_lhs=checkConditionStmtHasIdx(block,stmt,it->getLHS()->IgnoreParenCasts()->IgnoreImpCasts(),input,con,flag,checkTaint);
				bool temp_rhs=checkConditionStmtHasIdx(block,stmt,it->getRHS()->IgnoreParenCasts()->IgnoreImpCasts(),input,con,flag,checkTaint);
				if(flag)
				{
					result=temp_lhs || temp_rhs;
				}
				else
				{
					result=temp_lhs && temp_rhs;
				}

				break;
			}
		case clang::BinaryOperatorKind::BO_LOr:
			{
				bool temp_lhs=checkConditionStmtHasIdx(block,stmt,it->getLHS()->IgnoreParenCasts()->IgnoreImpCasts(),input,con,flag,checkTaint);
				bool temp_rhs=checkConditionStmtHasIdx(block,stmt,it->getRHS()->IgnoreParenCasts()->IgnoreImpCasts(),input,con,flag,checkTaint);
				if(flag)
				{
					result=temp_lhs && temp_rhs;
				}
				else
				{
					result=temp_lhs || temp_rhs;
				}
				break;
			}
		default:
			{
				Expr* lhs=it->getLHS()->IgnoreImpCasts()->IgnoreImpCasts()->IgnoreParenCasts()->IgnoreImpCasts();
				Expr* rhs=it->getRHS()->IgnoreImpCasts()->IgnoreImpCasts()->IgnoreParenCasts()->IgnoreImpCasts();
				
				if(flag){
					if(con.op==clang::BinaryOperatorKind::BO_LT){						
						if(printStmt(idx)==printStmt(lhs))
						{
							//common::printLog( "Index matches lhs\n",common::CheckerName::arrayBound,0,*common::configure);
							common::printLog( "Index matches lhs\nrhs:FuncNow-"+FuncNow->getNameAsString()+"\tStmt-"+printStmt(stmt)+"\trhs-"+printStmt(rhs)+"\n",common::CheckerName::arrayBound,0,*common::configure);
							if(it->getOpcode()==clang::BinaryOperatorKind::BO_LT||
								it->getOpcode()==clang::BinaryOperatorKind::BO_LE)
							{
								if(checkTaint){
									if(!isTainted(FuncNow,block,stmt,rhs)){
										result=true;
									}
									else{
										result=false;
									}
								}
								else{ 
									result=true;
								}
							}							
						}
						else if(printStmt(idx)==printStmt(rhs))
						{
							if(it->getOpcode()==clang::BinaryOperatorKind::BO_GT||
								it->getOpcode()==clang::BinaryOperatorKind::BO_GE)
							{
								result=true;								
							}
						}
					}
					else if(con.op==clang::BinaryOperatorKind::BO_GE){
						if(printStmt(idx)==printStmt(lhs))
						{
							common::printLog( "Index match\n",common::CheckerName::arrayBound,0,*common::configure);
							if(it->getOpcode()==clang::BinaryOperatorKind::BO_GT||
								it->getOpcode()==clang::BinaryOperatorKind::BO_GE)
							{
								if(checkTaint){
									if(!isTainted(FuncNow,block,stmt,rhs)){
										result=true;
									}
									else{
										result=false;
									}
								}
								else{ 
									result=true;
								}								
							}							
						}
						else if(printStmt(idx)==printStmt(rhs))
						{
							if(it->getOpcode()==clang::BinaryOperatorKind::BO_LT||
								it->getOpcode()==clang::BinaryOperatorKind::BO_LE)
							{
								if(checkTaint){
									if(!isTainted(FuncNow,block,stmt,lhs)){
										result=true;
									}
									else{
										result=false;
									}
								}
								else{ 
									result=true;
								}								
							}
						}
					}
				}
				else
				{
					common::printLog( int2string(flag)+"path\n",common::CheckerName::arrayBound,0,*common::configure);
					if(con.op==clang::BinaryOperatorKind::BO_LT){						
						if(printStmt(idx)==printStmt(lhs))
						{
							common::printLog( "Index match\n",common::CheckerName::arrayBound,0,*common::configure);
							if(it->getOpcode()==clang::BinaryOperatorKind::BO_GT||
								it->getOpcode()==clang::BinaryOperatorKind::BO_GE)
							{
								if(checkTaint){
									if(!isTainted(FuncNow,block,stmt,rhs)){
										result=true;
									}
									else{
										result=false;
									}
								}
								else{ 
									result=true;
								}								
							}							
						}
						else if(printStmt(idx)==printStmt(rhs))
						{
							if(it->getOpcode()==clang::BinaryOperatorKind::BO_LT||
								it->getOpcode()==clang::BinaryOperatorKind::BO_LE)
							{
								if(checkTaint){
									if(!isTainted(FuncNow,block,stmt,lhs)){
										result=true;
									}
									else{
										result=false;
									}
								}
								else{ 
									result=true;
								}								
							}
						}
					}
					else if(con.op==clang::BinaryOperatorKind::BO_GE){
						if(printStmt(idx)==printStmt(lhs))
						{
							common::printLog( "Index match\n",common::CheckerName::arrayBound,0,*common::configure);
							if(it->getOpcode()==clang::BinaryOperatorKind::BO_LT||
								it->getOpcode()==clang::BinaryOperatorKind::BO_LE)
							{
								if(checkTaint){
									if(!isTainted(FuncNow,block,stmt,rhs)){
										result=true;
									}
									else{
										result=false;
									}
								}
								else{ 
									result=true;
								}								
							}							
						}
						else if(printStmt(idx)==printStmt(rhs))
						{
							if(it->getOpcode()==clang::BinaryOperatorKind::BO_GT||
								it->getOpcode()==clang::BinaryOperatorKind::BO_GE)
							{
								if(checkTaint){
									if(!isTainted(FuncNow,block,stmt,lhs)){
										result=true;
									}
									else{
										result=false;
									}
								}
								else{ 
									result=true;
								}								
							}
						}
					}
				}
			}
			}
			}
	//else if(UnaryOperator* it=dyn_cast<UnaryOperator>(idx)){
	//	bool temp_lhs=checkConditionStmtHasIdx(stmt,input,it->getSubExpr()->IgnoreParenCasts()->IgnoreImpCasts(),flag);
	//	result=temp_lhs;
	//}
	////if(isTainted(input.func,input.block,input.stmt,idx)){
	//else if(printStmt(stmt).find(printStmt(idx))!=string::npos){ result=true;}
	else{result=false;}
	return result;
}
