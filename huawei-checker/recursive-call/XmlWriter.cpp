#include "XmlWriter.h"

#include <iostream>
#include <fstream>
#include <vector>

#include "ValueObject.h"

void XmlWriter::writeToXml(std::string file, std::vector<RecursiveCall> recursiveCalls, std::vector<StronglyConnectedComponent> stronglyConnectedComponents) {

    if (recursiveCalls.empty() && stronglyConnectedComponents.empty()) {
        return; 
    }
    std::string checkerId = "MOLINT.RECURSIVE.CALL";
    std::ofstream ofs;
    ofs.open(file);

    for (RecursiveCall recursiveCall : recursiveCalls) {
        ofs << "<error>\n";
        ofs << "\t<checker>" << checkerId << "</checker>\n";
        ofs << "\t<domain>STATIC_C</domain>\n";
        ofs << "\t<file>" << recursiveCall.functions.at(0).file << "</file>\n";
        ofs << "\t<function>" << recursiveCall.functions.at(0).name << "</function>\n";
        ofs << "\t<score>100</score>\n";
        ofs << "\t<ordered>false</ordered>\n";
        ofs << "\t<event>\n";
        ofs << "\t\t<main>true</main>\n";
        ofs << "\t\t<tag>Error</tag>\n";
        ofs << "\t\t<description>\n";
        ofs << "\t\t\tFind a recursive call involving the folowing functions:\n";
        for (Function function : recursiveCall.functions) {
            ofs << "\t\t\t\t" << "Function:[ "<< function.name << " ] defined at location [ " << function.location << " ]\n";
        }
        ofs << "\t\t\tThe call relationship are as follows:\n";
        for (FunctionCall functionCall : recursiveCall.functionCalls) {
            ofs << "\t\t\t\t" << "Function:[ " << functionCall.caller.name << " ] calls [ " << functionCall.callee.name << " ] at location [ " << functionCall.getLocation() << " ]\n";
        }
        ofs << "\t\t</description>\n";
        ofs << "\t\t<line>" << recursiveCall.functions.at(0).line << "</line>\n";
        ofs << "\t</event>\n";
        ofs << "\t<extra>none</extra>\n";
        ofs << "\t<subcategory>none</subcategory>\n";
        ofs << "</error>\n\n";
    }

    for (StronglyConnectedComponent stronglyConnectedComponent : stronglyConnectedComponents) {
        ofs << "<error>\n";
        ofs << "\t<checker>" << checkerId << "</checker>\n";
        ofs << "\t<domain>STATIC_C</domain>\n";
        ofs << "\t<file>" << stronglyConnectedComponent.functions.at(0).file << "</file>\n";
        ofs << "\t<function>" << stronglyConnectedComponent.functions.at(0).name << "</function>\n";
        ofs << "\t<score>100</score>\n";
        ofs << "\t<ordered>false</ordered>\n";
        ofs << "\t<event>\n";
        ofs << "\t\t<main>true</main>\n";
        ofs << "\t\t<tag>Error</tag>\n";
        ofs << "\t\t<description>\n";
        ofs << "\t\t\tFind recursive call set involving " << stronglyConnectedComponent.size() << " functions.\n";
        ofs << "\t\t\tIt may take too much time to find all the recursive call in it, you can rerun the checker with option 'max_scc_size=[some value larger than the functions size]' in configuration file to find all recursive calls.\n";
        ofs << "\t\t\tJust list the involved functions as follows:\n";
        for (Function function : stronglyConnectedComponent.functions) {
            ofs << "\t\t\t\t" << "Function:[ "<< function.name << " ] defined at location [ " << function.location << " ]\n";
        }
        ofs << "\t\t\tThe call relationship are as follows:\n";
        for (FunctionCall functionCall : stronglyConnectedComponent.functionCalls) {
            ofs << "\t\t\t\t" << "Function:[ " << functionCall.caller.name << " ] calls [ " << functionCall.callee.name << " ] at location [ " << functionCall.getLocation()<< " ]\n";
        }
        ofs << "\t\t</description>\n";
        ofs << "\t\t<line>" << stronglyConnectedComponent.functions.at(0).line << "</line>\n";
        ofs << "\t</event>\n";
        ofs << "\t<extra>none</extra>\n";
        ofs << "\t<subcategory>none</subcategory>\n";
        ofs << "</error>\n\n";

    }

    ofs.close();
}
