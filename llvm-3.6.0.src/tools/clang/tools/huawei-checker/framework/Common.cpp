
#include <fstream>

#include "framework/Common.h"
#include "taint-check/TaintChecker.h"

#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"

#include "clang/Tooling/Tooling.h"
#include "clang/Tooling/CommonOptionsParser.h"

using namespace std;

namespace {

class ASTFunctionLoad : public ASTConsumer,
                        public RecursiveASTVisitor<ASTFunctionLoad> {

public:
  void HandleTranslationUnit(ASTContext &Context) override {
    TranslationUnitDecl *TUD = Context.getTranslationUnitDecl();
    TraverseDecl(TUD);
  }

  bool TraverseDecl(Decl *D) {
    if (!D)
      return true;
    bool rval = true;
    if (D->getASTContext().getSourceManager().isInMainFile(D->getLocation()) ||
        D->getKind() == Decl::TranslationUnit) {
      rval = RecursiveASTVisitor<ASTFunctionLoad>::TraverseDecl(D);
    }
    return rval;
  }
  bool TraverseFunctionDecl(FunctionDecl *FD) {
    if (FD && FD->isThisDeclarationADefinition()) {
      functions.push_back(FD);
    }
    return true;
  }

  bool TraverseStmt(Stmt *S) { return true; }

  const std::vector<FunctionDecl *> &getFunctions() const { return functions; }

private:
  std::vector<FunctionDecl *> functions;
};

class FunctionDeclLoad : public ASTConsumer,
                         public RecursiveASTVisitor<FunctionDeclLoad> {

public:
  void HandleTranslationUnit(ASTContext &Context) override {
    TranslationUnitDecl *TUD = Context.getTranslationUnitDecl();
    TraverseDecl(TUD);
  }

  bool VisitFunctionDecl(FunctionDecl *FD) {
    if (FD) {
      functions.push_back(FD);
    }
    return true;
  }

  std::vector<FunctionDecl *> &getFunctions(ASTUnit *AU) {
    HandleTranslationUnit(AU->getASTContext());
    return functions;
  }

private:
  std::vector<FunctionDecl *> functions;
};

class ASTVariableLoad : public RecursiveASTVisitor<ASTVariableLoad> {

public:
  bool VisitDeclStmt(DeclStmt *S) {
    for (auto D : S->decls()) {
      if (VarDecl *VD = dyn_cast<VarDecl>(D)) {
        variables.push_back(VD);
      }
    }
    return true;
  }

  const std::vector<VarDecl *> &getVariables() { return variables; }

private:
  std::vector<VarDecl *> variables;
};

class ASTCalledFunctionLoad
    : public RecursiveASTVisitor<ASTCalledFunctionLoad> {

public:
  bool VisitCallExpr(CallExpr *E) {
    if (FunctionDecl *FD = E->getDirectCallee()) {
      functions.insert(FD);
    }
    return true;
  }

  const std::vector<FunctionDecl *> getFunctions() {
    return std::vector<FunctionDecl *>(functions.begin(), functions.end());
  }

private:
  std::set<FunctionDecl *> functions;
};

class ASTCallExprLoad : public RecursiveASTVisitor<ASTCallExprLoad> {

public:
  bool VisitCallExpr(CallExpr *E) {
    if (!E)
      return true;
    call_exprs.push_back(E);
    return true;
  }

  const std::vector<CallExpr *> &getCallExprs() { return call_exprs; }

private:
  std::vector<CallExpr *> call_exprs;
};

} // end of anonymous namespace

namespace common {

/**
 * load an ASTUnit from ast file.
 * AST : the name of the ast file.
 */
std::unique_ptr<ASTUnit> loadFromASTFile(std::string AST) {

  FileSystemOptions FileSystemOpts;
  IntrusiveRefCntPtr<DiagnosticsEngine> Diags =
      CompilerInstance::createDiagnostics(new DiagnosticOptions());
  return ASTUnit::LoadFromASTFile(AST, Diags, FileSystemOpts);
}

/**
 * get all functions's decl from an ast context.
 */
std::vector<FunctionDecl *> getFunctions(ASTContext &Context) {

  ASTFunctionLoad load;
  load.HandleTranslationUnit(Context);
  return load.getFunctions();
}

std::vector<FunctionDecl *> getFunctionDecl(ASTUnit *AU) {
  FunctionDeclLoad load;
  return load.getFunctions(AU);
}

llvm::cl::OptionCategory listToolCategory("");
std::set<std::string> loadFromSourceFile(std::string file) {

  std::set<std::string> functions;
  using namespace clang::tooling;

  const char *argv[] = {"", file.c_str(), "--"};
  int argc = sizeof(argv) / sizeof(char *);
  CommonOptionsParser parser(argc, argv, listToolCategory);

  std::vector<std::string> paths;
  paths.push_back(file);
  ClangTool tool(parser.getCompilations(), paths);

  std::vector<std::unique_ptr<ASTUnit>> list;
  tool.buildASTs(list);

  if (list.size() > 0) {
    ASTUnit *au = list[0].get();
    for (auto FD : getFunctionDecl(au)) {
      functions.insert(common::getFullName(FD));
    }
  }
  return functions;
}

/**
 * get all variables' decl of a function
 * FD : the function decl.
 */
std::vector<VarDecl *> getVariables(FunctionDecl *FD) {

  std::vector<VarDecl *> variables;
  variables.insert(variables.end(), FD->params().begin(), FD->params().end());

  ASTVariableLoad load;
  load.TraverseStmt(FD->getBody());
  variables.insert(variables.end(), load.getVariables().begin(),
                   load.getVariables().end());

  return variables;
}

std::vector<FunctionDecl *> getCalledFunctions(FunctionDecl *FD) {

  ASTCalledFunctionLoad load;
  load.TraverseStmt(FD->getBody());
  return load.getFunctions();
}

std::vector<CallExpr *> getCallExpr(FunctionDecl *FD) {

  ASTCallExprLoad load;
  load.TraverseStmt(FD->getBody());
  return load.getCallExprs();
}

std::string getParams(FunctionDecl *FD) {

  std::string params = "";
  for (auto param : FD->params()) {
    params = params + param->getOriginalType().getAsString() + "  ";
  }
  return params;
}

std::string getFullName(FunctionDecl *FD) {

  std::string name = FD->getQualifiedNameAsString();

  name = name + "  " + getParams(FD);
  return name;
}

std::vector<ASTFunction *> getTopoOrder() {

  std::vector<unsigned> in_size;
  std::map<unsigned, std::unordered_set<ASTFunction *>> collection;

  for (ASTFunction *F : common::resource->getFunctions()) {
    unsigned n = common::call_graph->getParents(F).size();
    in_size.push_back(n);
    collection[n].insert(F);
  }

  std::vector<ASTFunction *> result;

  while (!collection.empty()) {

    if (collection.begin()->second.size() == 0) {
      collection.erase(collection.begin());
      continue;
    }

    ASTFunction *F = *(collection.begin()->second.begin());

    collection.begin()->second.erase(F);

    result.push_back(F);

    for (ASTFunction *CF : common::call_graph->getChildren(F)) {

      unsigned size = in_size[CF->getID()];
      if (in_size[CF->getID()] != 0)
        in_size[CF->getID()]--;

      if (collection[size].erase(CF) == 1) {
        collection[size - 1].insert(CF);
      }
    }
  }
  return result;
}

std::string getStringStmt(const Stmt *stmt, const LangOptions &LO) {
  if (stmt == nullptr)
    return "";

  string buffer;
  llvm::raw_string_ostream exprOS(buffer);

  stmt->printPretty(exprOS, nullptr, PrintingPolicy(LO));
  return exprOS.str();
}

} // end of namespace common

std::string trim(std::string s) {
  std::string result = s;
  result.erase(0, result.find_first_not_of(" \t"));
  result.erase(result.find_last_not_of(" \t") + 1);
  return result;
}

std::vector<std::string> initializeASTList(std::string astList) {

  std::vector<std::string> astFiles;

  std::ifstream fin(astList);

  std::string line;
  while (getline(fin, line)) {
    line = trim(line);
    if (line == "")
      continue;
    std::string fileName = line;
    astFiles.push_back(fileName);
  }
  fin.close();

  return astFiles;
}

Json::Value initializeConfigure(std::string config) {
  Json::Value configure;
  std::ifstream file(config);
  file >> configure;
  return configure;
}

void initializePathReport(int argc, const char *argv[]) {

  (*common::configure)["pathToReport"]["path"] = "./";

  if (argc >= 4) {
    std::string path = argv[3];
    if (path[path.size() - 1] != '\\' && path[path.size() - 1] != '/') {
      path += "/";
    }
    (*common::configure)["pathToReport"]["path"] = path;
  }
}

void initializeBlackWhite(int argc, const char *argv[]) {

  std::string path = "./";
  if (argc == 5) {
    path = argv[4];
    if (path[path.size() - 1] != '\\' && path[path.size() - 1] != '/') {
      path += "/";
    }
  }

  ifstream blackFile(path + "black.cpp");
  if (blackFile.is_open()) {
    taint::TaintChecker::blackList =
        common::loadFromSourceFile(path + "black.cpp");
  }
  blackFile.close();

  ifstream whiteFile(path + "white.cpp");
  if (whiteFile.is_open()) {
    taint::TaintChecker::whiteList =
        common::loadFromSourceFile(path + "white.cpp");
  }
  whiteFile.close();
}

void process_bar(float progress) {

  int barWidth = 70;
  std::cout << " [";

  int pos = progress * barWidth;
  for (int i = 0; i < barWidth; i++) {
    if (i < pos)
      std::cout << "|";
    else
      std::cout << " ";
  }
  std::cout << "] " << int(progress * 100.0) << "%\r";
  std::cout.flush();
}
