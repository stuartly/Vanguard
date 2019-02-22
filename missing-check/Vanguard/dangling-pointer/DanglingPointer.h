#include <sstream>
#include <iostream>
#include <string>
#include <list>
#include <queue>

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

#include "../framework/Common.h"
#include "../library/xml/pugixml/pugixml.hpp"

using namespace clang;
using namespace llvm;
using namespace clang::driver;
using namespace clang::tooling;
using namespace std;

enum Color{WHITE,BLACK,GRAY};

class FunInfo{
public:
    string funName;
    int argsNum;
    string var;
    int TTL;
    FunInfo* parent;
    string loc;
    string file;
    int line;
    int refCount;
    FunInfo(string name, int arg, int ttl, string loc, int line = 0);
    FunInfo(string name, string varName, int ttl, string loc, int line = 0);
};


class VarInfo {
public:
    string varName;
    bool ifFree;
    bool ifNulled;
    bool ifStatic;
    VarInfo* alias;
	//this is used to delete alia relationships.
	list<VarInfo*> preAlias;
    FunInfo* freeLike;
    string loc;
    string file;
	string fun;
    int line;
	bool ifReported;
    VarInfo(string varName, bool ifFree, bool ifStatic, string loc, string fun);
};

class DanglingPtr{
public:
	
    void check(ASTFunction *F, FunctionDecl *FD, CFG *function_cfg);
	DanglingPtr();
	~DanglingPtr();
    
    clock_t get_time() {
        return run_time;
    }

private:

    clock_t run_time;

    list<VarInfo> vis;
    list<FunInfo> fis;
    //Config
    int levelOfDete;
    bool ifRegardParAsFreelike; //f(a){free(a);};fb{f(a);a=null;} false means ignore this situation.

    int warningCount;
	//ifCalled:  [callee, line]
    std::unordered_map<std::string, int> ifCalled;
	pugi::xml_document doc;
	std::unordered_map<std::string, int> memoryReleaseFuns;
	std::unordered_map<FunInfo*, bool> reportedFun;

    int readConfig();
    int clearLocalVar();
    int ifMemoryFun(string funName);
    bool VisitFunDecl(FunctionDecl *f, int ttl, CFG *myCFG);
    CFG::BuildOptions cfgBuildOptions;
    int ifStaticVar(string var, list<VarInfo>::iterator *iterator, string loc);
    int varStateTransform(string lfh, string rfh, FunctionDecl *f, string loc, string fun, Expr* rhs);
    int checkVar(string funName);
	int finalCheck();
    int reportWarning(VarInfo &tmp, VarInfo* alias = nullptr);
    int getOperLine(string loc);
	int writingToXML(string file, string fun, string descr, string line);
	std::string replace_all(string str, const string old_value, const string new_value); 
	int handleMemoryReleaseFun(FunctionDecl *f, string varName, string loc);
	int addStaticVar(const Expr* expr);
    void printList();
	void _DFSTopSort(ASTFunction *i, std::unordered_map<ASTFunction*, Color>& colors, std::vector<ASTFunction*> &NonTopoOrder);
    std::vector<ASTFunction *> getNonTopoOrder();
};
