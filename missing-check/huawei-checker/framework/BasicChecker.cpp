
#include <iostream>

#include "framework/Common.h"
#include "framework/BasicChecker.h"

void BasicChecker::check(ASTFunction *F, FunctionDecl *FD, CFG *function_cfg) {
    
    clock_t start = clock();
    
    run_time += clock() - start;

}

clock_t BasicChecker::get_time() {
    
    return run_time;
}

