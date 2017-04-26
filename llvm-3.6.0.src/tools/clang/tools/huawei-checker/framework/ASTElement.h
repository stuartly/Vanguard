
#ifndef AST_ELEMENT_H
#define AST_ELEMENT_H

#include <string>
#include <vector>
#include <unordered_map>

#include "clang/AST/AST.h"
#include "clang/Frontend/ASTUnit.h"

using namespace clang;

class ASTFunction;
class ASTVariable;

class ASTFile {

public:
  ASTFile(unsigned id, std::string AST) : id(id), AST(AST){};

  const std::string &getAST() const { return AST; }

  void addFunction(ASTFunction *F) { functions.push_back(F); }

  unsigned getID() const { return id; }

  const std::vector<ASTFunction *> &getFunctions() const { return functions; }

private:
  unsigned id;
  std::string AST;

  std::vector<ASTFunction *> functions;
};

class ASTElement {

public:
  ASTElement() {}

  ASTElement(unsigned id, std::string name, ASTFile *AF)
      : id(id), name(name), AF(AF) {}

  unsigned getID() const { return id; }

  const std::string &getName() const { return name; }

  ASTFile *getASTFile() const { return AF; }

  const std::string &getAST() const { return AF->getAST(); }

protected:
  unsigned id;
  std::string name;

  ASTFile *AF;
};

class ASTFunction : public ASTElement {

public:
  ASTFunction(unsigned id, FunctionDecl *FD, ASTFile *AF);

  void addVariable(ASTVariable *V) { variables.push_back(V); }

  unsigned getParamSize() const { return param_size; }

  const std::string &getFullName() const { return fullName; }

  const std::vector<ASTVariable *> &getVariables() const { return variables; }

private:
  std::string fullName;
  unsigned param_size;

  std::vector<ASTVariable *> variables;
};

class ASTVariable : public ASTElement {

public:
  ASTVariable(unsigned id, VarDecl *VD, ASTFunction *F);

  ASTFunction *getFunction() const { return F; }

  bool isPointerOrReferenceType() const { return pointer_reference_type; }

private:
  bool pointer_reference_type;

  ASTFunction *F;
};

class ASTResource {

public:
  ~ASTResource();

  const std::vector<ASTFunction *> &getFunctions() const;
  std::vector<ASTFile *> getASTFiles() const;

  const std::vector<ASTFunction *> &getFunctions(std::string AST) const;

  friend class ASTQueue;

private:
  std::unordered_map<std::string, ASTFile *> ASTs;
  std::vector<ASTFunction *> ASTFunctions;
  std::vector<ASTVariable *> ASTVariables;

  ASTFile *addASTFile(std::string AST);
  ASTFunction *addASTFunction(FunctionDecl *FD, ASTFile *AF);
  ASTVariable *addASTVariable(VarDecl *VD, ASTFunction *F);
};

#endif
