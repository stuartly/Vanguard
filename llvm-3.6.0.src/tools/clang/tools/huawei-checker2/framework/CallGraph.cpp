
#include <iostream>

#include "framework/Common.h"
#include "framework/CallGraph.h"

using namespace std;

CallGraphNode::CallGraphNode(ASTFunction *F, FunctionDecl *FD) {

    this->F = F;
    childrenInfo = new ChildrenInfo;

    for (CallExpr *E : common::getCallExpr(FD)) {
        if (FunctionDecl *CFD = E->getDirectCallee()) {
            
            std::string location = E->getLocStart().printToString(FD->getASTContext().getSourceManager());
            size_t spell_pos = location.rfind("<Spelling=");
            if (spell_pos != std::string::npos) {
                location.erase(spell_pos);
            }
            std::size_t colon_pos = location.rfind(":");
            std::string s(location);
            s.erase(colon_pos, s.length() - colon_pos); 
            colon_pos = s.rfind(":");
            unsigned line = std::stoi(s.substr(colon_pos + 1, location.length() - colon_pos - 1));
            (*childrenInfo)[common::getFullName(CFD)].push_back(line);
        }
    }
}

std::unordered_map<std::string, CallGraphNode *> CallGraph::nodes;

void CallGraph::init(ASTFunction *F, FunctionDecl *FD) {
    
    nodes[F->getFullName()] = new CallGraphNode(F, FD);
}

void CallGraph::build() {
    
    for (auto &content : nodes) {

        CallGraphNode *node = content.second;

        for (auto &content : *(node->childrenInfo)) {
            std::string childName = content.first;

            auto it = nodes.find(childName);
            if (it != nodes.end()) {
                node->children.push_back(it->second->getFunction());
                it->second->parents.push_back(node->getFunction());

                node->line_map[it->second->getFunction()] = content.second;
            }
        }
        delete node->childrenInfo;
    }
    
    for (auto &content : nodes) {

        ASTFunction *F = content.second->getFunction();
        if (nodes[F->getFullName()]->getParents().size() == 0) {
            topLevelFunctions.push_back(F);
        }
    }

}

CallGraph::~CallGraph() {
    for (auto &content : nodes) {
        delete content.second;
    }
}

const std::vector<ASTFunction *> &CallGraph::getTopLevelFunctions() const {
    return topLevelFunctions;
}

ASTFunction *CallGraph::getFunction(FunctionDecl *FD) const {

    if (FD == nullptr)
        return nullptr;
	
    std::string fullName = common::getFullName(FD);
    auto it = nodes.find(fullName);
    if (it != nodes.end()) {
        return it->second->getFunction();
    }
    return nullptr;
}

const std::vector<ASTFunction *> &CallGraph::getParents(ASTFunction *F) const {
    auto it = nodes.find(F->getFullName());
    return it->second->getParents();
}

const std::vector<ASTFunction *> &CallGraph::getChildren(ASTFunction *F) const {
    auto it = nodes.find(F->getFullName());
    return it->second->getChildren();
}

const std::vector<ASTFunction *> &CallGraph::getParents(FunctionDecl *FD) const {
    auto it = nodes.find(common::getFullName(FD));
    return it->second->getParents();
}

const std::vector<ASTFunction *> &CallGraph::getChildren(FunctionDecl *FD) const {
    auto it = nodes.find(common::getFullName(FD));
    return it->second->getChildren();
}


std::vector<unsigned> CallGraph::getChildLineNo(ASTFunction *F, ASTFunction *CF) const {

    auto it = nodes.find(F->getFullName());
    if (it->second->line_map.count(CF) == 0) {
        return std::vector<unsigned>();
    }
    return it->second->line_map[CF];
}
