
#include <fstream>
#include <iostream>

#include "clang/Frontend/CompilerInstance.h"

#include "framework/Common.h"
#include "framework/BasicChecker.h"
#include "taint-check/TaintInterface.h"

#include "taint-memory-op/TaintedMemOPChecker.h"
#include "arraybound/ArrayOutOfBoundsCheckerV4.h"
#include "taint-check-div/DivChecker.h"
#include "BufferToStruct/BufferToStruct.h"
#include "PreCheckDataSet/PreCheckDataSet.h"
//#include "CollectData/CollectData.h"

//#define CHECK 0

namespace common {

Config *configure = nullptr;
ASTManager *manager = nullptr;
ASTResource *resource = nullptr;
CallGraph *call_graph = nullptr;
}

using namespace taint;
using namespace CheckDataSet;
using namespace std;

int main(int argc, const char *argv[]) {

  clock_t start = clock();

  // input check
  if (argc < 3 || argc > 5) {
    std::cerr << "usage: huawei-checker astList.txt config.json [pathToReport] "
                 "[pathToBlackWhiteList]" << std::endl;
    return -1;
  }

  // parsing input
  std::vector<std::string> ASTFiles = initializeASTList(argv[1]);
  Config configure = initializeConfigure(argv[2]);
  common::configure = &configure;
  initializePathReport(argc, argv);
  initializeBlackWhite(argc, argv);

  ofstream process_file(
      (*common::configure)["pathToReport"]["path"].asString() + "time.txt");

  ASTResource resource;
  common::resource = &resource;
  CallGraph call_graph;
  common::call_graph = &call_graph;

  // load AST and construct CG
  ASTManager manager(ASTFiles);
  common::manager = &manager;
  common::call_graph->build();

#if CHECK
  common::call_graph->dump();
#endif

  process_file << "Initialize AST/CallGraph : "
               << float(clock() - start) / CLOCKS_PER_SEC << "sec" << std::endl;

  clock_t taint = clock();
  TaintChecker taint_checker;
  unsigned i = 0;
  std::vector<ASTFunction *> topoOrder = common::getTopoOrder();

#if CHECK
  cout << "[CHECK] [topo order] " << topoOrder.size() << endl;
  for (auto it = topoOrder.begin(); it != topoOrder.end(); ++it) {
    // the order is not deterministic
    // cout << "[CHECK] [topo order] " << (*it)->getFullName() << endl;
  }
#endif

  // calculate taint environment
  std::cout << "Taint Checker  :" << std::endl;
  for (auto it = topoOrder.rbegin(); it != topoOrder.rend(); it++) {

    ASTFunction *F = *it;
    FunctionDecl *FD = common::manager->load(F);
    CFG *function_cfg = common::manager->getCFG(F, FD);

    taint_checker.check(F, FD, function_cfg);

    i++;
    process_bar(float(i) / topoOrder.size());
  }
  std::cout << std::endl;

  taint_checker.build(topoOrder);

#if CHECK
  taint::printTaintEnvironment();
#endif

  process_file << "TaintChecker_get_time   : "
               << float(taint_checker.get_time()) / CLOCKS_PER_SEC << "sec"
               << std::endl;
  process_file << "Taint analysis   : "
               << float(clock() - taint) / CLOCKS_PER_SEC << "sec" << std::endl;

  clock_t missingcheck = clock();

  TaintedMemOPChecker MOPChecker;
 // ArrayOutOfBoundsCheckerV4 arraybound;
 // DivChecker div_checker;
 
  std::cout << "Missing Check : " << std::endl;
  unsigned j = 0;
  unsigned func_size = common::resource->getFunctions().size();
  int function_nums = 0;
  int files_num = 0;

  clock_t sumAST = clock();

 

  for (std::string AST : ASTFiles) {

    clock_t beginAST = clock();
    files_num++;

    std::vector<ASTFunction *> ast_functions;
    std::vector<FunctionDecl *> functions;
    common::manager->load(AST, ast_functions, functions);
    unsigned astfunction_num = ast_functions.size();

    for (unsigned i = 0; i < ast_functions.size(); i++) {

      ASTFunction *F = ast_functions[i];
      FunctionDecl *FD = functions[i];
      CFG *function_cfg = common::manager->getCFG(F, FD);
      function_nums++;
/*
     if((*common::configure)["CheckerEnable"]["arrayBound"].asBool()) {
                arraybound.check(F, FD, function_cfg);
     }

     if((*common::configure)["CheckerEnable"]["divideChecker"].asBool()) {
                div_checker.check(F, FD, function_cfg);
     }
*/

     if ((*common::configure)["CheckerEnable"]["memoryOPChecker"].asBool()) {
        MOPChecker.check(F, FD, function_cfg);
     }

     j++;

     process_bar(float(j) / func_size);
    }

    common::manager->clear();
    process_file << " |ASTName|" << AST << "|ASTfunction|" << astfunction_num
                 << "|astTime|" << float(clock() - beginAST) / CLOCKS_PER_SEC
                 << "|sec|"
                 << "|ASTNums|" << files_num << "|ASTfuns_now |"
                 << function_nums << "|ASTfunsTime|"
                 << float(clock() - sumAST) / CLOCKS_PER_SEC << "| sec"
                 << std::endl;
  }
  std::cout << std::endl;
  process_file << "MemoryOPChecker: "
               << float(clock() - missingcheck) / CLOCKS_PER_SEC << "sec"
               << std::endl;

  clock_t repair = clock();
  PreCheckDataSet CheckDataSet;
  std::cout << "Generating Repair: " << std::endl;
  unsigned n = 0;
  

  for (std::string AST : ASTFiles) {
    std::vector<ASTFunction *> ast_functions;
    std::vector<FunctionDecl *> functions;
    common::manager->load(AST, ast_functions, functions);
    unsigned astfunction_num = ast_functions.size();

    for (unsigned i = 0; i < ast_functions.size(); i++) {

      ASTFunction *F = ast_functions[i];
      FunctionDecl *FD = functions[i];
      CFG *function_cfg = common::manager->getCFG(F, FD);

      if ((*common::configure)["CheckerEnable"]["PreCheckDataSet"].asBool()) {
        CheckDataSet.check(F, FD, function_cfg);
      }
      n++;

      process_bar(float(n) / func_size);
    }

    common::manager->clear();
  }
  std::cout << std::endl;


  process_file << "Generating Repair: "
               << float(clock() - repair) / CLOCKS_PER_SEC << "sec"
               << std::endl;

  process_file << "totol files: " << files_num << std::endl;
  process_file << "totol functions: " << function_nums << std::endl;

 /*
  process_file << "PreCheckDataSet: "
               << float(CheckDataSet.get_time()) / CLOCKS_PER_SEC << "sec"
               << std::endl;
               */

  process_file << "Detecting_get_time: "
               << float(MOPChecker.get_Detect_time()) / CLOCKS_PER_SEC << "sec"
               << std::endl;

  process_file << "ConfirmDangerous_get_time: "
               << float(MOPChecker.get_Confirm_time()) / CLOCKS_PER_SEC << "sec"
               << std::endl;

  process_file << "RepairGenerating_get_time: "
               << float(MOPChecker.get_Repair_time()) / CLOCKS_PER_SEC << "sec"
               << std::endl;

  process_file << "Total time: " << float(clock() - start) / CLOCKS_PER_SEC
               << "sec" << std::endl;

  return 0;
}
