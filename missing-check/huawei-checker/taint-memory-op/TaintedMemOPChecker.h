#include <sstream>
#include <iostream>
#include <string>
#include <string.h>
#include <list>
#include <queue>
#include  <vector>

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
#include "../library/xml/pugixml/pugixml.hpp"

using namespace clang;
using namespace llvm;
using namespace clang::driver;
using namespace clang::tooling;
using namespace std;



struct Node
{
		std::string NodeValue;
		struct Node * LHS;
		struct Node * RHS; 
};

class Tree
{
  public:
	  Node* root;
	  Tree()
	  {
		  root=NULL;
	  }
};


class TaintedMemOPChecker 
{
	public:	  
		void check(ASTFunction *F, FunctionDecl *FD, CFG *function_cfg);
		
		void visitCallExpr(CallExpr* callExpr,ASTFunction* caller);
		
		enum Color{WHITE,BLACK,GRAY};
		pugi::xml_document doc;
		int WarningNum; 
		clock_t get_time();
		TaintedMemOPChecker();
		~TaintedMemOPChecker();

	private:
		clock_t run_time;
		bool StringMode;
		bool ASTMode;
		bool Z3Mode;
		bool MemopSimpleValue;
		std::string PreOrderString;
		std::string Tainted_Data;		
		Expr *Tainted_Data_Expr;	
		std::string Tainted_Data_Location;
		

		FunctionDecl* RecordfunDel;
		CFGBlock* Recordblock ;
		Stmt* Recordstmt;
		CallExpr* RecordcallExpr;

		std::string Tainted_FileName;
		std::string Tainted_FunctionName;
		std::string Tainted_MemOPName;
		std::string Tainted_Locline;
		std::string Tainted_Description;
		

		
		std::string Caller2;
		std::string Caller3;
		std::string Tainted_MemOP_location;


		int location_of_Arg;	
		std::set<Expr*> If_ConditionExpr_Set;
		std::unordered_map<std::string,Expr*> IfCheckInfoList;

		int MemOPCheckLevel;		
		std::unordered_map<std::string, int> MemoryOPFunction;
		std::unordered_map<std::string, std::string> MemOPCheckStlye;
		std::unordered_map<std::string, std::string> RightIfCheckMode;
		std::unordered_map<std::string, std::string> SimpleExprCheckOnly;

		
		std::vector<ASTFunction *> getNonTopoOrder();		
		//void _DFSTopSort(ASTFunction *i,std::unordered_map<ASTFunction *, Color>& colors, std::vector<ASTFunction*> &NonTopoOrder);		
		void visitStmt(ASTFunction *F, FunctionDecl *FD, CFG *function_cfg);

		std::string getString_of_Expr( Expr* expr);
        bool is_MemOP(std::string calleeName);
        bool has_Tainted_Arg( CallExpr* callExpr,int location_of_Arg);
        bool is_Tainted(Expr* arg );
        bool has_ifCheck( ASTFunction*caller);
		bool is_right_ifCheck(std::string Tainted_MemOPName, Expr *Tainted_Data_Expr, std::set<Expr*> If_ConditionExpr_Set);

		std::string getMemOPCheckStlye(std::string Tainted_MemOPName);		

		bool is_Relevent_String(std::string if_condition_string,std::string Tainted_Data_Expr_string);
		bool is_Relevent_Expr(Expr* if_condition, Expr* Tainted_Data_Expr);	
		bool is_Equal_String(std::string if_condition, std::string rightCheckStlye);
				
	
		void N_Level_Check( ASTFunction *caller,int N);
		void writingToXML(std::string fileName, std::string funName, std::string descr, std::string locLine);
		void readConfig();

		
		void TwoCheck(ASTFunction* caller);

		std::string getOpCodeString(int opCode);
		Node* createTree(Expr* expr);
		void getTreePreOrderString(Node* T);
		int  Height(Node* T);
		void NormallizeOrder(Node* T);
		bool isSubTree(Node *bigTreeRoot,Node *smallTreeRoot);
		bool isEqualTree(Node* root1, Node* root2);	

		
   
		

};


 
