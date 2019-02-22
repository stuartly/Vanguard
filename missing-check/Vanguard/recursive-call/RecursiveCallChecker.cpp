#include "RecursiveCallChecker.h"

#include <ctime>
#include <iostream>
#include <sstream>
#include <string>

#include "Bimap.h"
#include "Digraph.h"
#include "ElementaryCyclesSearch.h"
#include "TarjanSCC.h"
#include "ValueObject.h"
#include "XmlWriter.h"

#include "framework/Logger.h"

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/Basic/SourceManager.h"

void RecursiveCallChecker::storeFunctionLocation(ASTFunction *F, FunctionDecl *FD) {
    clock_t start = clock();
    std::string location = FD->getLocation().printToString(FD->getASTContext().getSourceManager());
      
    size_t spell_pos = location.rfind("<Spelling=");
    if (spell_pos != std::string::npos) {
        location.erase(spell_pos);
    }
    functionLocation[F] = location; 
    clock_t end = clock();
    runtime += (end - start);
}

void RecursiveCallChecker::check() {

    clock_t start = clock();

    // construct a directed graph using call graph
    // use a bidirectional map to query index of vertice of a funtiondecl node or the reverse
    Bimap<ASTFunction*, int> functionIndexMap;
    std::vector<ASTFunction*> functions= common::resource->getFunctions();

    int id = 0;
    for (auto itr = functions.begin(); itr != functions.end(); ++itr, ++id) {
        functionIndexMap.add(*itr, id);
    }

    // the directed graph use int 0,1,2,...,N-1 to represent N vertices
    Digraph G(functions.size());

    // add edges to the graph
    for (auto itr = functions.begin(); itr != functions.end(); ++itr) {
        std::vector<ASTFunction *> children = common::call_graph->getChildren(*itr);
        for (auto citr = children.begin(); citr != children.end(); ++citr) {
            G.addEdge(functionIndexMap.getRight(*itr), functionIndexMap.getRight(*citr));
        }
    }

    Logger().log(TCheck::RECURSIVE_CALL, TLogLevel::LOG_DEBUG_2) << G << std::endl;

    std::vector<int> loops;
    // traverse all vertices of the call graph to find all recursive call with size 1(loops) because tarjan strongly connected component algorithm can not handle them
    for (int i = 0; i < G.numOfVertices(); ++i) {
        if (G.hasEdge(i, i)) {
            loops.push_back(i); 
            G.removeEdge(i, i);
        } 
    } 

    Logger().log(TCheck::RECURSIVE_CALL, TLogLevel::LOG_DEBUG_2) << "============================ LOOPS FINISHED ==================================" << std::endl;
    Logger().log(TCheck::RECURSIVE_CALL, TLogLevel::LOG_DEBUG_2) << "FIND FOLLOWING LOOPS:\n";
    for (int loop : loops) {
        Logger().log(TCheck::RECURSIVE_CALL, TLogLevel::LOG_DEBUG_2) << loop << "\n";
    }


    // use tarjan strongly connected components algorithm to find all strongly connected components in the call graph
    TarjanSCC SCCFinder(G);

    // use a map structure to store all the strongly connected components
    std::map<int, std::vector<int>> SCCMap;
    for (int i = 0; i < G.numOfVertices(); ++i) {
        auto itr = SCCMap.find(SCCFinder.id(i));
        if (itr != SCCMap.end()) {
            (itr->second).push_back(i);
        } else {
            std::vector<int> v;
            v.push_back(i);
            SCCMap.insert(std::pair<int, std::vector<int>>(SCCFinder.id(i), v));
        }
    }

    // remove strongly connected components with size 1 from the map 
    for (auto itr = SCCMap.cbegin(); itr != SCCMap.cend();) {
        if ((itr->second).size() == 1) {
            SCCMap.erase(itr++); 
        } else {
            itr++; 
        }
    }

    Logger().log(TCheck::RECURSIVE_CALL, TLogLevel::LOG_DEBUG_2) << "============================ SCC FINISHED ==================================" << std::endl;
    Logger().log(TCheck::RECURSIVE_CALL, TLogLevel::LOG_DEBUG_2) << "Find following strongly connected components:\n";
    for (auto itr = SCCMap.begin(); itr != SCCMap.end(); ++itr) {
        Logger().log(TCheck::RECURSIVE_CALL, TLogLevel::LOG_DEBUG_2) << (itr->second).size() << "\t:";
        for (auto v : itr->second) {
            Logger().log(TCheck::RECURSIVE_CALL, TLogLevel::LOG_DEBUG_2) << v << " ";  
        } 
        Logger().log(TCheck::RECURSIVE_CALL, TLogLevel::LOG_DEBUG_2) << "\n";
    }
    Logger().log(TCheck::RECURSIVE_CALL, TLogLevel::LOG_DEBUG_2) << "\n";


    // construct a subgraph according to every strongly connected components and find cycles in it   

    unsigned threshold = (*common::configure)["RecursiveCall"]["max_scc_size"].asInt();
    std::vector<std::vector<int>> largeSCC;
    std::vector<std::vector<int>> cycles;
    for (auto itr = SCCMap.cbegin(); itr != SCCMap.cend(); ++itr) {
        std::vector<int> SCC = itr->second; 

        // if the strongly connected component is larger than a threshold, we just output it
        if (SCC.size() > threshold) {
            largeSCC.push_back(SCC);
            continue;
        }

        Bimap<int, int> vertexMap;
        for (unsigned i = 0; i < SCC.size(); ++i) {
            vertexMap.add(i, SCC.at(i)); 
        }

        // construct the subgraph according to the original graph
        Digraph subgraph(SCC.size());
        for (int i : SCC) {
            for(int j : SCC) {
                if (G.hasEdge(i, j)) {
                    subgraph.addEdge(vertexMap.getLeft(i), vertexMap.getLeft(j)); 
                }
            }
        }

        ElementaryCyclesSearch cycleFinder(subgraph); 

        Logger().log(TCheck::RECURSIVE_CALL, TLogLevel::LOG_DEBUG_2) << "=================================HANLDE SUBGRAPH===========================" << std::endl;
        Logger().log(TCheck::RECURSIVE_CALL, TLogLevel::LOG_DEBUG_2) << "THE GRAPH HAS " << subgraph << std::endl;

        std::vector<std::vector<int>> cyclesInSubgraph = cycleFinder.getElementaryCycles();
        // convert cycles in subgraph to cycles in the original graph
        for (auto c : cyclesInSubgraph) {
            std::vector<int> cycle;
            for (unsigned i = 0; i < c.size(); ++i) {
                cycle.push_back(vertexMap.getRight(c[i]));  
            }
            cycles.push_back(cycle);
        }

    }  

    
    Logger().log(TCheck::RECURSIVE_CALL, TLogLevel::LOG_DEBUG_2) << "Find following cycles:\n";
    for (auto cycle : cycles) {
        for (int v : cycle) {
            Logger().log(TCheck::RECURSIVE_CALL, TLogLevel::LOG_DEBUG_2) << v << " "; 
        } 
        Logger().log(TCheck::RECURSIVE_CALL, TLogLevel::LOG_DEBUG_2) << "\n";
    }

    Logger().log(TCheck::RECURSIVE_CALL, TLogLevel::LOG_DEBUG_2) << "Find following large strongly connected components:\n";
    for (auto SCC : largeSCC) {
        for (int v : SCC) {
            Logger().log(TCheck::RECURSIVE_CALL, TLogLevel::LOG_DEBUG_2) << v << " "; 
        } 
        Logger().log(TCheck::RECURSIVE_CALL, TLogLevel::LOG_DEBUG_2) << "\n";
    }

    // encapsulate loops, cycles and large storngly connected components to value objects and output to xml
    std::vector<RecursiveCall> recursiveCalls;

    for (auto loop : loops) {
        ASTFunction* F = functionIndexMap.getLeft(loop); 
        Function function(F, functionLocation[F]);
        FunctionCall functionCall;
        functionCall.caller = function;
        functionCall.callee = function;
        std::vector<unsigned> callPositionLineNo = common::call_graph->getChildLineNo(F, F);
        functionCall.line = callPositionLineNo.front();

        std::vector<Function> functions;
        std::vector<FunctionCall> functionCalls;
        functions.push_back(function);
        functionCalls.push_back(functionCall);

        RecursiveCall recursiveCall(functions, functionCalls);
        recursiveCalls.push_back(recursiveCall);
    }

    for (auto cycle : cycles) {
        std::vector<Function> functions;
        for (auto e : cycle) {
            ASTFunction *F = functionIndexMap.getLeft(e);
            Function function(F, functionLocation[F]);    
            functions.push_back(function);
        }

        std::vector<FunctionCall> functionCalls;
        for (unsigned i = 0; i < cycle.size(); ++i) {
            ASTFunction *FCR = functionIndexMap.getLeft(cycle[i]);
            ASTFunction *FCE = functionIndexMap.getLeft(cycle[(i+1) % cycle.size()]);

            Function caller(FCR, functionLocation[FCR]);
            Function callee(FCE, functionLocation[FCE]);

            FunctionCall functionCall; 
            functionCall.caller = caller;
            functionCall.callee = callee;
            functionCall.line = common::call_graph->getChildLineNo(FCR, FCE).front();
            functionCalls.push_back(functionCall);
        } 

        RecursiveCall recursiveCall(functions, functionCalls);
        recursiveCalls.push_back(recursiveCall);
    }

    std::vector<StronglyConnectedComponent> stronglyConectedComponents;
    for (auto SCC : largeSCC) {
        std::vector<Function> functions;
        std::vector<FunctionCall> functionCalls;
        for (int v : SCC) {
            ASTFunction *F = functionIndexMap.getLeft(v);
            Function function(F, functionLocation[F]);
            functions.push_back(function);
        }
        for (int v: SCC) {
            for (int u : SCC) {
                FunctionCall functionCall;
                ASTFunction *FCR = functionIndexMap.getLeft(v);
                ASTFunction *FCE = functionIndexMap.getLeft(u);
                std::vector<unsigned> lineNo = common::call_graph->getChildLineNo(FCR, FCE); 
                if (!lineNo.empty()) {
                    functionCall.caller = Function(FCR, functionLocation[FCR]); 
                    functionCall.callee = Function(FCE, functionLocation[FCE]);
                    functionCall.line= lineNo.front();
                    functionCalls.push_back(functionCall);
                }
            }  
        }
        StronglyConnectedComponent stronglyConectedComponent(functions, functionCalls);
        stronglyConectedComponents.push_back(stronglyConectedComponent);
    }

    XmlWriter writer;
    writer.writeToXml((*common::configure)["pathToReport"]["path"].asString() + "MOLINT.RECURSIVE.CALL.xml", recursiveCalls, stronglyConectedComponents);

    clock_t end = clock();
    runtime += (end - start);
}

clock_t RecursiveCallChecker::getRuntime() {
    return runtime;
}

