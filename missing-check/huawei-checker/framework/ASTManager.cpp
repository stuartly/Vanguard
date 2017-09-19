
#include "framework/Common.h"
#include "framework/ASTManager.h"
#include "framework/BasicChecker.h"

#include "taint-check/TaintChecker.h"

ASTManager::ASTManager(std::vector<std::string> &ASTFiles) {
    
    current_size = 0;
    max_size = 10;
    
    std::unordered_set<std::string> unique_functions;
    std::cout << "Initialize AST : " << std::endl;
    unsigned n = 0;
    for (std::string AST : ASTFiles) {
        std::vector<FunctionDecl *> functions;
        std::vector<ASTFunction *> ast_functions;
        priorityQueue.push_back(AST, unique_functions, filter_functions, functions, ast_functions);
        
        for (unsigned i = 0; i < functions.size(); i++) {
            
            CallGraph::init(ast_functions[i], functions[i]);

            CFG * function_cfg = priorityQueue.maps[AST]->getCFG(ast_functions[i], functions[i]);
            taint::TaintChecker::init(ast_functions[i], functions[i], function_cfg);
        }
        n++;
        process_bar(float(n) / ASTFiles.size());
    }
    std::cout << std::endl;
}

void ASTManager::clear() {
    current_size = 0;
}

void ASTManager::push(std::string AST) {
    
    std::unique_ptr<ASTUnit> AU = common::loadFromASTFile(AST);
    ASTUnit *au_point = AU.get();
    if (current_size >= max_size) {
        if (tempASTUnit != nullptr) {
            maps.erase(tempASTUnit->getASTFileName());
        }
        tempASTUnit = std::move(AU);
    }
    else if (queue.size() >= max_size) {
        auto &it = queue.front();
        maps.erase(it->getASTFileName());
        queue.pop();
        queue.push(std::move(AU));
        current_size++;
    }
    else {
        queue.push(std::move(AU));
        current_size++;
    }
    maps[AST] = llvm::make_unique<ASTMap>();
    maps[AST]->buildMap(au_point, filter_functions, true);
}

void ASTManager::load(std::string AST, std::vector<ASTFunction*> &ast_functions, std::vector<FunctionDecl*> &functions) {

    priorityQueue.load(AST, filter_functions, ast_functions, functions);
}

FunctionDecl *ASTManager::load(ASTFunction *F) {
    
    if (F == nullptr) return nullptr;
    
    return priorityQueue.load(F, filter_functions);
}

ASTVariable *ASTManager::getASTVariable(VarDecl *VD, ASTFunction *F) {
    
    if (priorityQueue.maps.count(F->getAST()) != 0) {
        return priorityQueue.maps[F->getAST()]->getVariable(VD);
    }

    if (maps.count(F->getAST()) != 0) {
        return maps[F->getAST()]->getVariable(VD);
    }
    return nullptr;
}

FunctionDecl *ASTManager::getFunctionDecl(ASTFunction *F) {
    
    if (priorityQueue.maps.count(F->getAST()) != 0) {
        return priorityQueue.maps[F->getAST()]->getFunction(F);
    }
    
    if (maps.count(F->getAST()) != 0) {
        return maps[F->getAST()]->getFunction(F);
    }

    push(F->getAST());
    return maps[F->getAST()]->getFunction(F);
}

CFG *ASTManager::getCFG(ASTFunction *F, FunctionDecl *FD) {
    
    if (priorityQueue.maps.count(F->getAST()) != 0) {
        return priorityQueue.maps[F->getAST()]->getCFG(F, FD);
    }
    else if (maps.count(F->getAST()) != 0){
        return maps[F->getAST()]->getCFG(F, FD);
    }
    assert(0);
    return nullptr;
}

