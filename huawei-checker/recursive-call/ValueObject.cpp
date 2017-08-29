#include "ValueObject.h"

#include <sstream>

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
  std::stringstream ss(s);
  std::string item;
  while (std::getline(ss, item, delim)) {
    elems.push_back(item);
  }
  return elems;
} 

std::vector<std::string> split(const std::string &s, char delim) {
  std::vector<std::string> elems; 
  split(s, delim, elems);
  return elems;
}

std::string replaceStrChar(std::string str, char from, char to) {
    for (int i = 0; i < str.length(); ++i) {
        if (str[i] == from) {
            str[i] = to; 
        } 
    }
    return str;
}

