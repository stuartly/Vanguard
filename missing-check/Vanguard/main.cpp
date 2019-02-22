
#include <fstream>
#include <iostream>

#include "clang/Frontend/CompilerInstance.h"

#include "framework/Common.h"
#include "framework/BasicChecker.h"

#include "taint-check/TaintChecker.h"
#include "taint-check-div/DivChecker.h"
#include "dangling-pointer/DanglingPointer.h"
#include "arraybound/ArrayOutOfBoundsCheckerV4.h"
#include "taint-memory-op/TaintedMemOPChecker.h"
#include "recursive-call/RecursiveCallChecker.h"

namespace common {

    Config* configure = nullptr;
    ASTManager *manager = nullptr;
    ASTResource* resource = nullptr;
    CallGraph *call_graph = nullptr;
}

using namespace taint;

int main(int argc, const char *argv[]) {

    clock_t start = clock();
    
    if (argc < 3 || argc > 5) {
        std::cerr << "usage: huawei-checker astList.txt config.json [pathToReport] [pathToBlackWhiteList]" << std::endl;
        return -1;
    }
    
    std::vector<std::string> ASTFiles = initializeASTList(argv[1]);
    Config configure = initializeConfigure(argv[2]);
    common::configure = &configure;
    initializePathReport(argc, argv);
    initializeBlackWhite(argc, argv);

    ofstream process_file((*common::configure)["pathToReport"]["path"].asString() + "time.txt");

    // global variable
    ASTResource resource;
    common::resource = &resource;
    CallGraph call_graph;
    common::call_graph = &call_graph;
    
    // create ast manager and initialize
    ASTManager manager(ASTFiles);
    common::manager = &manager;
    common::call_graph->build();
    
    process_file << "Initialize AST : " << float(clock() - start) / CLOCKS_PER_SEC << "sec" << std::endl;

    TaintChecker taint_checker;
    
    // non recursive callgraph
    NonRecursiveCallGraph nonRecursiveCallGraph(common::call_graph);

    unsigned i = 0;
    std::vector<ASTFunction *> topoOrder = common::getTopoOrder();
    
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
    
    process_file << "TaintChecker   : " << float(taint_checker.get_time()) / CLOCKS_PER_SEC << "sec" << std::endl;
    
    TaintedMemOPChecker MOPChecker;
    ArrayOutOfBoundsCheckerV4 arraybound;
    DivChecker div_checker;
    RecursiveCallChecker recursiveCallChecker; 
    DanglingPtr DPChecker;


    std::cout << "MissingChecker : " << std::endl;
    unsigned j = 0;
    unsigned func_size = common::resource->getFunctions().size();
    int ast_num =0, func_num =0;

 
   for (std::string AST : ASTFiles) {
	    ast_num ++;
        std::vector<ASTFunction *> ast_functions;
        std::vector<FunctionDecl *> functions;
        common::manager->load(AST, ast_functions, functions);
        
        for (unsigned i = 0; i < ast_functions.size(); i++) {
            func_num++;
            ASTFunction *F = ast_functions[i];
            FunctionDecl *FD = functions[i];
            CFG *function_cfg = common::manager->getCFG(F, FD);

            if((*common::configure)["CheckerEnable"]["recursiveCall"].asBool()) {
                recursiveCallChecker.storeFunctionLocation(F, FD);
            }

            if((*common::configure)["CheckerEnable"]["arrayBound"].asBool()){
                arraybound.check(F, FD, function_cfg);
            }
            
            if((*common::configure)["CheckerEnable"]["divideChecker"].asBool()){
                div_checker.check(F, FD, function_cfg);
            }     

            if((*common::configure)["CheckerEnable"]["memoryOPChecker"].asBool()){
                MOPChecker.check(F, FD, function_cfg);
            }

	        if((*common::configure)["CheckerEnable"]["danglingPointer"].asBool()){
	           DPChecker.check(F, FD, function_cfg);
            }  
              
            j++;
            process_bar(float(j) / func_size);
        }
        common::manager->clear();
    }

    std::cout << std::endl;
    
	if((*common::configure)["CheckerEnable"]["recursiveCall"].asBool()) {
        recursiveCallChecker.check();
    }

    process_file << "AST            : " << ast_num << std::endl;
    process_file << "Function       : " << func_num << std::endl;
    process_file << "RecursiveCall  : " << float(recursiveCallChecker.getRuntime()) / CLOCKS_PER_SEC << "sec" << std::endl;
    process_file << "ArrayBound     : " << float(arraybound.get_time()) / CLOCKS_PER_SEC << "sec" << std::endl;
    process_file << "DivChecker     : " << float(div_checker.get_time()) / CLOCKS_PER_SEC << "sec" << std::endl;
    process_file << "MemoryOPChecker: " << float(MOPChecker.get_time()) / CLOCKS_PER_SEC << "sec" << std::endl;
    process_file << "DanglingPointer: " << float(DPChecker.get_time()) / CLOCKS_PER_SEC << "sec" << std::endl;    
    process_file << "total time : " << float(clock() - start) / CLOCKS_PER_SEC << "sec" << std::endl;

    return 0;
}

