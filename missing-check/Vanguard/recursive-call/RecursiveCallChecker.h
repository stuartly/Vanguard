#ifndef _RECURSIVE_CALL_CHECKER_H
#define _RECURSIVE_CALL_CHECKER_H

#include "framework/Common.h" 

#include <string>
#include <unordered_map>

#include "clang/AST/Decl.h"

using namespace clang;


class RecursiveCallChecker {
public:
    RecursiveCallChecker() {
        runtime = 0;
    }
    void storeFunctionLocation(ASTFunction *F, FunctionDecl *FD);
    void check();
    clock_t getRuntime();
private:
    std::unordered_map<ASTFunction*, std::string> functionLocation;
    clock_t runtime;
};

#endif
