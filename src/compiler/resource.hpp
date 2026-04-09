#pragma once

#include "expression.hpp"
#include <string>
#include <vector>

struct Resource {
  std::string name;
  Expression *lastWrittenBy;
  // The current set of expressions that access this value after the most recent
  // write (including the write)
  std::vector<std::reference_wrapper<Expression>> currAccesses;

  Resource(std::string name);
};