#pragma once

#include "resource.hpp"
#include "syntaxError.hpp"
#include <string>
#include <unordered_map>

class GraphLinker {
  std::shared_ptr<std::vector<SyntaxError>> errors;
  std::unordered_map<std::string, Resource> resources;
  std::vector<std::reference_wrapper<Expression>> expressions;
  std::shared_ptr<BlockExpression> root;

  Resource &createResource(std::string name);

  void createResource(Expression &expr);
  void useResource(Expression &expr, std::string name, bool write);
  void processExpression(Expression &expr);

  void syntaxError(int line, std::string msg);

public:
  GraphLinker(std::shared_ptr<BlockExpression> root);
  void linkGraph();
  std::shared_ptr<std::vector<SyntaxError>> getErrors();
  std::unordered_map<std::string, Resource> &getResources();
};