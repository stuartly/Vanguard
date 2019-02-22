
set(LLVM_LINK_COMPONENTS support)
set(LLVM_USED_LIBS clangTooling clangBasic clangAST)
set(CMAKE_EXE_LINKER_FLAGS "-static")

set_source_files_properties(library/json/jsoncpp.cpp PROPERTIES COMPILE_FLAGS -fexceptions)

add_clang_executable(huawei-checker
  main.cpp

  framework/Common.cpp
  framework/Logger.cpp
  framework/CallGraph.cpp
  framework/BasicChecker.cpp
  
  framework/ASTElement.cpp
  framework/ASTQueue.cpp
  framework/ASTManager.cpp

  taint-check/TaintValue.cpp
  taint-check/TaintEnv.cpp
  taint-check/FunctionEnv.cpp
  taint-check/TaintChecker.cpp
  taint-check/TaintInterface.cpp
  
  arraybound/ArrayOutOfBoundsCheckerV4.cpp
  dangling-pointer/DanglingPointer.cpp
  taint-memory-op/TaintedMemOPChecker.cpp
  taint-check-div/DivChecker.cpp

  recursive-call/Digraph.cpp
  recursive-call/ElementaryCyclesSearch.cpp
  recursive-call/RecursiveCallChecker.cpp
  recursive-call/TarjanSCC.cpp
  recursive-call/ValueObject.cpp
  recursive-call/XmlWriter.cpp

  library/json/jsoncpp.cpp
  library/xml/pugixml/pugixml.cpp
  )
include_directories(
  library/json
  library/xml
  )
target_link_libraries(huawei-checker
  clangTooling
  clangBasic
  clangASTMatchers
  clangFrontend
  LLVMTarget
  LLVMX86AsmParser
  )
