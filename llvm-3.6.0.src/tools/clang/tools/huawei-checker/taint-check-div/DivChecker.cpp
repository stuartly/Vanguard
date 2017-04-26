
#include "DivChecker.h"

using namespace std;
using namespace taint;


void DivChecker::check(ASTFunction *F, FunctionDecl *FD, CFG *function_cfg){
    
    clock_t start = clock();

	funcLevel = (*common::configure)["DivChecker"]["level"].asInt();
	conditionalCheck = (*common::configure)["DivChecker"]["ConditionalCheckEnable"].asBool();
	compoundCheck = (*common::configure)["DivChecker"]["CompoundCheckEnable"].asBool();
    std::string test_function = (*common::configure)["DivChecker"]["FunctionName"].asString();
    if ( test_function != "" && test_function != FD->getNameAsString()) {
        return;
    }

	initTaintDiv(F, FD);
	
	checkCFG(F);
	//Caller checker
	vector<ExprPosition* > temp;
	for(auto var : taintDivMap){
		ExprPosition* v = var.first;
		Location l = var.second;
		if(taintDivProMap[v].inPro[l])
			temp.push_back(v);
	}
	if(funcLevel != 0 && !temp.empty()){
		for(auto t : temp){
			vector<ExprPosition*> tt;
			tt.push_back(t);
			map<int, set<ExprPosition *>> argMap = initArgMap(F, tt);
			map<int, bool> argProMap = checkArgProMap(F, argMap, funcLevel, "", t);

			int bID = function_cfg->getEntry().getBlockID();
			Edge e(F->getName(), bID, -1);
			for(auto argPro: argProMap){
			//cerr << func->getName() << ": " << argPro.first << "\t" <<argPro.second <<endl;
				if(!argPro.second){
					for(auto ep : argMap[argPro.first]){
						Location l = taintDivMap[ep];
						taintDivProMap[ep].inPro[l] = false;
					}
				}
			}
		}
			
	}
    

       // cerr << "writeResult.......\n";
	writeResult();
	taintDivMap.clear();
	taintDivProMap.clear();
	divCallStack.clear();
		//}
		//cerr << "---->"  << func->getName() << endl;
	
    run_time += clock() - start;
}

map<int, set<ExprPosition*>> DivChecker::initArgMap(ASTFunction *f, vector<ExprPosition*> divs){
	map<int, set<ExprPosition*>> result;
	FunctionDecl* fDecl = common::manager->getFunctionDecl(f);
	for(auto var : divs){
		ExprPosition* v = var;
		string name = common::getStringStmt(v->expr,v->function->getASTContext().getLangOpts());
		for(unsigned i = 0; i < fDecl->getNumParams(); i ++){
			auto p = fDecl->getParamDecl(i);
			string argName = p->getNameAsString();
			if(argName == name){
				//cerr << i << "\t" << *v;
				if(result.find(i) == result.end()){
					set<ExprPosition *> tmp;
					tmp.insert(v);
					result.insert(make_pair(i, tmp));
				}else{
					result[i].insert(v);
				}
			}
		}
	}
	return result;
}

map<int, bool> DivChecker::checkArgProMap(ASTFunction* callee, map<int, set<ExprPosition*>> argMap, int level, string callStack, ExprPosition* div){
	
    map<int, bool> result;
	
    vector<ASTFunction*> parents = common::call_graph->getParents(callee);
	if(parents.size() == 0 || level == 0){
		map<string, bool> tmpCallList;
		tmpCallList.insert(make_pair(callStack, true));
		if(divCallStack.find(div) == divCallStack.end())
			divCallStack.insert(make_pair(div, tmpCallList));
		else{
			divCallStack[div].insert(make_pair(callStack, true));
			
		}
		return result;
	}
	else{
		for(ASTFunction* caller : parents){
			
            map<int, bool> caller_result;
			auto callee_result = checkFuncCFG(caller, callee, argMap, caller_result);
			
            //DFS
			vector<ExprPosition*> tmp_arg;
			for(auto var : callExprMap){
				ExprPosition* v = var.first;
				Location l = var.second;
				if(callExprProMap[v].inPro[l]){
					tmp_arg.push_back(v);
					//cerr << *v;
				}
			}
			auto t_argMap = initArgMap(caller, tmp_arg);
			callArgMap.clear();
			callExprMap.clear();
			callExprProMap.clear();

			if(!t_argMap.empty()){ //No arguments need to check
				//cerr << "2222222222222\n";
				caller_result = checkArgProMap(caller, t_argMap, level-1, callStack+" "+caller->getName(), div);

				if(!caller_result.empty()){
					
                    callee_result = checkFuncCFG(caller, callee, argMap, caller_result);
					callArgMap.clear();
					callExprMap.clear();
					callExprProMap.clear();
				}
			}else{
				bool tmpCallResult = false;
				for(auto argPro : callee_result)
					tmpCallResult |= argPro.second;
				//cerr << tmp_arg.size() << endl;
				map<string, bool> tmpCallList;
				tmpCallList.insert(make_pair(callStack+" "+caller->getName(), tmpCallResult));
				if(divCallStack.find(div) == divCallStack.end())
					divCallStack.insert(make_pair(div, tmpCallList));
				else{
					divCallStack[div].insert(make_pair(callStack+" "+caller->getName(), tmpCallResult));
			
				}
				//cerr << callStack + caller->getName() << "\tFALSE" << endl;
			}
			for(auto t : callee_result){
				if(result.find(t.first) == result.end()){
					result.insert(make_pair(t.first, t.second));
				}else{
					result[t.first] |= t.second;
				}
				//cerr << t.first << "\t" << result[t.first] << endl;
			}
		}
		
		return result;
	}
}

map<int, bool> DivChecker::checkFuncCFG(ASTFunction* caller, ASTFunction* callee,  map<int, set<ExprPosition*>> ID, map<int, bool> caller_result){
	initCallExpr(caller, callee, ID, caller_result);
	
	checkCallExprCFG(caller);
	map<int, bool> result;
	for(auto var : callExprMap){
		ExprPosition* v = var.first;
		Location l = var.second;
		bool r = callExprProMap[v].inPro[l];
		int i = callArgMap[v];
		if(result.find(i) == result.end()){
			result.insert(make_pair(i, r));	
		}else{
			result[i] = result[i] || r;
		}
		//cerr << *v << i <<"\t"<< r << endl; 
	}
	
	return result;
}

void DivChecker::initCallExpr(ASTFunction* caller, ASTFunction* callee,  map<int, set<ExprPosition*>> ID, map<int, bool> caller_result){
	FunctionDecl* fd_caller = common::manager->getFunctionDecl(caller);
	FunctionDecl* fd = common::manager->getFunctionDecl(callee);
	
	auto cfg = common::manager->getCFG(caller, fd_caller);
	string fName = caller->getName();
	map<Expr*, ExprPosition*> taintCall;
	Property p;
	if(cfg== nullptr || fd_caller == nullptr)return;
	for(auto &block : *cfg){
		//block->dump();
		int bID = block->getBlockID();
		int sID = 1;
		for(auto &element : *block){
			if(element.getKind() == CFGStmt::Statement){
				
				Location l(fName, bID, sID);
				p.inPro.insert(make_pair(l, true));
				p.outPro.insert(make_pair(l, true));
				
				Stmt *stmt = const_cast<Stmt*>(element.castAs<CFGStmt>().getStmt());
				FindCallExprVisitor call_visitor;
				call_visitor.TraverseStmt(stmt);
				//cerr << "1"<<common::getFullName(fd) << endl;
				const auto &callExprs = call_visitor.getCallExpr(fd);
				
				if(!callExprs.empty()){
					for(auto ce : callExprs){
						for(auto index : ID){
							Expr* e = ce->getArg(index.first);
							if(e != NULL){
							ExprPosition* ep = new ExprPosition(fd_caller, block, stmt, e);
							if(taintCall.find(ce) == taintCall.end()){
								taintCall.insert(make_pair(ce,ep));
								callExprMap.insert(make_pair(ep, l));
							}else{
								auto ep2 = taintCall.find(ce)->second;
								string stmt1 = common::getStringStmt(ep->stmt,ep->function->getASTContext().getLangOpts());
								string stmt2 = common::getStringStmt(ep2->stmt,ep2->function->getASTContext().getLangOpts());
								if(stmt1.length() < stmt2.length()){
									taintCall[ce] = ep;
									callExprMap.erase(ep2);
									callExprMap.insert(make_pair(ep, l));
								}
							}
							callArgMap.insert(make_pair(ep, index.first));
							}
						}
					}
									
				}
				sID ++;
			}
		}
		for(CFGBlock::succ_iterator succ=block->succ_begin(); succ != block->succ_end(); ++ succ){
			
			CFGBlock* succ_cb = (*succ);
			if(!succ_cb)
				succ_cb = succ->getPossiblyUnreachableBlock();
			if(!succ_cb)	continue;
            int toID = succ_cb->getBlockID();
            Edge e(fName, bID, toID);
			
            p.edgePro.insert(make_pair(e, false));
        }
		if(block->pred_empty()){
			Edge e(fName, bID, -1);
			p.edgePro.insert(make_pair(e, true));
		}
		
	}
	if(!taintCall.empty()){
		
		for(auto ce : taintCall){
			callExprProMap.insert(make_pair(ce.second, p));
		}
	}
	if(!caller_result.empty()){
		for(auto cr : caller_result){
			int i = cr.first;
			int r = cr.second;
			auto p = common::manager->getFunctionDecl(caller)->getParamDecl(i);
			string argName = p->getNameAsString();
			for(auto ce : callExprProMap){
				auto ep = ce.first;
				string name = common::getStringStmt(ep->expr,ep->function->getASTContext().getLangOpts());
				if(name == argName){
					Property *pro = &(callExprProMap[ep]);
					CFGBlock entry = cfg->getEntry();
					CFGBlock* succ = *(entry.succ_begin());
					int fID = entry.getBlockID();
					int tID = -1;
					Edge e(fName, fID, tID);
					pro->edgePro[e] = r;
				}
			}
		}
	}
}

void DivChecker::checkCallExprCFG(ASTFunction* f){
	set<ExprPosition *> vSet;
	for(auto ce : callExprMap)
		vSet.insert(ce.first);

	int count_var = vSet.size();
	auto cfg = common::manager->getCFG(f, common::manager->getFunctionDecl(f));
	while(count_var > 0){
		for(auto cb : *cfg){
			checkCallExprBlock(cb, vSet, f);
		}

		for(auto var : vSet){
			if(callExprProMap[var].changed)
				callExprProMap[var].changed = false;
			else{
				count_var --;
			}
		}
	}
	return;
}

void DivChecker::checkCallExprBlock(CFGBlock* cb, set<ExprPosition*> vSet, ASTFunction* f){
	string FuncName = f->getName();
	for(auto var : vSet){
		//cb->dump();
		Property *pro = &(callExprProMap[var]);
		//Process the IN EDGE
		bool eqZero_begin = false;
		int result_end = -1;
		int bID = cb->getBlockID();
		if(cb->pred_empty()){
			Edge e(FuncName, bID, -1);
			eqZero_begin = pro->edgePro[e];
		}else{
			for(CFGBlock::pred_iterator pred=cb->pred_begin(); pred != cb->pred_end(); ++ pred){
				CFGBlock* pred_cb = (*pred);
				if(!pred_cb)
					pred_cb = pred->getPossiblyUnreachableBlock();
				if(!pred_cb)	continue;
				int pred_bID = pred_cb->getBlockID();
				Edge e(FuncName, pred_bID, bID);
				eqZero_begin |= pro->edgePro[e];
			}
		}
		//Process the Stmts
		int sID = 1;
        for(CFGBlock::iterator cb_it=cb->begin(); cb_it!=cb->end(); ++cb_it){
			CFGElement ce = *cb_it;
            if(ce.getKind() == CFGElement::Kind::Statement){
				Location l_cur(FuncName, bID, sID);
                Location l_pre(FuncName, bID, sID-1);
				//inProperty
                bool eqZero_IN = eqZero_begin;
                if(sID != 1)
					eqZero_IN = pro->outPro[l_pre];	
				pro->setInPro(l_cur, eqZero_IN);
                const Stmt* st = ((CFGStmt*)&ce)->getStmt();
                //consider the expr stmt
				if(st == NULL)	continue;
                if(const Expr* expr = dyn_cast<Expr>(st)){
                    int result = checkExpression(var, expr, f, true);
					
                    if(result == 0){
						pro->setOutPro(l_cur, false);
                    }
                    else if(result == 1){
						pro->setOutPro(l_cur, true);
                    }
                    else{
						pro->setOutPro(l_cur, eqZero_IN);    
                    }
					result_end = result;
                }else{
					pro->setOutPro(l_cur, pro->inPro[l_cur]);
                }
				/*if(bID > 6){
					st->dump();
					cerr << "Location:" << bID << sID << "--->" << pro->inPro[l_cur] << pro->outPro[l_cur] << endl;
				}*/
				//cerr << "Location:" << bID << sID << "--->" << pro->inPro[l_cur] << pro->outPro[l_cur] << endl;
				sID ++;
            }
		}

		//Process the Terminator
		CFGTerminator ct = cb->getTerminator();
		bool eqZero_end = eqZero_begin;
	    if(cb->size() != 0){
			Location l_end(FuncName, bID, cb->size());
		    eqZero_end = pro->outPro[l_end];
	    }
        if(!ct){     //No Branchz`
			if(cb->succ_size() != 0){
				CFGBlock::AdjacentBlock element = (*cb->succ_begin());
			    CFGBlock* succ_cb = element.getReachableBlock();
				if(succ_cb == nullptr){
					continue;
				}
		        int succ_ID = succ_cb->getBlockID();
				Edge e(FuncName, bID, succ_ID);
				pro->setEdgePro(e, eqZero_end);
				//cerr << "Edge:" << bID << succ_ID << "--->" << pro->edgePro[e] <<endl;
			}
		}else{	//Branch
			if(IfStmt* if_stmt = dyn_cast<IfStmt>(ct.getStmt())){   //If Stmt
				//cerr << ct->getStmtClassName() << endl;
				int thenBlockID, elseBlockID;
				bool tag = true;
				for(CFGBlock::succ_iterator succ=cb->succ_begin(); succ != cb->succ_end(); ++ succ){
					CFGBlock* succ_cb = (*succ);
					if(!succ_cb)
						succ_cb = succ->getPossiblyUnreachableBlock();
					if(!succ_cb)	continue;
					
					if(tag){
						thenBlockID = succ_cb->getBlockID();
						tag = false;
					}else
						elseBlockID = succ_cb->getBlockID();
				}
				Expr* cond_expr = if_stmt->getCond();
				int result = checkExpression(var, cond_expr, f, true);
				Edge e1(FuncName, bID, thenBlockID);
				Edge e2(FuncName, bID, elseBlockID);
				if(result_end == 0){
					pro->setEdgePro(e1, false);
					pro->setEdgePro(e2, eqZero_end);
				}else if(result_end == 1){
					pro->setEdgePro(e1, true);
					pro->setEdgePro(e2, false);
				}else{
					pro->setEdgePro(e1, eqZero_end);
					pro->setEdgePro(e2, eqZero_end);
				}
				/*cerr << "Edge:" << bID << thenBlockID << "--->" << pro->edgePro[e1] <<endl;
				cerr << "Edge:" << bID << elseBlockID << "--->" << pro->edgePro[e2] <<endl;*/
			}else if(ConditionalOperator* co_stmt = dyn_cast<ConditionalOperator>(ct.getStmt())){ //ConditionOperator Stmt
				Expr* cond_expr = co_stmt->getCond();
				//cond_expr->dump();
				int thenBlockID, elseBlockID;
				bool tag = true;
				for(CFGBlock::succ_iterator succ=cb->succ_begin(); succ != cb->succ_end(); ++ succ){
					CFGBlock* succ_cb = (*succ);
					if(!succ_cb)
						succ_cb = succ->getPossiblyUnreachableBlock();
					if(!succ_cb)	continue;
					
					if(tag){
						thenBlockID = succ_cb->getBlockID();
						tag = false;
					}else
						elseBlockID = succ_cb->getBlockID();
				}
				Edge e1(FuncName, bID, thenBlockID);
				Edge e2(FuncName, bID, elseBlockID);
				int result = checkExpression(var, cond_expr, f, true);
				if(result_end == 0){
					pro->setEdgePro(e1, false);
					pro->setEdgePro(e2, eqZero_end);
				}else if(result_end == 1){
					pro->setEdgePro(e1, true);
					pro->setEdgePro(e2, false);
				}else{
					pro->setEdgePro(e1, eqZero_end);
					pro->setEdgePro(e2, eqZero_end);
				}
				/*cerr << eqZero_begin << "  " << eqZero_end << endl;
				cerr << "Edge:" << bID << thenBlockID << "--->" << pro->edgePro[e1] <<endl;
				cerr << "Edge:" << bID << elseBlockID << "--->" << pro->edgePro[e2] <<endl;*/
			}else if(const BinaryOperator* bo_expr = dyn_cast<BinaryOperator>(ct)){
				
				int thenBlockID, elseBlockID;
				bool tag = true;
				for(CFGBlock::succ_iterator succ=cb->succ_begin(); succ != cb->succ_end(); ++ succ){
					CFGBlock* succ_cb = (*succ);
					if(!succ_cb)
						succ_cb = succ->getPossiblyUnreachableBlock();
					if(!succ_cb)	continue;
					
					if(tag){
						thenBlockID = succ_cb->getBlockID();
						tag = false;
					}else
						elseBlockID = succ_cb->getBlockID();
				}
				
				Edge e1(FuncName, bID, thenBlockID);
				Edge e2(FuncName, bID, elseBlockID);
				
				if(result_end == 0){
					pro->setEdgePro(e1, false);
					pro->setEdgePro(e2, eqZero_end);
				}else if(result_end == 1){
					pro->setEdgePro(e1, true);
					pro->setEdgePro(e2, false);
				}else{
					pro->setEdgePro(e1, eqZero_end);
					pro->setEdgePro(e2, eqZero_end);
				}
			}else if(ForStmt* fStmt = dyn_cast<ForStmt>(ct)){
				Expr* cond_expr = fStmt->getCond();
				//cond_expr->dump();
				int thenBlockID, elseBlockID;
				bool tag = true;
				for(CFGBlock::succ_iterator succ=cb->succ_begin(); succ != cb->succ_end(); ++ succ){
					CFGBlock* succ_cb = (*succ);
					if(!succ_cb)
						succ_cb = succ->getPossiblyUnreachableBlock();
					if(!succ_cb)	continue;
					
					if(tag){
						thenBlockID = succ_cb->getBlockID();
						tag = false;
					}else
						elseBlockID = succ_cb->getBlockID();
				}
				int result = checkExpression(var, cond_expr, f, true);
				Edge e1(FuncName, bID, thenBlockID);
				Edge e2(FuncName, bID, elseBlockID);
				if(result_end == 0){
					pro->setEdgePro(e1, false);
					pro->setEdgePro(e2, eqZero_end);
				}else if(result_end == 1){
					pro->setEdgePro(e1, true);
					pro->setEdgePro(e2, false);
				}else{
					pro->setEdgePro(e1, eqZero_end);
					pro->setEdgePro(e2, eqZero_end);
				}
			}else if(WhileStmt* wStmt = dyn_cast<WhileStmt>(ct)){
				Expr* cond_expr = wStmt->getCond();
				int thenBlockID, elseBlockID;
				bool tag = true;
				for(CFGBlock::succ_iterator succ=cb->succ_begin(); succ != cb->succ_end(); ++ succ){
					CFGBlock* succ_cb = (*succ);
					if(!succ_cb)
						succ_cb = succ->getPossiblyUnreachableBlock();
					if(!succ_cb)	continue;
					
					if(tag){
						thenBlockID = succ_cb->getBlockID();
						tag = false;
					}else
						elseBlockID = succ_cb->getBlockID();
				}
				int result = checkExpression(var, cond_expr, f, true);
				Edge e1(FuncName, bID, thenBlockID);
				Edge e2(FuncName, bID, elseBlockID);
				if(result_end == 0){
					pro->setEdgePro(e1, false);
					pro->setEdgePro(e2, eqZero_end);
				}else if(result_end == 1){
					pro->setEdgePro(e1, true);
					pro->setEdgePro(e2, false);
				}else{
					pro->setEdgePro(e1, eqZero_end);
					pro->setEdgePro(e2, eqZero_end);
				}
			}
			
        }
	}
}
void DivChecker::initTaintDiv(ASTFunction* f, FunctionDecl* fd){
	auto cfg = common::manager->getCFG(f, fd);
	string fName = f->getName();
	map<Expr*, ExprPosition*> taintDiv;
	Property p;
	if(cfg== nullptr || fd == nullptr)return;
	for(auto &block : *cfg){
		//common::dumpLog(*block, common::CheckerName::divideChecker, 3, *configure);
		int bID = block->getBlockID();
		int sID = 1;
		for(auto &element : *block){
			////cerr  << "1111111111111111111111\n";
			if(element.getKind() == CFGStmt::Statement){
				
				Location l(fName, bID, sID);
				p.inPro.insert(make_pair(l, true));
				p.outPro.insert(make_pair(l, true));
				
				Stmt *stmt = const_cast<Stmt*>(element.castAs<CFGStmt>().getStmt());
				//stmt->dump();
				
				FindDivExprVisitor div_visitor(compoundCheck);
				div_visitor.TraverseStmt(stmt);
				const auto &divExprs = div_visitor.getDivExprs();
				if(!divExprs.empty()){
					for(auto div : divExprs){
						ExprPosition* ep = new ExprPosition(fd, block, stmt, div);
						auto taintValue = TaintChecker::is_tainted(fd, block, stmt, div);
						//div->dump();
						//cerr << *ep;

						if(isConditionalDivisor(div)){
							if(conditionalCheck)
								continue;
							else{
								bool result = checkConditionalExpr(div, f);
								//cerr << result << endl;
								if(!result) continue;
							}
						}
						if(taintValue){
							//cerr << "----->Tainted\n";
							if(taintDiv.find(div) == taintDiv.end()){
								taintDiv.insert(make_pair(div,ep));
								taintDivMap.insert(make_pair(ep, l));
							}else{
								auto ep2 = taintDiv.find(div)->second;
								string stmt1 = common::getStringStmt(ep->stmt,ep->function->getASTContext().getLangOpts());
								string stmt2 = common::getStringStmt(ep2->stmt,ep2->function->getASTContext().getLangOpts());
								if(stmt1.length() < stmt2.length()){
									taintDiv[div] = ep;
									taintDivMap.erase(ep2);
									taintDivMap.insert(make_pair(ep, l));
								}
							}
						}else{
							delete ep;
						}
						//cerr<<"...\n";

		
					}
					//cerr << f->getName() << " " << divExprs.size()<<endl;					
				}
				sID ++;
			}
		}
		//cerr << "22222222222222222222\n";
		for(CFGBlock::succ_iterator succ=block->succ_begin(); succ != block->succ_end(); ++ succ){
			
			CFGBlock* succ_cb = (*succ);
			if(!succ_cb)
				succ_cb = succ->getPossiblyUnreachableBlock();
			if(!succ_cb)	continue;
            int toID = succ_cb->getBlockID();
            Edge e(fName, bID, toID);
			
            p.edgePro.insert(make_pair(e, false));
        }
		if(block->pred_empty()){
			Edge e(fName, bID, -1);
			p.edgePro.insert(make_pair(e, true));
		}
		//cerr << "333333333333333333333333\n";
	}
	//cerr << "4444444444444444444444444\n";
	if(!taintDiv.empty()){
		//p.printProperty();
		for(auto div : taintDiv){
			taintDivProMap.insert(make_pair(div.second, p));
		}
	}
	//cerr << "5555555555555555555\n";
}

void DivChecker::checkCFG(ASTFunction* f){
	set<ExprPosition *> vSet;
	for(auto div : taintDivMap){
		if(div.second.FuncName == f->getName()){
			vSet.insert(div.first);
		}
	}
	
	int count_var = vSet.size();
	auto cfg = common::manager->getCFG(f,common::manager->getFunctionDecl(f));
	
	while(count_var > 0){
		for(auto cb : *cfg){
			checkCFGBlock(cb, vSet, f);
		}

		for(auto var : vSet){
			if(taintDivProMap[var].changed)
				taintDivProMap[var].changed = false;
			else
				count_var --;
		}
		//cerr << count_var << endl;
	}
	return;
}

void DivChecker::checkCFGBlock(CFGBlock* cb, set<ExprPosition*> vSet, ASTFunction* f){
	//cb->dump();
	string FuncName = f->getName();
	for(auto var : vSet){
		//Property  *pro = &(divProperty[var]);
		Property *pro = &(taintDivProMap[var]);
		//Process the IN EDGE
		bool eqZero_begin = false;
		int result_end = -1;
		int bID = cb->getBlockID();
		if(cb->pred_empty()){
			Edge e(FuncName, bID, -1);
			eqZero_begin = pro->edgePro[e];
		}else{
			for(CFGBlock::pred_iterator pred=cb->pred_begin(); pred != cb->pred_end(); ++ pred){
				CFGBlock* pred_cb = (*pred);
				if(!pred_cb)
					pred_cb = pred->getPossiblyUnreachableBlock();
				if(!pred_cb)	continue;
				int pred_bID = pred_cb->getBlockID();
				Edge e(FuncName, pred_bID, bID);
				eqZero_begin |= pro->edgePro[e];
			}
		}
		//Process the Stmts
		int sID = 1;
        for(CFGBlock::iterator cb_it=cb->begin(); cb_it!=cb->end(); ++cb_it){
			CFGElement ce = *cb_it;
            if(ce.getKind() == CFGElement::Kind::Statement){
				Location l_cur(FuncName, bID, sID);
                Location l_pre(FuncName, bID, sID-1);
				//inProperty
                bool eqZero_IN = eqZero_begin;
                if(sID != 1)
					eqZero_IN = pro->outPro[l_pre];	
				pro->setInPro(l_cur, eqZero_IN);
                const Stmt* st = ((CFGStmt*)&ce)->getStmt();
				//st->dump();
                //consider the expr stmt
                if(const Expr* expr = dyn_cast<Expr>(st)){
					int result = checkExpression(var, expr, f, false);
				    // if(cb->getBlockID() == 2) {expr->dump();cout << result <<endl;}	
                    if(result == 0){
						pro->setOutPro(l_cur, false);
                    }
                    else if(result == 1){
						pro->setOutPro(l_cur, true);
                    }
                    else{
						pro->setOutPro(l_cur, eqZero_IN);    
                    }
					result_end = result;
                }else{
					pro->setOutPro(l_cur, pro->inPro[l_cur]);
                }
				if(bID == 1){
					//st->dump();
					//cerr << "Location:" << bID << sID << "--->" << pro->inPro[l_cur] << pro->outPro[l_cur] << endl;
				}
				//st->dump();
				//cerr << "Location:" << bID << sID << "--->" << pro->inPro[l_cur] << pro->outPro[l_cur] << endl;
				sID ++;
            }
		}

		//Process the Terminator
		CFGTerminator ct = cb->getTerminator();
		bool eqZero_end = eqZero_begin;
	    if(cb->size() != 0){
			Location l_end(FuncName, bID, cb->size());
			if(!ct)    eqZero_end = pro->outPro[l_end];
			else eqZero_end = pro->inPro[l_end];
	    }
        //cb->dump();
        if(!ct){     //No Branch
			if(cb->succ_size() != 0){
				CFGBlock::AdjacentBlock element = (*cb->succ_begin());
			    CFGBlock* succ_cb = element.getReachableBlock();
				if(succ_cb == nullptr){
					continue;
				}
		        int succ_ID = succ_cb->getBlockID();
				Edge e(FuncName, bID, succ_ID);
				pro->setEdgePro(e, eqZero_end);
				// cerr << "Edge:" << bID << succ_ID << "--->" << pro->edgePro[e] <<endl;
			}
		}else{	//Branch
			//ct->dump();
			if(IfStmt* if_stmt = dyn_cast<IfStmt>(ct.getStmt())){   //If Stmt
				//cerr << ct->getStmtClassName() << endl;
				int thenBlockID, elseBlockID;
				bool tag = true;
				for(CFGBlock::succ_iterator succ=cb->succ_begin(); succ != cb->succ_end(); ++ succ){
					CFGBlock* succ_cb = (*succ);
					if(!succ_cb)
						succ_cb = succ->getPossiblyUnreachableBlock();
					if(!succ_cb)	continue;
					
					if(tag){
						thenBlockID = succ_cb->getBlockID();
						tag = false;
					}else
						elseBlockID = succ_cb->getBlockID();
				}
				Expr* cond_expr = if_stmt->getCond();
				int result = checkExpression(var, cond_expr, f, false);
				Edge e1(FuncName, bID, thenBlockID);
				Edge e2(FuncName, bID, elseBlockID);
				if(result_end == 0){
					pro->setEdgePro(e1, false);
					pro->setEdgePro(e2, eqZero_end);
				}else if(result_end == 1){
					pro->setEdgePro(e1, true);
					pro->setEdgePro(e2, false);
				}else{
					pro->setEdgePro(e1, eqZero_end);
					pro->setEdgePro(e2, eqZero_end);
				}
				//cerr << "Edge:" << bID << thenBlockID << "--->" << pro->edgePro[e1] <<endl;
				//cerr << "Edge:" << bID << elseBlockID << "--->" << pro->edgePro[e2] <<endl;
			}else if(ConditionalOperator* co_stmt = dyn_cast<ConditionalOperator>(ct.getStmt())){ //ConditionOperator Stmt
				Expr* cond_expr = co_stmt->getCond();
				//co_stmt->dump();
				int thenBlockID, elseBlockID;
				bool tag = true;
				for(CFGBlock::succ_iterator succ=cb->succ_begin(); succ != cb->succ_end(); ++ succ){
					CFGBlock* succ_cb = (*succ);
					if(!succ_cb)
						succ_cb = succ->getPossiblyUnreachableBlock();
					if(!succ_cb)	continue;
					
					if(tag){
						thenBlockID = succ_cb->getBlockID();
						tag = false;
					}else
						elseBlockID = succ_cb->getBlockID();
				}
				int result = checkExpression(var, cond_expr, f, false);
				Edge e1(FuncName, bID, thenBlockID);
				Edge e2(FuncName, bID, elseBlockID);
				if(result_end == 0){
					pro->setEdgePro(e1, false);
					pro->setEdgePro(e2, eqZero_end);
				}else if(result_end == 1){
					pro->setEdgePro(e1, true);
					pro->setEdgePro(e2, false);
				}else{
					pro->setEdgePro(e1, eqZero_end);
					pro->setEdgePro(e2, eqZero_end);
				}
				/*
				cerr << eqZero_begin << "  " << eqZero_end << endl;
				cerr << "Edge:" << bID << thenBlockID << "--->" << pro->edgePro[e1] <<endl;
				cerr << "Edge:" << bID << elseBlockID << "--->" << pro->edgePro[e2] <<endl;*/
			}else if(const BinaryOperator* bo_expr = dyn_cast<BinaryOperator>(ct)){
				
				int thenBlockID, elseBlockID;
				bool tag = true;
				for(CFGBlock::succ_iterator succ=cb->succ_begin(); succ != cb->succ_end(); ++ succ){
					CFGBlock* succ_cb = (*succ);
					if(!succ_cb)
						succ_cb = succ->getPossiblyUnreachableBlock();
					if(!succ_cb)	continue;
					
					if(tag){
						thenBlockID = succ_cb->getBlockID();
						tag = false;
					}else
						elseBlockID = succ_cb->getBlockID();
				}
				
				Edge e1(FuncName, bID, thenBlockID);
				Edge e2(FuncName, bID, elseBlockID);
				
				if(result_end == 0){
					pro->setEdgePro(e1, false);
					pro->setEdgePro(e2, eqZero_end);
				}else if(result_end == 1){
					pro->setEdgePro(e1, true);
					pro->setEdgePro(e2, false);
				}else{
					pro->setEdgePro(e1, eqZero_end);
					pro->setEdgePro(e2, eqZero_end);
				}
                //cerr<< "add cout" << endl;
                //bo_expr->dump();
                //cerr<< result_end << " " << eqZero_end << endl;
				/*cerr << eqZero_begin << "  " << eqZero_end << endl;
				cerr << "Edge:" << bID << thenBlockID << "--->" << pro->edgePro[e1] <<endl;
				cerr << "Edge:" << bID << elseBlockID << "--->" << pro->edgePro[e2] <<endl;*/
			}else if(ForStmt* fStmt = dyn_cast<ForStmt>(ct)){
				Expr* cond_expr = fStmt->getCond();
				//cond_expr->dump();
				int thenBlockID, elseBlockID;
				bool tag = true;
				for(CFGBlock::succ_iterator succ=cb->succ_begin(); succ != cb->succ_end(); ++ succ){
					CFGBlock* succ_cb = (*succ);
					if(!succ_cb)
						succ_cb = succ->getPossiblyUnreachableBlock();
					if(!succ_cb)	continue;
					
					if(tag){
						thenBlockID = succ_cb->getBlockID();
						tag = false;
					}else
						elseBlockID = succ_cb->getBlockID();
				}
				int result = checkExpression(var, cond_expr, f, false);
				Edge e1(FuncName, bID, thenBlockID);
				Edge e2(FuncName, bID, elseBlockID);
				if(result_end == 0){
					pro->setEdgePro(e1, false);
					pro->setEdgePro(e2, eqZero_end);
				}else if(result_end == 1){
					pro->setEdgePro(e1, true);
					pro->setEdgePro(e2, false);
				}else{
					pro->setEdgePro(e1, eqZero_end);
					pro->setEdgePro(e2, eqZero_end);
				}
			}else if(WhileStmt* wStmt = dyn_cast<WhileStmt>(ct)){
				Expr* cond_expr = wStmt->getCond();
				int thenBlockID, elseBlockID;
				bool tag = true;
				for(CFGBlock::succ_iterator succ=cb->succ_begin(); succ != cb->succ_end(); ++ succ){
					CFGBlock* succ_cb = (*succ);
					if(!succ_cb)
						succ_cb = succ->getPossiblyUnreachableBlock();
					if(!succ_cb)	continue;
					
					if(tag){
						thenBlockID = succ_cb->getBlockID();
						tag = false;
					}else
						elseBlockID = succ_cb->getBlockID();
				}
				int result = checkExpression(var, cond_expr, f, false);
				Edge e1(FuncName, bID, thenBlockID);
				Edge e2(FuncName, bID, elseBlockID);
				if(result_end == 0){
					pro->setEdgePro(e1, false);
					pro->setEdgePro(e2, eqZero_end);
				}else if(result_end == 1){
					pro->setEdgePro(e1, true);
					pro->setEdgePro(e2, false);
				}else{
					pro->setEdgePro(e1, eqZero_end);
					pro->setEdgePro(e2, eqZero_end);
				}
			}	
        }
	}
	
}

int DivChecker::checkExpression(ExprPosition* ep, const Expr* e, ASTFunction* FuncName, bool checkFunc){
	//e->dump();
   // cerr <<"Check Expression start......." << std::endl;
	if(e == NULL)
		return 2;
	if(const BinaryOperator* bo_expr = dyn_cast<BinaryOperator>(e)){
	//	cerr << "Binary........" << std::endl;
		Expr* lhs = bo_expr->getLHS();
        Expr* rhs = bo_expr->getRHS();
        if (lhs == nullptr || rhs == nullptr) {
        //    std::cerr << "lhs rhs null" << std::endl;
            return 2;
        }
        BinaryOperatorKind op = bo_expr->getOpcode();
		if(op == BinaryOperatorKind::BO_Assign){
			//lhs->dump();
			//rhs->dump();
			if(isTaintedDivisor(ep, lhs, FuncName, checkFunc)){
				if(const IntegerLiteral* IL_expr = dyn_cast<IntegerLiteral>(rhs)){
					if(IL_expr->getValue() != 0)    return 0;
					else    return 1; 
				}else if(ImplicitCastExpr* ICE_expr = dyn_cast<ImplicitCastExpr>(rhs)){
					Expr* sub_expr = ICE_expr->getSubExpr();
					if (sub_expr == nullptr) {
                    //    std::cerr << "sub expr null" << std::endl;
                        return 2;
                    }
					if(IntegerLiteral* IL_expr = dyn_cast<IntegerLiteral>(sub_expr)){
						if(IL_expr->getValue() != 0)    return 0;
						else    return 1;
					}else if(DeclRefExpr* dr_expr = dyn_cast<DeclRefExpr>(sub_expr)){
						if (dr_expr->getDecl() == nullptr) {
                           // std::cerr << "get decl null" << std::endl;
                            return 2;
                        }
                        VarDecl* vd = dyn_cast<VarDecl>(dr_expr->getDecl());
						return 0;
						/*if(divProperty.find(vd) == divProperty.end()){
							divProperty.insert(make_pair(vd, Property()));
							vd->dump();
						}
						return 2;*/
					}else{
						auto rhs_value = calculateExpr(rhs);
						if(rhs_value.find(true) != rhs_value.end()){
							//rhs->dump();
							//cerr << rhs_value[true] << endl;
							if(rhs_value[true] != 0)	return 0;
							else return 1;
						}
						return 2;
					}
				}else if(isConditionalDivisor(rhs)){
					if(conditionalCheck)	return false;
					else{
						bool result = checkConditionalExpr(rhs, FuncName);
						//cerr << result << endl;
						return result;
					}
				}else{
					auto rhs_value = calculateExpr(rhs);
					if(rhs_value.find(true) != rhs_value.end()){
						//rhs->dump();
						//cerr << rhs_value[true] << endl;
						if(rhs_value[true] != 0)	return 0;
						else return 1;
					}
					return 2;
				}
			}//else if(isRelatedDivisor(lhs, FuncName)){}
			return 2;
		}else if(BinaryOperator::isComparisonOp(op)){
			//cerr << "Comparision......"<<std::endl;
			if(isTaintedDivisor(ep, lhs, FuncName, checkFunc)){
				//if(ImplicitCastExpr* ic_expr = dyn_cast<ImplicitCastExpr>(lhs)){
					//if(isTaintedDivisor(ep, lhs, FuncName, checkFunc)){
						if(const IntegerLiteral* IL_expr = dyn_cast<IntegerLiteral>(rhs)){
							if(op == BinaryOperatorKind::BO_LT){
								if(IL_expr->getValue().sle(0))    return 0;
								else    return 1;
							}else if(op == BinaryOperatorKind::BO_GT){
								if(IL_expr->getValue().sge(0))    return 0;
								else    return 1;
							}else if(op == BinaryOperatorKind::BO_LE){
								if(IL_expr->getValue().slt(0))    return 0;
								else    return 1;
							}else if(op == BinaryOperatorKind::BO_GE){
								if(IL_expr->getValue().sgt(0))    return 0;
								else    return 1;
							}else if(op == BinaryOperatorKind::BO_EQ){
								if(IL_expr->getValue() != 0)    return 0;
								else    return 1;
							}else if(op == BinaryOperatorKind::BO_NE){
								if(IL_expr->getValue() == 0)    return 0;
								else    return 1;
							}else
								return 0;
						}else{
							auto rhs_value = calculateExpr(rhs);
							//rhs->dump();
							if(rhs_value.find(true) != rhs_value.end()){
								//cerr << rhs_value[true] << endl;
								if(op == BinaryOperatorKind::BO_LT){
									if(rhs_value[true] <= 0)    return 0;
									else    return 1;
								}else if(op == BinaryOperatorKind::BO_GT){
									if(rhs_value[true] >= 0)    return 0;
									else    return 1;
								}else if(op == BinaryOperatorKind::BO_LE){
									if(rhs_value[true] < 0)    return 0;
									else    return 1;
								}else if(op == BinaryOperatorKind::BO_GE){
									if(rhs_value[true] > 0)    return 0;
									else    return 1;
								}else if(op == BinaryOperatorKind::BO_EQ){
									if(rhs_value[true] != 0)    return 0;
									else    return 1;
								}else if(op == BinaryOperatorKind::BO_NE){
									if(rhs_value[true] == 0)    return 0;
									else    return 1;
								}else
									return 0;
							}
						}
					//}
				//}//else if()
			}
			else if(isTaintedDivisor(ep, rhs, FuncName, checkFunc)){
				//rhs->dump();
				//if(ImplicitCastExpr* ic_expr = dyn_cast<ImplicitCastExpr>(rhs)){
					//if(isTaintedDivisor(ep, rhs, FuncName, checkFunc)){
						if(const IntegerLiteral* IL_expr = dyn_cast<IntegerLiteral>(lhs)){
							if(op == BinaryOperatorKind::BO_LT){
								if(IL_expr->getValue().slt(0))    return 1;
								else    return 0;
							}else if(op == BinaryOperatorKind::BO_GT){
								if(IL_expr->getValue().sgt(0))    return 1;
								else    return 0;
							}else if(op == BinaryOperatorKind::BO_LE){
								if(IL_expr->getValue().sle(0))    return 1;
								else    return 0;
							}else if(op == BinaryOperatorKind::BO_GE){
								if(IL_expr->getValue().sge(0))    return 1;
								else    return 0;
							}else if(op == BinaryOperatorKind::BO_EQ){
								if(IL_expr->getValue() != 0)    return 0;
								else    return 1;
							}else if(op == BinaryOperatorKind::BO_NE){
								if(IL_expr->getValue() == 0)    return 0;
								else    return 1;
							}else
								return 0;
						}else{
							auto lhs_value = calculateExpr(lhs);
							if(lhs_value.find(true) != lhs_value.end()){
								if(op == BinaryOperatorKind::BO_LT){
									if(lhs_value[true] < 0)    return 1;
									else    return 0;
								}else if(op == BinaryOperatorKind::BO_GT){
									if(lhs_value[true] > 0)    return 1;
									else    return 0;
								}else if(op == BinaryOperatorKind::BO_LE){
									if(lhs_value[true] <= 0)    return 1;
									else    return 0;
								}else if(op == BinaryOperatorKind::BO_GE){
									if(lhs_value[true] >= 0)    return 1;
									else    return 0;
								}else if(op == BinaryOperatorKind::BO_EQ){
									if(lhs_value[true] != 0)    return 0;
									else    return 1;
								}else if(op == BinaryOperatorKind::BO_NE){
									if(lhs_value[true] == 0)    return 0;
									else    return 1;
								}else
									return 0;
							}
						}
					//}
				//}
			}
            return 2;
		}else{
			//cerr << "oooooooooooooooooooooo\n";
			return 2;
		}
	}else if(const CallExpr* call_expr = dyn_cast<CallExpr>(e)){
        //std::cerr << "call expr" << std::endl;
        if (call_expr->getDirectCallee() == nullptr) {
            //std::cerr << "direct callee is null" << std::endl;
            return 2;
        }
        if (!(call_expr->getDirectCallee()->getDeclName().isIdentifier())) {
            return 2;
        }
		string fName = call_expr->getDirectCallee()->getName();
		//cerr << "FuncName: " << fName << endl;
		if(fName == "Assert" || fName == "assert"){
            if (call_expr->getNumArgs() == 0) {
                //std::cerr << "assert arg num 0" << std::endl;
                return 2;
            } 
			const Expr* arg = call_expr->getArg(0);
			int r =  checkExpression(ep, arg, FuncName, checkFunc);
			return r;
		}else if(fName == "__builtin_expect"){
			if(call_expr->getNumArgs() == 0)
				return 2;
			const Expr* arg = call_expr->getArg(0);
			//arg->dump();
			
			int r =  checkExpression(ep, arg, FuncName, checkFunc);
			//cerr << r << endl;
			if(r == 0)	return 1;
			else if (r == 1) return 0;
			else return 2;
		}
	}else if(const UnaryOperator* uo_expr = dyn_cast<UnaryOperator>(e)){
        //std::cerr << "unary operator" << std::endl;
		const Expr* sub_expr = uo_expr->getSubExpr();
		if(sub_expr != NULL){
		if(uo_expr->getOpcode() == UnaryOperatorKind::UO_LNot){
			//sub_expr->dump();
			int r = checkExpression(ep, sub_expr, FuncName, checkFunc);
			if(r == 0)	return 1;
			else if (r == 1) return 0;
			else return 2;
		}
		}
		//cerr << "----------------\n";
		//sub_expr->dump();
	}else if(const ParenExpr* pa_expr = dyn_cast<ParenExpr>(e)){
		const Expr* sub_expr = pa_expr->getSubExpr();
		if(sub_expr != NULL)
			return checkExpression(ep, sub_expr, FuncName, checkFunc);
		//sub_expr->dump();
	}else if(const ImplicitCastExpr* ic_expr = dyn_cast<ImplicitCastExpr>(e)){
		auto sub_expr = ic_expr->getSubExpr();
		if(sub_expr != NULL){
			return checkExpression(ep, sub_expr, FuncName, checkFunc);
		}else
			return 2;
	}else if(const DeclRefExpr* dr_expr = dyn_cast<DeclRefExpr>(e)){
		if(isTaintedDivisor(ep, dr_expr, FuncName, checkFunc))
			return 2;
		else
			return 2;
	}
	return 2;
}

bool DivChecker::isTaintedDivisor(ExprPosition* ep, const Expr* e, ASTFunction* f, bool checkFunc){
	if(ep->expr == NULL || e == NULL)	return false;
	if(checkFunc){
		for(auto ce : callExprMap){
			ExprPosition* ep = ce.first;
			const LangOptions& LO = ep->function->getASTContext().getLangOpts();
			string s1 = common::getStringStmt(ep->expr,LO);
			string s2 = common::getStringStmt(e, LO);
			if(CStyleCastExpr* cs_expr = dyn_cast<CStyleCastExpr>(ep->expr)){
				Expr* sub_expr = cs_expr->getSubExpr();
				if(sub_expr != NULL) s1 = common::getStringStmt(sub_expr, LO);
			}
			if(const CStyleCastExpr* cs_expr = dyn_cast<CStyleCastExpr>(e)){
				const Expr* sub_expr = cs_expr->getSubExpr();
				if(sub_expr != NULL) s2 = common::getStringStmt(sub_expr, LO);
			}
			if(s1 == s2)	return true;
		}
		return false;
	}else{
		//for(auto divisor : taintDivMap){
		if(f->getName() == ep->function->getName()){
				const LangOptions& LO = ep->function->getASTContext().getLangOpts();
				const Expr* e1 = delParen(ep->expr);
				const Expr* e2 = delParen(e);
				string s1 = common::getStringStmt(e1,LO);
				string s2 = common::getStringStmt(e2, LO);

				if(CStyleCastExpr* cs_expr = dyn_cast<CStyleCastExpr>(ep->expr)){
					Expr* sub_expr = cs_expr->getSubExpr();
					if(sub_expr != NULL) {
						const Expr* eDel = delParen(sub_expr);
						s1 = common::getStringStmt(eDel, LO);
					}
				}
				if(const CStyleCastExpr* cs_expr = dyn_cast<CStyleCastExpr>(e)){
					const Expr* sub_expr = cs_expr->getSubExpr();
					if(sub_expr != NULL){
						const Expr* eDel = delParen(sub_expr);
						s2 = common::getStringStmt(eDel, LO);
						
					}
				}
				//ep->expr->dump();
				//e->dump();
				//cerr << s1 << "\t" << s2 << endl;
				//Terms t1(s1);
				//Terms t2(s2);
				//if(t1.isEqual(t2))	return true;
				
				if(s1 == s2)	return true;
			}
		//}
		return false;
	}
}
const Expr* DivChecker::delParen(const Expr* e){
	if(e == NULL)	return e;
	
	if(const ParenExpr* pa_expr = dyn_cast<ParenExpr>(e)){
		const Expr* sub_expr = pa_expr->getSubExpr();
		if(sub_expr != NULL)
			return delParen(sub_expr);
		else
			return e;
	}
	if(const ImplicitCastExpr* ic_expr = dyn_cast<ImplicitCastExpr>(e)){
		const Expr* sub_expr = ic_expr->getSubExpr();
		if(sub_expr != NULL)	return delParen(sub_expr);
		else return e;
	}
	return e;
}

bool DivChecker::isConditionalDivisor(Expr* e){
	if(ParenExpr* pa_expr = dyn_cast<ParenExpr>(e)){
		Expr* sub_expr = pa_expr->getSubExpr();
		if(sub_expr != NULL)
			return isConditionalDivisor(sub_expr);
		else
			return false;
	}else if(dyn_cast<ConditionalOperator>(e))
		return true;
	else
		return false;
}
bool DivChecker::checkConditionalExpr(Expr* e, ASTFunction* f){
	if(ParenExpr* pa_expr = dyn_cast<ParenExpr>(e)){
		Expr* sub_expr = pa_expr->getSubExpr();
		if(sub_expr != NULL)
			return checkConditionalExpr(sub_expr, f);
		else
			return true;
	}else if(ConditionalOperator* co_expr = dyn_cast<ConditionalOperator>(e)){
		Expr* cond = co_expr->getCond();
		Expr* lhs = co_expr->getLHS();
		Expr* rhs = co_expr->getRHS();

		auto l_result = calculateExpr(lhs);
		auto r_result = calculateExpr(rhs);
		if(l_result.find(false) == l_result.end() && r_result.find(false) == r_result.end()){
			bool result = (l_result[true] == 0) || (r_result[true] == 0);
			return result;
		}else if(l_result.find(false) == l_result.end()){
			//cerr << l_result[true] << endl;
			//cond->dump();
			//rhs->dump();
			return (checkComparisionExpr(cond, rhs, f, true)) || (l_result[true]==0);
		}else{
			//rhs->dump();
			return (checkComparisionExpr(cond, lhs, f, false)) || (r_result[true]==0);
		}
	}else
		return true;

}

bool DivChecker::checkComparisionExpr(Expr* cond, Expr* e, ASTFunction* f, bool flag){
	if(cond == NULL)
		return false;
	if(BinaryOperator* bo_expr = dyn_cast<BinaryOperator>(cond)){
		Expr* lhs = bo_expr->getLHS();
		Expr* rhs = bo_expr->getRHS();
		BinaryOperatorKind op = bo_expr->getOpcode();
		if(flag){
			if(op == BinaryOperatorKind::BO_LT){
				op = BinaryOperatorKind::BO_GE;
			}else if(op == BinaryOperatorKind::BO_GT){
				op = BinaryOperatorKind::BO_LE;
			}else if(op == BinaryOperatorKind::BO_LE){
				op = BinaryOperatorKind::BO_GT;
			}else if(op == BinaryOperatorKind::BO_GE){
				op = BinaryOperatorKind::BO_LT;
			}else if(op == BinaryOperatorKind::BO_EQ){
				op = BinaryOperatorKind::BO_NE;
			}else if(op == BinaryOperatorKind::BO_NE){
				op = BinaryOperatorKind::BO_EQ;
			}
		}
		FunctionDecl* function = common::manager->getFunctionDecl(f);
		const LangOptions& LO = function->getASTContext().getLangOpts();
		string s = common::getStringStmt(e, LO);
		string s1 = common::getStringStmt(lhs, LO);
		string s2 = common::getStringStmt(rhs, LO);
		//cerr << s << s1 << s2 << endl;
		if(s1 != s && s2 != s)
			return true;
		else if(s1 != s){
			auto lhs_value = calculateExpr(lhs);	
			if(lhs_value.find(true) != lhs_value.end()){
				//cerr << lhs_value[true];
				if(op == BinaryOperatorKind::BO_LT){
					if(lhs_value[true] < 0)    return 1;
					else    return 0;
				}else if(op == BinaryOperatorKind::BO_GT){
					if(lhs_value[true] > 0)    return 1;
					else    return 0;
				}else if(op == BinaryOperatorKind::BO_LE){
					if(lhs_value[true] <= 0)    return 1;
					else    return 0;
				}else if(op == BinaryOperatorKind::BO_GE){
					if(lhs_value[true] >= 0)    return 1;
					else    return 0;
				}else if(op == BinaryOperatorKind::BO_EQ){
					if(lhs_value[true] != 0)    return 0;
					else    return 1;
				}else if(op == BinaryOperatorKind::BO_NE){
					if(lhs_value[true] == 0)    return 0;
					else    return 1;
				}else
					return 1;
			}else 
				return true;
		}else{
			auto rhs_value = calculateExpr(rhs);
			if(rhs_value.find(true) != rhs_value.end()){
				if(op == BinaryOperatorKind::BO_LT){
					if(rhs_value[true] <= 0)    return 0;
					else    return 1;
				}else if(op == BinaryOperatorKind::BO_GT){
					if(rhs_value[true] >= 0)    return 0;
					else    return 1;
				}else if(op == BinaryOperatorKind::BO_LE){
					if(rhs_value[true] < 0)    return 0;
					else    return 1;
				}else if(op == BinaryOperatorKind::BO_GE){
					if(rhs_value[true] > 0)    return 0;
					else    return 1;
				}else if(op == BinaryOperatorKind::BO_EQ){
					if(rhs_value[true] != 0)    return 0;
					else    return 1;
				}else if(op == BinaryOperatorKind::BO_NE){
					if(rhs_value[true] == 0)    return 0;
					else    return 1;
				}else
					return 1;
			}else
				return true;
		}
		
		auto r_result = calculateExpr(rhs);
	}else if(ParenExpr* pe_expr = dyn_cast<ParenExpr>(cond)){
		Expr* sub_expr = pe_expr->getSubExpr();
		if(sub_expr != NULL)	return checkComparisionExpr(sub_expr, e, f, flag);
		else return true;
	}else{
		return true;
	}
}

map<bool, int> DivChecker::calculateExpr(Expr* e){
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
        if(sub_expr == NULL){
            result.insert(make_pair(false, -1));
            return result;
        }
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


/********************PRINT FUNCTIONS****************************/
void DivChecker::printResult(){
	for(auto var : taintDivMap){
		ExprPosition* v = var.first;
		Location l = var.second;
	//	bool result = taintDivProMap[v].inPro[l];
		//cerr << *v ;
	
	}
}
int DivChecker::writingToXML(string fileName, string funName, string descr, string locLine)
{
    pugi::xml_node node = doc.append_child("error");

    pugi::xml_node checker = node.append_child("checker");
    checker.append_child(pugi::node_pcdata).set_value("MOLINT.TAINTED.DIV.MOD");

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

	return 0;
}
void DivChecker::writeResult(){
	bool tag = false;
	for(auto var : taintDivMap){	
		ExprPosition* v = var.first;

        stringstream expr_string;
        expr_string << *v;
        // common::printLog(expr_string.str(), common::divideChecker, 1, *common::configure);

		Location l = var.second;
		bool result = taintDivProMap[v].inPro[l];
		if(result){
			tag = true;
			string file;
			string stmt = common::getStringStmt(v->stmt,v->function->getASTContext().getLangOpts());
			string name = common::getStringStmt(v->expr,v->function->getASTContext().getLangOpts());
			string loc = v->expr->getLocStart().printToString(v->function->getASTContext().getSourceManager());
			int pos1 = loc.find_last_of('<');
			string str;
			if(pos1 == -1){
				pos1 = loc.find_last_of(':');
			}else{
    			loc.assign(loc.c_str(),pos1);
    			pos1 = loc.find_last_of(':');
			}
			str.assign(loc.c_str(),pos1);
			int pos2=str.find_last_of(':');
			file.assign(loc.c_str(),pos2);	
			string line = str.substr(pos2+1);
			string des_Call = "";
			if(divCallStack.find(v) != divCallStack.end()){
				auto cStack = divCallStack[v];
				for(auto fList : cStack){
					if(fList.second){	
						des_Call += fList.first;
						des_Call += "\n\t\t\t\t\t\t";
					}
					//cerr << fList.first << fList.second << endl;
				}
			}
			string desc = "\n\t\t\tDivisor: " + name+"\n\t\t\tResult: Could be 0, Please Check.";
			if(des_Call != ""){
				desc += "\n\t\t\tCallStack:\t";
				desc += des_Call;
			}
			desc +=  "\n\t\t";
			for(unsigned i = 0; i < file.length(); i ++)
				if(file[i] == '\\') file[i] = '/';
			writingToXML(file, var.second.FuncName, desc, line);
			}
		//cerr << name << "\t" << file << "\t" << line << endl;
	}
	
	std::string xmlFile = (*common::configure)["pathToReport"]["path"].asString() + "MOLINT.TAINTED.DIV.MOD.xml";
	if(tag)doc.save_file(xmlFile.c_str(),"\t", pugi::format_default|pugi::format_no_declaration);
}

void Property::printProperty(){
    cerr << "***************Print Property Start***************\n";
	
	cerr << "Function\tBID\tSID\tInPro\tOutPro\n";
    map<Location, bool>::iterator it_out, it_in;
	for(it_in = inPro.begin(), it_out = outPro.begin(); it_in != inPro.end(); it_in ++,it_out ++){
		cerr << it_in->first << "\t" << it_in->second << "\t" << it_out->second << endl;
    }
	
	cerr << "Function\tFID\tTID\tPro\n";
	for(auto ep : edgePro){
		cerr << ep.first << "\t" << ep.second << endl;
	}
    cerr << "***************Print Property End***************\n\n";
}

ostream& operator<<(ostream& out, const Location& l){
	out << l.FuncName << "\tB" << l.basicBlockID << "\tS" << l.stmtID ;
    return out;
}
ostream& operator<<(ostream& out, const Edge &e){
	out << e.FuncName << "\tB" << e.fromID << "\tB" << e.toID;
	return out;
}
ostream& operator<<(ostream &out, const Property& p){
	map<Location, bool>::iterator it_out, it_in;
	map<Location, bool> inPro = p.inPro;
	map<Location, bool> outPro = p.outPro;
	for(it_in = inPro.begin(), it_out = outPro.begin(); it_in != inPro.end(); it_in ++,it_out ++){
		out << it_in->first << "\t" << it_in->second << "\t" << it_out->second << endl;
    }
	return out;	
}
