
#ifndef TAINT_CHECKER_H
#define TAINT_CHECKER_H

#include "taint-check/FunctionEnv.h"

namespace taint {

class TaintChecker {

public:
    
    TaintChecker() {
        run_time = 0;
    };

    ~TaintChecker();

    static void init(ASTFunction *F, FunctionDecl *FD, CFG *function_cfg);

    void check(ASTFunction *F, FunctionDecl *FD, CFG *function_cfg);

    void build(std::vector<ASTFunction *> &topOrder);    
    
    static std::vector<FunctionEnvironment *> functions;
    
    static bool is_tainted(FunctionDecl *F, CFGBlock *B, Stmt *S, Expr *E);

    static std::set<std::string> blackList;
    static std::set<std::string> whiteList;

    clock_t get_time() {
        return run_time;
    }

private:
    clock_t run_time;
    
};


}


#endif
