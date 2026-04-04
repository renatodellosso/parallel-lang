#pragma once

#include <string>
#include "expression.hpp"

struct Resource
{
  std::string name;
  Expression &lastWrittenBy;
};