
#include "framework/Logger.h"

#include "llvm/Support/raw_ostream.h"

TLogLevel Logger::configurationLevel = LOG_DEBUG_3;
TCheck Logger::checkType = TAINT_CHECK;
bool Logger::options[6] = {false, false, false, false, false, false};

void Logger::configure(Config &c) {
    auto block = c["PrintLog"];
    int level = block["level"].asInt();
    switch (level) {
        case 1: configurationLevel = LOG_DEBUG_1; break;
        case 2: configurationLevel = LOG_DEBUG_2; break; 
        case 3: configurationLevel = LOG_DEBUG_3; break;
        case 4: configurationLevel = LOG_DEBUG_4; break; 
        case 5: configurationLevel = LOG_DEBUG_5; break; 
        default: configurationLevel = LOG_DEBUG_3;
    }

    if(block["taintChecker"].asBool())
        options[TAINT_CHECK] = true;
    if(block["danglingPointer"].asBool())
        options[DANGLING_POINTER] = true;
    if(block["arrayBound"].asBool())
        options[ARRAY_BOUND] = true;
    if(block["recursiveCall"].asBool())
        options[RECURSIVE_CALL] = true;
    if(block["divideChecker"].asBool())
        options[DIVIDE_CHECK] = true;
    if(block["memoryOPChecker"].asBool())
        options[MEMORY_OP] = true;
}


void common::printLog(std::string logString, common::CheckerName cn, int level, Config &c) {

    auto block = c["PrintLog"];
    int l = block["level"].asInt();
    switch(cn) {
        case common::CheckerName::taintChecker: 
            if(block["taintChecker"].asBool() && level>=l) {
                llvm::errs()<<logString;
            }
            break;
        case common::CheckerName::danglingPointer: 
            if(block["danglingPointer"].asBool() && level>=l) {
                llvm::errs()<<logString;
            }
            break;
        case common::CheckerName::arrayBound: 
            if(block["arrayBound"].asBool() && level>=l) {
                llvm::errs()<<logString;
            }
            break;
        case common::CheckerName::recursiveCall: 
            if(block["recursiveCall"].asBool() && level>=l) {
                llvm::errs()<<logString;
            }
            break;
        case common::CheckerName::divideChecker: 
            if(block["divideChecker"].asBool() && level>=l) {
                llvm::errs()<<logString;
            }
            break;
        case common::CheckerName::memoryOPChecker: 
            if(block["memoryOPChecker"].asBool() && level>=l) {
                llvm::errs()<<logString;
            }
            break;
    }
}

