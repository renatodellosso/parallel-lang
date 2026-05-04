#pragma once

#include "../scope.hpp"
#include "expression.hpp"
#include <memory>
#include <optional>
#include <string>
#include <vector>

struct Resource {
  std::string name;
  Expression *lastWrittenBy;
  // The current set of expressions that access this value after the most recent
  // write (including the write)
  std::vector<std::reference_wrapper<Expression>> currAccesses;

  std::optional<std::reference_wrapper<FunctionExpression>> function;

  Resource(std::string name);
  Resource(std::string name,
           std::reference_wrapper<FunctionExpression> function);
};

// Clones the scope, but reinitializes all vars in the clone
std::shared_ptr<Scope<Resource>>
cloneResourceScope(std::shared_ptr<Scope<Resource>> scope);