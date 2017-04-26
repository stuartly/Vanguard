#ifndef _VALUE_OBJECT_H
#define _VALUE_OBJECT_H

#include "framework/ASTElement.h"

#include <sstream>
#include <string>
#include <vector>
#include <cstddef>

#include "clang/AST/ASTContext.h"

// Helper method to split a string by a character
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
std::vector<std::string> split(const std::string &s, char delim); 

// Value object class representing a function
struct Function {

  std::string name;
  std::string file;
  std::string line;
  std::string column;
  std::string location;

  Function() {}

  Function(ASTFunction* F, std::string location) : location(location) {

    this->name = F->getName(); 
    
    // Location of function is formed as [File]:[Line]:[Column]
    std::size_t colon_pos;
    colon_pos = location.rfind(":");
    std::string s(location);
    this->column = s.substr(colon_pos + 1, location.length() - colon_pos - 1);
    s.erase(colon_pos, s.length() - colon_pos); 
    colon_pos = s.rfind(":");
    this->line = s.substr(colon_pos + 1, location.length() - colon_pos - 1);
    s.erase(colon_pos, s.length() - colon_pos); 
    this->file = s;
  }

};

// Value object class representing a function call
struct FunctionCall {
  Function caller;
  Function callee;
  std::string file; 
  unsigned line;
  std::string getLocation() {
    return caller.file + ":" + std::to_string(line); 
  }
};

// Value object class representing a strongly connected component 
struct StronglyConnectedComponent {

  std::vector<Function> functions;
  std::vector<FunctionCall> functionCalls;

  StronglyConnectedComponent(std::vector<Function> functions, std::vector<FunctionCall> functionCalls) : functions(functions), functionCalls(functionCalls) {}

  int size() {
    return functions.size(); 
  }

};

// Value object class representing a recursive call 
struct RecursiveCall {
  RecursiveCall() {}
  RecursiveCall(std::vector<Function> functions, std::vector<FunctionCall> functionCalls) : functions(functions), functionCalls(functionCalls) {}
  std::vector<Function> functions;
  std::vector<FunctionCall> functionCalls;
};

#endif
