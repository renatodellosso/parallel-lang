#pragma once

#include "expression.hpp"
#include "resource.hpp"
#include "syntaxError.hpp"
#include <functional>
#include <memory>
#include <optional>
#include <stack>
#include <string>
#include <unordered_map>

class GraphLinker {
  std::shared_ptr<std::vector<SyntaxError>> errors;
  // std::unordered_map<std::string, Resource> resources;
  std::shared_ptr<Scope<Resource>> scope;
  std::vector<std::reference_wrapper<Expression>> expressions;
  std::stack<int> scopeLifetimes;

  std::optional<std::reference_wrapper<FunctionExpression>> function;
  std::stack<std::unique_ptr<int>> funcExprsRemaining;

  Resource &createResource(std::string name);

  void createResource(Expression &expr);
  void useResource(Expression &expr, std::string name, bool write);
  void processExpression(Expression &expr);

  void enterFunction(std::reference_wrapper<Expression> expr);
  void exitFunction();

  void syntaxError(int line, std::string msg);

public:
  GraphLinker(
      std::shared_ptr<std::vector<std::shared_ptr<Expression>>> exprVector);
  void linkGraph();
  std::shared_ptr<std::vector<SyntaxError>> getErrors();
  std::unordered_map<std::string, std::shared_ptr<Resource>> &getResources();
};