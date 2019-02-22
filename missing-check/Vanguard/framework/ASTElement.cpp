
#include "framework/Common.h"
#include "framework/ASTElement.h"

ASTFunction::ASTFunction(unsigned id, FunctionDecl *FD, ASTFile *AF) : ASTElement(id, FD->getNameAsString(), AF) {

    fullName = common::getFullName(FD);
    param_size = FD->param_size();
    
    if (dyn_cast<CXXMethodDecl>(FD))
        param_size += 1;

}

ASTVariable::ASTVariable(unsigned id, VarDecl *VD, ASTFunction *F) {
    
    this->id = id;
    this->F = F;
    this->AF = F->getASTFile();
    
    if (VD == nullptr) {
        this->name = "this";
        pointer_reference_type = true;
    }
    else {
        this->name = VD->getNameAsString();
        pointer_reference_type = (VD->getType()->isPointerType() || VD->getType()->isReferenceType());
    }

}


const std::vector<ASTFunction *> &ASTResource::getFunctions() const {

    return ASTFunctions;
}


const std::vector<ASTFunction *> &ASTResource::getFunctions(std::string AST) const {

    return ASTs.at(AST)->getFunctions();

}

std::vector<ASTFile *> ASTResource::getASTFiles() const {

    std::vector<ASTFile *> ASTFiles;
    for (auto &it : ASTs) {
        ASTFiles.push_back(it.second);
    }
    return ASTFiles;
}

ASTFile *ASTResource::addASTFile(std::string AST) {

    unsigned id = ASTs.size();
    ASTFile *AF = new ASTFile(id, AST);
    ASTs[AST] = AF;
    return AF;
}

ASTFunction *ASTResource::addASTFunction(FunctionDecl *FD, ASTFile *AF) {

    unsigned id = ASTFunctions.size();
    ASTFunction *F = new ASTFunction(id, FD, AF);
    ASTFunctions.push_back(F);
    AF->addFunction(F);
    return F;
}

ASTVariable *ASTResource::addASTVariable(VarDecl *VD, ASTFunction *F) {

    unsigned id = F->getVariables().size();
    ASTVariable *V = new ASTVariable(id, VD, F);
    ASTVariables.push_back(V);
    F->addVariable(V);
    return V;
}

ASTResource::~ASTResource() {

    for (auto &content : ASTs) {
        delete content.second;
    }
    for (ASTFunction *F : ASTFunctions) {
        delete F;
    }
    for (ASTVariable *V : ASTVariables) {
        delete V;
    }
}


