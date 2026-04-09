#include "graphLinker.hpp"
#include <format>
#include <iostream>

GraphLinker::GraphLinker(std::shared_ptr<BlockExpression> root)
    : root(root), errors(std::make_shared<std::vector<SyntaxError>>(
                      std::vector<SyntaxError>())),
      resources(std::unordered_map<std::string, Resource>()) {
  // Init default resources
  createResource("int");
  createResource("bool");
  createResource("string");
}

Resource &GraphLinker::createResource(std::string name) {
  Resource resource(name);

  auto [iterator, inserted] =
      resources.insert(std::make_pair(resource.name, resource));
  if (!inserted)
    throw new std::runtime_error(std::format(
        "Tried to declare resource '{}', but it already existed!", name));
  return iterator->second;
}

void GraphLinker::createResource(Expression &expr) {
  try {
    BinaryExpression &binary = dynamic_cast<BinaryExpression &>(expr);
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
  auto entry = resources.find(name);
  if (entry == resources.end())
    throw std::runtime_error(
        std::format("Expression attempted to use resource '{}', which does not "
                    "exist! Expression: {}",
                    name, expr.toString()));

  Resource &resource = entry->second;

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
        RootExpression &identifier =
            dynamic_cast<RootExpression &>(*set.left.get());
        useResource(set, identifier.token.raw, true);
      } catch (const std::bad_cast &err) {
        throw std::runtime_error(
            std::format("Attempted to write resource, but expression was not a "
                        "binary expression! Expression: {}",
                        expr.toString()));
      }
    }
  } catch (std::runtime_error err) {
    syntaxError(expr.lineNumber, err.what());
  }
}

void GraphLinker::syntaxError(int line, std::string msg) {
  errors.get()->push_back({line, msg});
}

void GraphLinker::linkGraph() {
  auto expressions = root.get()->getWithSubExpressions();
  for (auto expr : expressions) {
    processExpression(expr);
    expr.get().linkInternally();
  }
}

std::shared_ptr<std::vector<SyntaxError>> GraphLinker::getErrors() {
  return errors;
}

std::unordered_map<std::string, Resource> &GraphLinker::getResources() {
  return resources;
}
