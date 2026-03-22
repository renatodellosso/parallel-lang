#include "utils.hpp"

bool beginsWith(std::string str, std::string prefix)
{
  return strncmp(str.c_str(), prefix.c_str(), prefix.length()) == 0;
}