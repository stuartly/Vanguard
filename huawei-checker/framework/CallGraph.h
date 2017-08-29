
#ifndef CALL_GRAPH_H
#define CALL_GRAPH_H

#include <unordered_map>

#include "framework/ASTElement.h"

class CallGraphNode {

public:
    
    CallGraphNode(ASTFunction *F);

    CallGraphNode(ASTFunction *F, FunctionDecl *FD);
   
    ASTFunction *getFunction() const {
        return F;
    }

    const std::vector<ASTFunction *> &getParents() const {
        return parents;
    }

    const std::vector<ASTFunction *> &getChildren() const {
        return children;
    }
	
    friend class CallGraph;

private:
	 
    ASTFunction *F;

    std::vector<ASTFunction *> parents;
    std::vector<ASTFunction *> children;
    
    std::map<ASTFunction *, std::vector<unsigned>> line_map;

    typedef std::map<std::string, std::vector<unsigned>> ChildrenInfo;
    ChildrenInfo *childrenInfo;

};

/**
 * CallGraph for functions
 */
class CallGraph {
    
public:
    CallGraph() {}
    ~CallGraph();
    
    static void init(ASTFunction *F, FunctionDecl *FD);
    void build();

    const std::vector<ASTFunction *> &getTopLevelFunctions() const;

    ASTFunction *getFunction(FunctionDecl *FD) const;

    const std::vector<ASTFunction *> &getParents(ASTFunction *F) const;
    const std::vector<ASTFunction *> &getChildren(ASTFunction *F) const;

    const std::vector<ASTFunction *> &getParents(FunctionDecl *FD) const;
    const std::vector<ASTFunction *> &getChildren(FunctionDecl *FD) const;
    
    std::vector<unsigned> getChildLineNo(ASTFunction *F, ASTFunction *CF) const;
    
private:
    static std::unordered_map<std::string, CallGraphNode *> nodes;
    std::vector<ASTFunction *> topLevelFunctions;

};

class NonRecursiveCallGraphNode {

public:
    NonRecursiveCallGraphNode() {}

    NonRecursiveCallGraphNode(ASTFunction *F) {
        this->F = F;
    }

    ASTFunction *getFunction() const {
        return F;
    }

    const std::set<ASTFunction *> &getParents() const {
        return parents;
    }

    const std::set<ASTFunction *> &getChildren() const {
        return children;
    }
    
    friend class NonRecursiveCallGraph;

private:

    ASTFunction *F;
    std::set<ASTFunction*> parents;
    std::set<ASTFunction*> children;

};

class NonRecursiveCallGraph {

public:
    NonRecursiveCallGraph(CallGraph *call_graph);
    
    const std::set<ASTFunction *> &getParents(ASTFunction *F) const {
        return nodes.at(F).getParents();
    }
    const std::set<ASTFunction *> &getChildren(ASTFunction *F) const {
        return nodes.at(F).getChildren();
    }

private:

    void spanningTree(ASTFunction *F, std::unordered_map<ASTFunction *, int> &colors);

    std::unordered_map<ASTFunction *, NonRecursiveCallGraphNode> nodes;
    std::vector<ASTFunction *> topLevelFunctions;
    
    std::vector<std::pair<ASTFunction*, ASTFunction*>> removeEdges;

};

#endif
