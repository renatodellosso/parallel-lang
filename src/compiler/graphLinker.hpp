#pragma once

#include "resource.hpp"
#include "syntaxError.hpp"
#include <string>
#include <map>

class GraphLinker
{
  std::shared_ptr<std::vector<SyntaxError>> errors;
  std::unordered_map<std::string, Resource> resources;
  std::shared_ptr<BlockExpression> root;

  void syntaxError(int line, std::string msg);

public:
  GraphLinker(std::shared_ptr<BlockExpression> root);
  void linkGraph();
  std::shared_ptr<std::vector<SyntaxError>> getErrors();
};