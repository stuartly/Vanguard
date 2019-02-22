
#ifndef DIV_CHECKER_H
#define DIV_CHECKER_H

#include <iostream>
#include <string>

#include "framework/BasicChecker.h"
#include "framework/Common.h"
#include "framework/ASTManager.h"
#include "pugixml/pugixml.hpp"
#include "taint-check/TaintChecker.h"
#include "taint-check/TaintInterface.h"


using namespace clang;
using namespace taint;
using namespace std;

struct Location{
	string FuncName;
    int basicBlockID;
    int stmtID;
    Location(string f, int b, int s){
        FuncName = f;
		basicBlockID = b;
        stmtID = s;
    }
    Location(){
		FuncName = "";
        basicBlockID = -1;
        stmtID = -1;
    }
    bool operator ==(const Location& a)const{
		return a.FuncName == FuncName && a.basicBlockID == basicBlockID && a.stmtID == stmtID;
    }
    bool operator <(const Location& a) const {
		if(FuncName == a.FuncName){
			if(basicBlockID < a.basicBlockID)
				return true;
			else if(basicBlockID == a.basicBlockID)
				return stmtID < a.stmtID;
			else
				return false;
		}else{
			return FuncName < a.FuncName;
		}
	}
    friend ostream& operator<<(ostream &out, const Location& l);
};

struct Edge{
	string FuncName;
    int fromID;
    int toID;
    Edge(string f, int from, int to){
        FuncName = f;
		fromID = from;
        toID = to;
    }
    bool operator ==(const Edge& a)const{
        return a.FuncName == FuncName && a.fromID == fromID && a.toID == toID;
    }
    bool operator <(const Edge& a) const {
		if(FuncName == a.FuncName){
			if(fromID < a.fromID)
				return true;
			else if(fromID == a.fromID)
				return toID < a.toID;
			else
				return false;
		}else{
			return FuncName < a.FuncName;
		}
	}
	friend ostream& operator<<(ostream &out, const Edge &e);
};

struct Property{
	bool changed;
    map<Location, bool> inPro;
    map<Location, bool> outPro;
    map<Edge, bool> edgePro;
	friend ostream& operator<<(ostream &out, const Property& p);
	void setInPro(Location& l, bool b){
		if(inPro[l] != b){
			//cerr << "in\n";
			inPro[l] = b;
			changed = true;
		}
	}
	void setOutPro(Location& l, bool b){
		if(outPro[l] != b){
			//cerr << "out\n";
			outPro[l] = b;
			changed = true;
		}
	}
	void setEdgePro(Edge& e, bool b){
		if(edgePro[e] != b){
			//cerr << "Edge\n";
			edgePro[e] = b;
			changed = true;
		}
	}
    void printProperty();
};

class DivChecker{

public:
	
    DivChecker() {
        run_time = 0;
    }

	void check(ASTFunction *F, FunctionDecl *FD, CFG *function_cfg);

    clock_t get_time() {
        return run_time;
    }
	
private:
	int funcLevel;
	bool conditionalCheck;
	bool compoundCheck;
	map<ExprPosition*, Location> taintDivMap;
	map<ExprPosition*, Property> taintDivProMap;
	map<ExprPosition*, map<string, bool>> divCallStack;

	map<ExprPosition*, int> callArgMap;
	map<ExprPosition*, Location> callExprMap;
	map<ExprPosition*, Property> callExprProMap;
	
    clock_t run_time;

	map<int, set<ExprPosition*>> initArgMap(ASTFunction *f, vector<ExprPosition*> divs);
	map<int, bool> checkArgProMap(ASTFunction* f, map<int, set<ExprPosition*>> argMap, int level, string callStack, ExprPosition* div);
	
	//bool checkCallFunc(ASTFunction* f, set<int> ID, int level);
	map<int, bool> checkFuncCFG(ASTFunction* caller, ASTFunction* callee,  map<int, set<ExprPosition*>> ID, map<int, bool> argpro);

	void initCallExpr(ASTFunction* caller, ASTFunction* callee, map<int, set<ExprPosition*>> ID, map<int, bool> caller_result);
	void checkCallExprCFG(ASTFunction* f);
	void checkCallExprBlock(CFGBlock* cb, set<ExprPosition*> vSet, ASTFunction* f);
	
	void initTaintDiv(ASTFunction* f, FunctionDecl* fd);
	void checkCFG(ASTFunction* f);
	void checkCFGBlock(CFGBlock* cb, set<ExprPosition*> vSet, ASTFunction* f);
	int checkExpression(ExprPosition* ep, const Expr* e, ASTFunction* f, bool checkFunc);
	
	const Expr* delParen(const Expr* e);
	bool isTaintedDivisor(ExprPosition* ep,const Expr* e, ASTFunction* f, bool checkFunc);
	bool isConditionalDivisor(Expr* e);
	bool checkConditionalExpr(Expr* e, ASTFunction* f);
	bool checkComparisionExpr(Expr* cond, Expr* e, ASTFunction* f, bool flag);
	map<bool, int> calculateExpr(Expr* e);
	//Print result to Terminal
	void printResult();
	//Print result to XML file
	pugi::xml_document doc;
	int writingToXML(string fileName, string funName, string descr, string locLine);
	void writeResult();

};
class isCompoundExpr : public RecursiveASTVisitor<isCompoundExpr>{

private:
	bool result;
public:
	isCompoundExpr(){result = false;}
	bool VisitBinaryOperator(BinaryOperator* bo_expr){
		if (bo_expr) { 
            result = true;
        }
        return true;
	}
    bool VisitCallExpr(CallExpr *CE) {
        if (CE) {
            result = true;
        }
        return true;
    }
	bool getResult(){
		return result;
	}
};

class FindDivExprVisitor : public RecursiveASTVisitor<FindDivExprVisitor>{

private:
	std::vector<Expr*> divExprs;
	bool checkEnable;
public:
	FindDivExprVisitor(bool flag){checkEnable = flag;}
	bool VisitBinaryOperator(BinaryOperator* bo_expr){
		if (bo_expr) { 
            BinaryOperatorKind op = bo_expr->getOpcode();
			if(op == BinaryOperatorKind::BO_Div || op == BinaryOperatorKind::BO_Rem || op == BinaryOperatorKind::BO_DivAssign || op == BinaryOperatorKind::BO_RemAssign){ 
                isCompoundExpr ice;
				ice.TraverseStmt(bo_expr->getRHS());
				if(!checkEnable){
					if(!ice.getResult()) 
						divExprs.push_back(bo_expr->getRHS());
				}else
					divExprs.push_back(bo_expr->getRHS());
            }
        }
        return true;
	}
	std::vector<Expr*>& getDivExprs(){
		
		return divExprs;
	}
};


class FindCallExprVisitor : public RecursiveASTVisitor<FindCallExprVisitor> {
    
public:
    bool VisitCallExpr(CallExpr *E) {
		if(E){
			if (FunctionDecl *FD = E->getDirectCallee()) {
				string fName = common::getFullName(FD);
				if(callExprs.find(fName)==callExprs.end()){
            		std::vector<CallExpr*> tmp;
            		tmp.push_back(E);
            		callExprs.insert(make_pair(fName, tmp));
				}
				else
            		callExprs[fName].push_back(E);
			}
		}
        return true;
    }

    std::vector<CallExpr*> getCallExpr(FunctionDecl* FD)
    {
		vector<CallExpr*> result;
		string fName = common::getFullName(FD);
		if(callExprs.find(fName) == callExprs.end()){
			return result;
		}
		else
    		return callExprs[fName];
    }

private:
    std::map<string,std::vector<CallExpr* > > callExprs;
};
#endif
