
#ifndef BASIC_CHECKER_H
#define BASIC_CHECKER_H

#include "clang/Analysis/CFG.h"
#include "clang/Frontend/ASTUnit.h"


#include "framework/ASTElement.h"

using namespace clang;

class BasicChecker {
    
public:

    void check(ASTFunction *F, FunctionDecl *FD, CFG *function_cfg);
    
    clock_t get_time();

private:
    clock_t run_time;

};


#endif

