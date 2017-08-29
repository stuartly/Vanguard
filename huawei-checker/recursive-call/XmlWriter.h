#ifndef _XML_WRITER_H
#define _XML_WRITER_H

#include <string>
#include <vector>

#include "ValueObject.h"

class XmlWriter {
public:
  void writeToXml(std::string file, std::vector<RecursiveCall> recursiveCalls, std::vector<StronglyConnectedComponent> stronglyConnectedComponents);
  void writeToConsole(std::vector<RecursiveCall> recursiveCalls);
};

#endif
