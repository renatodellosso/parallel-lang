#include "graphLinker.hpp"
#include "expression.hpp"
#include "resource.hpp"
#include <algorithm>
#include <format>
#include <functional>
#include <iterator>
#include <memory>
#include <optional>
#include <vector>

static void deduplicateDependenciesForSet(BinaryExpression &set,
                                          RootExpression &identifier) {
  // Deduplicate dependency
  for (auto it = set.dependencies.begin(); it != set.dependencies.end(); it++) {
    if (&it->get() != &identifier)
      continue;

    // Remove duplicate dependent
    for (auto dIt = it->get().dependents.begin();
         dIt != it->get().dependents.end(); dIt++) {
      if (&dIt->expr.get() == &set && dIt->argIndex == std::nullopt) {
        it->get().dependents.erase(dIt);
        break;
      }
    }

    set.dependencies.erase(it);

    break;
  }
}

GraphLinker::GraphLinker(
    std::shared_ptr<std::vector<std::shared_ptr<Expression>>> exprVector)
    : errors(std::make_shared<std::vector<SyntaxError>>(
          std::vector<SyntaxError>())),
      scope(std::make_shared<Scope<Resource>>()), scopeLifetimes() {
  expressions = std::vector<std::reference_wrapper<Expression>>();
  for (auto expr : *exprVector.get()) {
    auto vec = expr->getWithSubExpressions();
    std::move(vec.begin(), vec.end(), std::back_inserter(expressions));
  }

  // Init default resources
  createResource("int");
  createResource("bool");
  createResource("string");
}

Resource &GraphLinker::createResource(std::string name) {
  Resource resource(name);

  if (scope->getVarTable().contains(resource.name))
    throw std::runtime_error(std::format(
        "Tried to declare resource '{}', but it already existed in this scope!",
        name));

  auto inserted = scope->alloc(resource.name, resource);
  return *inserted.get();
}

void GraphLinker::createResource(Expression &expr) {
  try {
    BinaryExpression &binary = dynamic_cast<BinaryExpression &>(expr);
    Expression *type = binary.left.get();
    RootExpression *name = dynamic_cast<RootExpression *>(binary.right.get());

    auto &resource = createResource(name->token.raw);
    resource.lastWrittenBy = &expr;
    resource.currAccesses.push_back(expr);
  } catch (const std::bad_cast &err) {
    throw std::runtime_error(
        std::format("Attempted to create resource, but expression was not a "
                    "binary expression! Expression: {}",
                    expr.toString()));
  }
}

void GraphLinker::useResource(Expression &expr, std::string name, bool write) {
  auto entry = scope->get(name);
  if (!entry)
    throw std::runtime_error(
        std::format("Expression attempted to use resource '{}', which does not "
                    "exist! Expression: {}",
                    name, expr.toString()));

  Resource &resource = *entry.get();

  // If not written yet, don't add dependency to nullptr
  // If we're writing to this resource, the last write is in currAccesses, so we
  // add the dependency there
  if (!write && resource.lastWrittenBy) {
    resource.lastWrittenBy->dependents.push_back(ExprDependent(expr));
    expr.dependencies.push_back(*resource.lastWrittenBy);
  }

  if (write) {
    resource.lastWrittenBy = &expr;

    // Add a dependency to everything in the access set
    for (auto dep : resource.currAccesses)
      dep.get().dependents.push_back(ExprDependent(expr));
    std::move(resource.currAccesses.begin(), resource.currAccesses.end(),
              std::back_inserter(expr.dependencies));

    // Clear accesses
    resource.currAccesses = std::vector<std::reference_wrapper<Expression>>();
  }

  resource.currAccesses.push_back(expr);
}

void GraphLinker::processExpression(Expression &expr) {
  if (scopeLifetimes.size()) {
    scopeLifetimes.top()--; // int& so this works

    if (scopeLifetimes.top() == -1) {
      scopeLifetimes.pop();
      scope = scope->getEnclosing();
    }
  }

  try {
    if (expr.type == InstructionType::Declare)
      createResource(expr);
    else if (expr.type == InstructionType::GetIdentifier) {
      try {
        RootExpression &root = dynamic_cast<RootExpression &>(expr);
        useResource(expr, root.token.raw, false);
      } catch (std::bad_cast err) {
        syntaxError(
            expr.lineNumber,
            std::format(
                "GetIdentifier expression was not a RootExpression! Expr: {}",
                expr.toString()));
      }
    } else if (expr.type == InstructionType::Set) {
      try {
        BinaryExpression &set = dynamic_cast<BinaryExpression &>(expr);

        RootExpression *identifier;
        if (set.left->type == InstructionType::Declare) {
          BinaryExpression &declare =
              dynamic_cast<BinaryExpression &>(*set.left.get());
          identifier = dynamic_cast<RootExpression *>(declare.left.get());
        } else
          identifier = dynamic_cast<RootExpression *>(set.left.get());

        useResource(set, identifier->token.raw, true);
        deduplicateDependenciesForSet(set, *identifier);
      } catch (const std::bad_cast &err) {
        throw std::runtime_error(
            std::format("Attempted to write resource, but expression was not a "
                        "binary expression! Expression: {}",
                        expr.toString()));
      }
    } else if (expr.type == InstructionType::If) {
      auto next = expressions[expr.id + 1];
      next.get().dependencies.push_back(expr);
      expr.dependents.emplace_back(next.get());
    } else if (expr.type == InstructionType::Block) {
      BlockExpression &block = *static_cast<BlockExpression *>(&expr);
      int size =
          block.countInstructions() - 1; // -1 to exclude the block itself

      // Push new scope
      scopeLifetimes.push(size);
      scope = std::make_shared<Scope<Resource>>(scope);

      // Don't add dependencies to things inside of a nested blocks
      // Deps only need to go out one layer

      int skip = 0;
      for (int i = 0; i < size; i++) {
        if (skip > 0) {
          skip--;
          continue;
        }

        // Handle nested blocks
        auto inner = expressions[expr.id + i + 1];
        if (inner.get().type == InstructionType::Block) {
          skip = static_cast<BlockExpression *>(&expr)->expressions.size();
        }

        // Only add dependencies to root-level expressions and blocks
        // All other exprs will have intra-block dependencies that already
        // depend on the block instruction
        if (inner.get().type != InstructionType::Block &&
            dynamic_cast<RootExpression *>(&inner.get()) == nullptr)
          continue;

        // Add dependency
        inner.get().dependencies.push_back(expr);
        expr.dependents.emplace_back(inner.get());
      }
    }
  } catch (std::runtime_error err) {
    // Careful with runtime_error vs runtime_error* - catching one won't catch
    // the other!
    syntaxError(expr.lineNumber, err.what());
  }
}

void GraphLinker::syntaxError(int line, std::string msg) {
  errors.get()->push_back({line, msg});
}

void GraphLinker::linkGraph() {
  for (auto expr : expressions) {
    processExpression(expr);
    expr.get().linkInternally();
  }
}

std::shared_ptr<std::vector<SyntaxError>> GraphLinker::getErrors() {
  return errors;
}

std::unordered_map<std::string, std::shared_ptr<Resource>> &
GraphLinker::getResources() {
  return scope->getVarTable();
}
