#include "utils.hpp"

bool beginsWith(std::string str, std::string prefix)
{
  return std::strncmp(str.c_str(), prefix.c_str(), prefix.length()) == 0;
}

bool isInteger(const std::string &s)
{
  return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
}