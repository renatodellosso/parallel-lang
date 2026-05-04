#include "graphLinker.hpp"
#include "expression.hpp"
#include "resource.hpp"
#include <algorithm>
#include <format>
#include <functional>
#include <iterator>
#include <memory>
#include <optional>
#include <stdexcept>
#include <unordered_map>
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

static void addDependency(Expression &expr, Expression &dependsOn) {
  auto redirect = dependsOn.dependentRedirect;
  if (redirect) {
    addDependency(expr, *redirect);
    return;
  }

  dependsOn.dependents.push_back(expr);
  expr.dependencies.push_back(dependsOn);
}

GraphLinker::GraphLinker(
    std::shared_ptr<std::vector<std::shared_ptr<Expression>>> exprVector)
    : errors(std::make_shared<std::vector<SyntaxError>>(
          std::vector<SyntaxError>())),
      scope(std::make_shared<Scope<Resource>>()), scopeLifetimes(),
      function(std::nullopt), funcExprsRemaining() {
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
    addDependency(expr, *resource.lastWrittenBy);
  }

  if (write) {
    resource.lastWrittenBy = &expr;

    // Add a dependency to everything in the access set
    for (auto dep : resource.currAccesses) {
      addDependency(expr, dep);
    }

    if (function.has_value() && !function->get().firstUses.contains(name)) {
      // This is our first write
      function->get().firstUses[name] = resource.currAccesses;
      function->get().firstWrites.emplace(name, expr);
    }

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
    else if (expr.type == InstructionType::GetIdentifier ||
             expr.type == InstructionType::ReferenceIdentifier) {
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
          identifier = dynamic_cast<RootExpression *>(declare.right.get());
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
    } else if (expr.type == InstructionType::If ||
               expr.type == InstructionType::While) {
      auto next = expressions[expr.id + 1];
      addDependency(next, expr); // Add dependency with block
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

        // Only add dependencies to root-level expressions, blocks, and
        // functions All other exprs will have intra-block dependencies that
        // already depend on the block instruction
        if (inner.get().type != InstructionType::Block &&
            inner.get().type != InstructionType::Function &&
            dynamic_cast<RootExpression *>(&inner.get()) == nullptr)
          continue;

        addDependency(inner, expr);

        if (inner.get().type == InstructionType::Function) {
          // Skip that function's block
          auto body = static_cast<BlockExpression *>(
              &expressions[inner.get().id + 1].get());
          skip = body->expressions.size() + 1;
        }
      }
    } else if (expr.type == InstructionType::GoTo) {
      RootExpression &root = static_cast<RootExpression &>(expr);
      int dist = std::atoi(root.token.raw.c_str());
      int returnTo = expr.id + dist;

      // Depend on everything between returnTo and expr
      // This is to allow loop bodies to run before restarting them
      int conditionIndex = -1;
      for (int i = returnTo; i < expr.id; i++) {
        // Skip everything before the if statement
        if (conditionIndex == -1) {
          if (expressions[i].get().type == InstructionType::While) {
            conditionIndex = i;
          }
          continue;
        }

        // Don't a dependency to the block since we'll already have one
        if (i != conditionIndex + 1)
          addDependency(expr, expressions[i]);

        // We've already linked everything in the loop body, so we don't have to
        // worry Nesting multiple levels of redirects
        expressions[i].get().dependentRedirect =
            &expressions[conditionIndex].get();
      }
    } else if (expr.type == InstructionType::Call) {
      CallExpression &call = static_cast<CallExpression &>(expr);
      auto name = call.getFunctionName();

      auto resource = scope->get(name);
      if (!resource)
        throw std::runtime_error(std::format(
            "Tried to call function '{}', but it did not exist!", name));
      if (!resource->function)
        throw std::runtime_error(std::format(
            "Tried to call function '{}', but it was not a function!", name));

      // Maps resource names to write (true/false)
      auto uses = std::unordered_map<std::string, bool>();

      // Set all reads first
      for (auto use : resource->function.value().get().firstUses) {
        uses[use.first] = false;
      }

      // Then all writes
      for (auto use : resource->function.value().get().firstWrites) {
        uses[use.first] = true;
      }

      // Use the resources
      for (auto resource : uses) {
        useResource(call, resource.first, resource.second);
      }
    }
  } catch (std::runtime_error err) {
    // Careful with runtime_error vs runtime_error* - catching one won't catch
    // the other!
    syntaxError(expr.lineNumber, err.what());
  }
}

void GraphLinker::enterFunction(std::reference_wrapper<Expression> expr) {
  // Make a temp variable so we can grab the name easily before setting function
  // (we want to make a resourcew without updating firstWrites)
  auto tempFunc =
      std::make_optional<std::reference_wrapper<FunctionExpression>>(
          static_cast<FunctionExpression &>(expr.get()));

  // Create function resource in outer scope (use auto & instead of just auto so
  // as not to make a copy!)
  auto name = tempFunc->get().name;
  auto &resource = createResource(name);
  useResource(tempFunc->get(), name, true);
  resource.function = tempFunc;

  function = tempFunc;

  int exprCount = expr.get().countInstructions();

  if (!funcExprsRemaining.empty()) { // Can't do top() to check non-emptiness
    // Decrease outer func since the decrement when processing
    // only applies to the current top
    (*funcExprsRemaining.top()) -= exprCount;
  }

  funcExprsRemaining.push(std::move(std::make_unique<int>(exprCount)));

  savedScopes.push(scope);
  scope = cloneResourceScope(scope);
  scopeLifetimes.push(expressions[expr.get().id + 1].get().countInstructions() +
                      1);

  for (auto param : function->get().params)
    createResource(param.name);
}

void GraphLinker::exitFunction() {
  // Populate lastUses and lastWrites
  for (auto key : scope->getKeys()) {
    auto resource = scope->get(key);

    // Don't add params to first/last uses/writes
    bool isParam = false;
    for (auto param : function->get().params) {
      if (param.name == key) {
        isParam = true;
        break;
      }
    }

    if (isParam)
      continue;

    if (resource->lastWrittenBy)
      function->get().lastWrites.emplace(key, *resource->lastWrittenBy);
    if (resource->currAccesses.size())
      function->get().lastUses[key] = resource->currAccesses;

    if (!function->get().firstUses.contains(key)) {
      // This is our first use/write
      if (resource->currAccesses.size())
        function->get().firstUses[key] = resource->currAccesses;
      if (resource->lastWrittenBy)
        function->get().firstWrites.emplace(key, *resource->lastWrittenBy);
    }
  }

  scope = savedScopes.top();
  savedScopes.pop();
  scopeLifetimes.pop();

  function = std::nullopt;
  funcExprsRemaining.pop();
}

void GraphLinker::syntaxError(int line, std::string msg) {
  errors.get()->push_back({line, msg});
}

void GraphLinker::linkGraph() {
  for (auto expr : expressions) {
    if (expr.get().type == InstructionType::Function)
      enterFunction(expr);

    processExpression(expr);
    expr.get().linkInternally();

    if (!funcExprsRemaining.empty()) {
      (*funcExprsRemaining.top().get())--;
      if (*funcExprsRemaining.top() == 0)
        exitFunction();
    }
  }

  // Exit any remaining functions
  while (function.has_value())
    exitFunction();
}

std::shared_ptr<std::vector<SyntaxError>> GraphLinker::getErrors() {
  return errors;
}

std::unordered_map<std::string, std::shared_ptr<Resource>> &
GraphLinker::getResources() {
  return scope->getVarTable();
}
