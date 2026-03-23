#include "utils.hpp"

bool beginsWith(std::string str, std::string prefix)
{
  return std::strncmp(str.c_str(), prefix.c_str(), prefix.length()) == 0;
}