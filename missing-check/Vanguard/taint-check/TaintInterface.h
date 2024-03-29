
#ifndef TAINT_INTERFACE_H
#define TAINT_INTERFACE_H


#include "clang/AST/AST.h"
#include "clang/Analysis/CFG.h"
#include "clang/Frontend/ASTUnit.h"

#include "taint-check/TaintValue.h"
#include "framework/Common.h"

namespace taint {


TaintValueV getExprTaintValue(FunctionDecl *FD, CFGBlock *B, Stmt *S, Expr *E);

	struct ExprPosition{
		FunctionDecl* function;
		CFGBlock* block;
		Stmt* stmt;
		Expr* expr;
		ExprPosition(FunctionDecl* f=nullptr, CFGBlock* b=nullptr,Stmt* s=nullptr, Expr* e=nullptr){
			function = f;
			block = b;
			stmt = s;
			expr = e;
		}

		ExprPosition(const FunctionDecl* f, const CFGBlock* b,const Stmt* s,const  Expr* e){
			function = const_cast<FunctionDecl*>(f);
			block = const_cast<CFGBlock*>(b);
			stmt = const_cast<Stmt*>(s);
			expr = const_cast<Expr*>(e);
		}

		void simpleDump(){
			llvm::errs() << function->getNameAsString()<<", blockID: "
				<<block->getBlockID()<<", ";
			stmt->dump();
			expr->dump();
			
		}

		void dump(){
			llvm::errs() << function->getNameAsString()<<"\n";
			block->dump();
			std::string buffer;
			llvm::raw_string_ostream exprOS(buffer); 
			expr->printPretty(exprOS,nullptr,PrintingPolicy(function->getASTContext().getLangOpts()));
			llvm::errs() << exprOS.str() << "\n";
		}
 

		void simpleDump()const{
			llvm::errs() << function->getNameAsString()<<", blockID: "
				<<block->getBlockID()<<", ";
			stmt->dump();
			expr->dump();
			
		}

		void dump() const{
			llvm::errs() << function->getNameAsString()<<"\n";
			block->dump();
			stmt->dump();
			std::string buffer;
			llvm::raw_string_ostream exprOS(buffer); 
			expr->printPretty(exprOS,nullptr,PrintingPolicy(function->getASTContext().getLangOpts()));
			llvm::errs() << exprOS.str() << "\n";
		}

		template<class OStream>
		friend OStream& operator << (OStream &OS, const ExprPosition &ep){
			if(ep.function)
				OS << "function: " << ep.function->getNameAsString() << "\n";

			if(ep.block)
				OS << "block: " << ep.block->getBlockID() << "\n";

			if(ep.block)
				OS << "stmt: " << common::getStringStmt(ep.stmt,ep.function->getASTContext().getLangOpts()) << "\n";
			
			if(ep.block)
				OS << "expr: " << common::getStringStmt(ep.expr,ep.function->getASTContext().getLangOpts()) << "\n";

			return OS;
		}

		template<class OStream>
		friend OStream& operator << (OStream &OS, ExprPosition &ep){
			if(ep.function)
				OS << "function: " << ep.function->getNameAsString() << "\n";

			if(ep.block)
				OS << "block: " << ep.block->getBlockID() << "\n";

			if(ep.block)
				OS << "stmt: " << common::getStringStmt(ep.stmt,ep.function->getASTContext().getLangOpts()) << "\n";
			
			if(ep.block)
				OS << "expr: " << common::getStringStmt(ep.expr,ep.function->getASTContext().getLangOpts()) << "\n";

			return OS;
		}
	};

}

#endif
