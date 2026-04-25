#pragma once

#include "../scope.hpp"
#include "expression.hpp"
#include <memory>
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

// Clones the scope, but reinitializes all vars in the clone
std::shared_ptr<Scope<Resource>> cloneResourceScope(std::shared_ptr<Scope<Resource>> scope);