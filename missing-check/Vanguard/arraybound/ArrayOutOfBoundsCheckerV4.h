#ifndef _ArrayOutOfBoundsCheckerV4_H
#define _ArrayOutOfBoundsCheckerV4_H
#include "../framework/BasicChecker.h"
#include <vector>
#include "clang/AST/Expr.h"
#include "clang/Analysis/CFG.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include <sstream>
#include <string>
#include <iostream>
#include <map>
#include <time.h>
#include <utility>
#include <unordered_set>
#include <set>
#include <stack>
#include "../taint-check/TaintChecker.h"
#include "../framework/Common.h"
#include "../library/xml/pugixml/pugixml.hpp"

using namespace std;

class AtomicAPInt{
	public:
	clang::BinaryOperatorKind op;
	Expr* lhs;
	uint64_t rhs;
	Expr* getLHS(){return lhs;}
	uint64_t getRHS(){return rhs;}
};
//record array's bound checking information, e.g. arr[b]=0, need bound checking: b<size
class ArraySubscript{
	public:
		CFGBlock *block;
		int ID;
		//int loopID;
		Stmt *stmt;
		Expr *orignIndex;
		Expr *index;
		FunctionDecl *func;
		std::string location;
		std::string originalLocation;
		bool isLoopBoundChecking;
		CFGBlock *loopblock;
		vector<AtomicAPInt> condition;
		void print();
		unsigned indexCnt;
		string arrayName;
		Expr *arrayIdx;
		stack<Expr*> indexs;
		void changeIndex(Expr * ni,bool flag=false)
		{
			if (MemberExpr *MRE = dyn_cast<MemberExpr>(index))
			{
				if(!flag)
				{
					indexs.push(index);
					index=ni;
				}
				else
				{
					indexs.push(MRE->getBase());
					MRE->setBase(ni);
					index=MRE;
				}
			}
			else
			{
				indexs.push(index);
				index=ni;
			}
			for(unsigned i=0;i<condition.size();i++)
				condition[i].lhs=index;
		}
		void resetIndex(bool flag=false)
		{
			if(indexs.empty())return;
			Expr* ni=indexs.top();
			indexs.pop();
			if (MemberExpr *MRE = dyn_cast<MemberExpr>(index))
			{
				if(!flag)
				{
					index=ni;
				}
				else
				{
					MRE->setBase(ni);
					index=MRE;
				}
			}
			else
			{
				index=ni;
			}
			for(unsigned i=0;i<condition.size();i++)
				condition[i].lhs=index;
		}
		void resetIndex(Expr * ni)
		{
			index=ni;
			for(unsigned i=0;i<condition.size();i++)
				condition[i].lhs=index;
		}
		ArraySubscript copy()
		{
			ArraySubscript tmp;
			tmp.block=block;
			tmp.ID=ID;
			tmp.stmt=stmt;
			tmp.orignIndex=orignIndex;
			tmp.index=index;
			tmp.func=func;
			tmp.location=location;
			tmp.originalLocation=originalLocation;
			isLoopBoundChecking=isLoopBoundChecking;
			tmp.loopblock=loopblock;
			tmp.condition=condition;
			
			tmp.indexCnt=indexCnt;
			tmp.arrayName=arrayName;
			tmp.arrayIdx=arrayIdx;
			tmp.indexs=indexs;
			tmp.arrayExpr=arrayExpr;
			return tmp;
		}
		string arrayExpr;//record the expression where uses array.
};

struct PNode{
	string name;
	double terms;
	PNode(string n,double t):name(n),terms(t){}
};

class FindArraySubscriptExpr : public RecursiveASTVisitor<FindArraySubscriptExpr> {
    
private:
    std::vector<ArraySubscriptExpr *> arrayExprs;

public:
    bool VisitArraySubscriptExpr(ArraySubscriptExpr *expr) {
        if (expr) {
            arrayExprs.push_back(expr);
        }
        return true;
    }	
    std::vector<ArraySubscriptExpr*>& getArrayExprs() {
        return arrayExprs;
    }
};

class ArrayOutOfBoundsCheckerV4 : public BasicChecker {
private:

	std::vector<CFGBlock*> unfinished;

	map<CFGBlock*, vector<ArraySubscript> > mapToBlockIn;
	map<CFGBlock*, vector<ArraySubscript> > mapToBlockOut;

	map<CFGBlock*, vector<pair<CFGBlock*,vector<Expr*> > > > mapToBlockOutLoopExpr;
	map<CFGBlock*, vector<Expr*> > mapToBlockInLoopExpr;
	map<CFGBlock*, vector<Expr*> > mapToBlockTaintedLoopExpr;

	map<Expr*,vector<Expr*>> mapToLoopTaintedExpr;
	map<Expr*,CFGBlock*> mapToCheckBlock;
	map<Expr*,Stmt*> mapToCheckStmt;
	vector<string> callpath;
	vector<int> reportedID;
	map<int,set<vector<string> > > mapToPath;
	int depth;
	string reportWarnings;

	int warningCount;
	//clock_t totalTime;
	//int cntCheck;
	FunctionDecl* FuncNow;
	bool SimpleCheckEnable;
	bool ifAnalyeLoopExpr;
	bool IndexIgnoreConditionalOperator;
	bool SimpleExprCheckOnly;
	bool DebugMode;
	std::string DebugFunctionName;
	vector<string> loopReport;
	clock_t run_time;
	map<int,vector<ArraySubscript>> sameID;
	//sameID: record the same ID in a function, means the ID is the same array subscript,
	//when encountering a for(i<m&&i<n) stmt, we split the it to two records with the same ID
public:
	pugi::xml_document doc_arraybound;
	pugi::xml_document doc_loopbound;
	bool doc_arraybound_empty;
	bool doc_loopbound_empty;
	int loc_time;
	int back_time;
//	void printTime(ofstream& process_file)
//	{
////		process_file<<"Loc Time:"<<loc_time/CLOCKS_PER_SEC/60<<"min"<<loc_time/CLOCKS_PER_SEC%60<<endl;
////		process_file<<"Check Time:"<<back_time/CLOCKS_PER_SEC/60<<"min"<<back_time/CLOCKS_PER_SEC%60<<endl;
//	}
	ArrayOutOfBoundsCheckerV4(){
		SimpleCheckEnable=false;
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
		DebugMode = (*common::configure)["ArrayBound"]["DebugMode"].asBool();
    		DebugFunctionName = (*common::configure)["ArrayBound"]["FunctionName"].asString();
		warningCount = 0;
        run_time = 0;
	}

    ~ArrayOutOfBoundsCheckerV4() {
        std::string xmlFile_loopbound = (*common::configure)["pathToReport"]["path"].asString() + "MOLINT.TAINTED.LOOP.BOUND.xml";
        std::string xmlFile_arraybound = (*common::configure)["pathToReport"]["path"].asString()  + "MOLINT.TAINTED.ARRAY.INDEX.xml";
        if(!doc_loopbound_empty) doc_loopbound.save_file(xmlFile_loopbound.c_str(), "\t", pugi::format_no_declaration);
        if(!doc_arraybound_empty) doc_arraybound.save_file(xmlFile_arraybound.c_str(), "\t", pugi::format_no_declaration);        

    }

	clock_t get_time();
	bool isTainted(FunctionDecl* Func,CFGBlock* block,Stmt* stmt,Expr* expr);
	bool checkExprTaintedForLoop(FunctionDecl* Func,CFGBlock* block,Stmt* stmt,Expr* expr);
	void check(ASTFunction *F, FunctionDecl *FD, CFG *function_cfg);
	void checkFunc(ASTFunction *astFunc);
	void initialize();
	vector<ArraySubscript> LocatingTaintExpr(FunctionDecl *func,CFG * myCFG);
	void AnalyeLoopExpr(FunctionDecl *func,CFG * cfg);
	vector<Expr*> checkTaintExpr(Expr* stmt,CFGBlock* block,FunctionDecl* func,bool flag);
	void DFS2func(ASTFunction *astFunc,int level,vector<ArraySubscript> list,unordered_set<int>& result);
	vector<ArraySubscript> backForwordAnalyse(CFG * cfg,vector<ArraySubscript> list,unordered_set<int>& set);
	
	std::unordered_map<Expr*, vector<Expr*>> getIdx(Expr *ex);
	vector<ArraySubscript> getFatherScript(vector<ArraySubscript> list,ASTFunction * father,ASTFunction * current,unordered_set<int>& errors,unordered_set<int>& erase);
	//std::unordered_map<Expr*, vector<Expr*>> getArrayInformation(Expr *ex);
	const vector<uint64_t> getArraySize(Expr* ex);
	VarDecl* getVarDecl(Expr* ex);
	bool isUnsigned(Expr *ex);
	void reportWarning(ArraySubscript as);
	void reportWarningSameID(vector<ArraySubscript> as);
	int writingToXML(bool isLoopBoundChecking, string fileName, string funName, string descr, string locLine,string checkerId, string indexCnt, string arrayExpr);
	vector<ArraySubscript> getArraySubscriptInExpr(Expr * ex,FunctionDecl *f,CFGBlock* block,Stmt* stmt,CFG * myCFG);
	vector<ArraySubscript> getArraySubscript(FunctionDecl *func,CFG * myCFG);

	vector<ArraySubscript> backForwordAnalyse(CFG * cfg,vector<ArraySubscript> list);
	vector<ArraySubscript> getScript(CFGBlock* block,Stmt* stmt,vector<ArraySubscript> input,bool T);
	vector<ArraySubscript> flowThrogth(CFGBlock* block,vector<ArraySubscript> list);
	bool CheckConditionExpr(Expr* expr,string op,uint64_t cons);
	bool checkConditionStmt(Expr* stmt,ArraySubscript input,AtomicAPInt con,bool flag);//true : remove this cons
	int throughStmt(Stmt* bo,ArraySubscript& con,CFGBlock* block);

	//vector<ArraySubscript> checkConditionStmtUseZ3(Expr* stmt,vector<ArraySubscript> input,bool flag);
	//bool checkConditionStmtHasIdx(Expr* stmt,ArraySubscript input,AtomicAPInt con,bool flag);
	bool checkConditionStmtHasIdx(CFGBlock* block,Stmt* stmt,Expr* cond,ArraySubscript input,AtomicAPInt con,bool flag,bool checkTaint);
	//bool checkConditionStmtUseZ3(Expr* stmt,ArraySubscript input,AtomicAPInt con,bool flag);
	std::string replace_all(string str, const string old_value, const string new_value);
	vector<ArraySubscript> ReplaceIdxLoopBound(CFGBlock* block,Stmt* cond,Expr* expr,ArraySubscript & input);
};
template <typename T> std::string int2string(const T& i);
string getInfo(Expr* expr);
string getInfo(uint64_t expr);
string getInfo(AtomicAPInt expr);
#endif
