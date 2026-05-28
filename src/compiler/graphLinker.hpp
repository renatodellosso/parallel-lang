#pragma once

#include "../cliUtils.hpp"
// Ignore the warning, we need it for the set
#include "deferredFunctionLinking.hpp"
#include "expression.hpp"
#include "resource.hpp"
#include "syntaxError.hpp"
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <stack>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class GraphLinker {
  CliArgs cliArgs;

  std::shared_ptr<std::vector<SyntaxError>> errors;
  std::shared_ptr<Scope<Resource>> scope;
  // We need an ordered map
  std::map<int, std::reference_wrapper<Expression>> expressions;
  std::stack<int> scopeLifetimes;

  std::optional<std::reference_wrapper<FunctionExpression>> function;
  std::stack<std::reference_wrapper<FunctionExpression>> funcStack;
  std::stack<std::unique_ptr<int>> funcExprsRemaining;
  std::stack<std::shared_ptr<Scope<Resource>>> savedScopes;

  std::unordered_set<std::reference_wrapper<FunctionExpression>>
      deferredFunctionLinkings;
  bool processingDeferred;

  struct ResourceState {
    std::shared_ptr<Resource> resource;
    Expression *lastWrittenBy;
    std::vector<std::reference_wrapper<Expression>> currAccesses;
  };

  struct BranchContext {
    int thenStart, thenEnd;
    std::optional<int> elseInstructionId;
    int elseStart, elseEnd;
    int mergeId;
    std::unordered_map<std::string, ResourceState> snapshot;
    std::unordered_set<std::string> touchedResources;
    std::unordered_set<std::string> writtenResources;
  };

  std::vector<BranchContext> branchContexts;
  std::unordered_map<int, int> branchByElseInstructionId;
  std::unordered_map<int, int> branchByMergeId;

  Resource &createResource(std::string name);

  void createResource(Expression &expr);
  // Uses the resource. If there is a current function, will add a write there,
  // too.
  void useResource(Expression &expr, std::string name, bool write);
  std::unordered_map<std::string, ResourceState> captureResourceSnapshot();
  void restoreResourceSnapshot(
      const std::unordered_map<std::string, ResourceState> &snapshot);
  void markBranchResourceUse(Expression &expr, std::string name, bool write);
  void registerIfExpression(IfExpression &expr);
  void enterElseBranch(Expression &expr);
  void finalizeBranchMerge(Expression &expr);
  void updateScopeLifetimes();
  void processExpression(Expression &expr);

  void enterFunction(std::reference_wrapper<Expression> expr);
  void exitFunction();

  void syntaxError(int line, std::string msg);

  void linkIteration(std::reference_wrapper<Expression> expr);

public:
  GraphLinker(
      const CliArgs &cliArgs,
      std::shared_ptr<std::vector<std::shared_ptr<Expression>>> exprVector);
  GraphLinker(
      std::shared_ptr<std::vector<std::shared_ptr<Expression>>> exprVector);
  void linkDeferred();
  void linkGraph();
  std::shared_ptr<std::vector<SyntaxError>> getErrors();
  std::unordered_map<std::string, std::shared_ptr<Resource>> &getResources();
};
