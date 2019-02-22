
#include "framework/Common.h"

#include "framework/ASTQueue.h"

void ASTMap::buildMap(ASTUnit *AU, std::unordered_map<std::string, std::set<unsigned>> &filter_functions, bool build_variable) {
    
    const std::vector<ASTFunction *> &ast_functions = common::resource->getFunctions(AU->getASTFileName());
    const std::vector<FunctionDecl *> &functions = common::getFunctions(AU->getASTContext());
    
    if (filter_functions.count(AU->getASTFileName()) != 0) {
        assert(functions.size() == ast_functions.size() + filter_functions[AU->getASTFileName()].size());
    }
    else assert(functions.size() == ast_functions.size());

    unsigned i = 0, j = 0;
    while (i < functions.size()) {
        if (filter_functions.count(AU->getASTFileName()) != 0) {
            if (filter_functions[AU->getASTFileName()].count(i) != 0) {
                i++;
                continue;
            }
        }
        addFunction(ast_functions[j], functions[i]);
        
        if (build_variable) {
            const std::vector<ASTVariable *> &ast_variables = ast_functions[j]->getVariables();
            const std::vector<VarDecl *> variables = common::getVariables(functions[i]);
            unsigned m = 0, n = 0;
            if (dyn_cast<CXXMethodDecl>(functions[i])) m = 1;
            for (; m < ast_variables.size(); m++, n++) {
                addVariable(ast_variables[m], variables[n]);
            }
            assert(m == ast_variables.size() && n == variables.size());
        }
        i++; j++;
    }
}


ASTQueue::ASTQueue() {

    max_size = (*common::configure)["Framework"]["queue_size"].asUInt();
}

ASTUnit *ASTQueue::push_back(std::string AST, std::unordered_set<std::string> &unique_functions, 
        std::unordered_map<std::string, std::set<unsigned>> &filter_functions, 
        std::vector<FunctionDecl *> &return_functions, std::vector<ASTFunction *> &return_ast_functions) {
    
    clock_t start = clock();
    std::unique_ptr<ASTUnit> AU = common::loadFromASTFile(AST);
    ASTFile *AF = common::resource->addASTFile(AST);
    std::vector<FunctionDecl *> functions = common::getFunctions(AU->getASTContext());
    
    maps[AST] = llvm::make_unique<ASTMap>();

    unsigned i = 0;
    for (FunctionDecl *FD : functions) {
        
        std::string name = common::getFullName(FD);
        if (unique_functions.count(name) != 0) {
            filter_functions[AST].insert(i);
            i++;
            continue;
        }
        i++;
        unique_functions.insert(name);

        ASTFunction *F = common::resource->addASTFunction(FD, AF);
        maps[AST]->addFunction(F, FD);
        
        return_functions.push_back(FD);
        return_ast_functions.push_back(F);
        
        std::vector<VarDecl *> variables = common::getVariables(FD);
        
        if (dyn_cast<CXXMethodDecl>(FD)) {
            common::resource->addASTVariable(nullptr, F);
        }

        for (VarDecl *VD : variables) {
            ASTVariable *V = common::resource->addASTVariable(VD, F);
            maps[AST]->addVariable(V, VD);
        }
    }
    
    ASTUnit *au_point = AU.get();
    ASTUnitT AUT(au_point, clock() - start);
     
    if (priorityQueue.size() >= max_size) {
        pop_front();
    }

    priorityQueue.push(AUT);
    collection[AST] = std::move(AU);
    return au_point;
}

void ASTQueue::pop_front() {
    
    auto top = priorityQueue.top();
    ASTUnit *AU = top.get();
    
    priorityQueue.pop();
    maps.erase(AU->getASTFileName());
    collection.erase(AU->getASTFileName());
}

void ASTQueue::load(std::string AST, std::unordered_map<std::string, std::set<unsigned>> &filter_functions, 
        std::vector<ASTFunction *> &return_ast_functions, std::vector<FunctionDecl *> &return_functions) {
    
    if (maps.count(AST) == 0) {

        if (tempASTUnit != nullptr) {
            maps.erase(tempASTUnit->getASTFileName());
        }
        std::unique_ptr<ASTUnit> AU = common::loadFromASTFile(AST);
        ASTUnit *au_point = AU.get();

        maps[AST] = llvm::make_unique<ASTMap>();
        maps[AST]->buildMap(au_point, filter_functions, true);

        tempASTUnit = std::move(AU);
    }

    for (auto &content : maps[AST]->functionMap) {
        return_ast_functions.push_back(content.first);
        return_functions.push_back(content.second);
    }
}


FunctionDecl *ASTQueue::load(ASTFunction *F, std::unordered_map<std::string, std::set<unsigned>> &filter_functions) {
    
    std::string AST = F->getAST();
    if (maps.count(AST) == 0) {

        if (tempASTUnit != nullptr) {
            maps.erase(tempASTUnit->getASTFileName());
        }
        std::unique_ptr<ASTUnit> AU = common::loadFromASTFile(AST);
        ASTUnit *au_point = AU.get();
        
        maps[AST] = llvm::make_unique<ASTMap>();
        maps[AST]->buildMap(au_point, filter_functions, true);

        tempASTUnit = std::move(AU);
    }
    return maps[AST]->functionMap[F];

}

