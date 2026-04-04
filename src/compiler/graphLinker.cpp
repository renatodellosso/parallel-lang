#include "graphLinker.hpp"
#include <format>

GraphLinker::GraphLinker(std::shared_ptr<BlockExpression> root) : root(root),
                                                                  errors(std::make_shared<std::vector<SyntaxError>>(std::vector<SyntaxError>())),
                                                                  resources(std::unordered_map<std::string, Resource>())
{
}

void GraphLinker::createResource(Expression &expr)
{
  try
  {
    BinaryExpression &binary = dynamic_cast<BinaryExpression &>(expr);
    RootExpression *name = dynamic_cast<RootExpression *>(binary.right.get());

    Resource resource = {
        .name = name->token.raw,
        .lastWrittenBy = expr};

    resources.insert(std::make_pair(resource.name, resource));
  }
  catch (const std::bad_cast &err)
  {
    throw std::runtime_error(std::format("Attempted to create resource, but expression was not a binary expression! Expression: {}", expr.toString()));
  }
}

void GraphLinker::useResource(Expression &expr, bool write)
{
  try
  {
    RootExpression &root = dynamic_cast<RootExpression &>(expr);

    auto entry = resources.find(root.token.raw);
    if (entry == resources.end())
      throw std::runtime_error(std::format("Expression attempted to use resource '{}', which does not exist! Expression: {}", root.token.raw, root.toString()));

    Resource &resource = entry->second;
    resource.lastWrittenBy.dependents.push_back(expr);
    expr.dependencies.push_back(resource.lastWrittenBy);

    if (write)
      resource.lastWrittenBy = expr;
  }
  catch (const std::bad_cast &err)
  {
    throw std::runtime_error(std::format("Attempted to use resource, but expression was not a root expression! Expression: {}", expr.toString()));
  }
}

void GraphLinker::processExpression(Expression &expr)
{
  try
  {
    if (expr.type == InstructionType::Declare)
      createResource(expr);
    if (expr.type == InstructionType::GetIdentifier)
      useResource(expr, false);
    if (expr.type == InstructionType::Set)
    {
      try
      {
        BinaryExpression &binary = dynamic_cast<BinaryExpression &>(expr);
        RootExpression &identifier = dynamic_cast<RootExpression &>(*binary.left.get());
        useResource(identifier, true);
      }
      catch (const std::bad_cast &err)
      {
        throw std::runtime_error(std::format("Attempted to create resource, but expression was not a binary expression! Expression: {}", expr.toString()));
      }
    }
  }
  catch (std::runtime_error err)
  {
    syntaxError(expr.lineNumber, err.what());
  }
}

void GraphLinker::syntaxError(int line, std::string msg)
{
  errors.get()->push_back({line, msg});
}

void GraphLinker::linkGraph()
{
  auto expressions = root.get()->getWithSubExpressions();
  for (auto expr : expressions)
  {
    processExpression(expr);
  }
}

std::shared_ptr<std::vector<SyntaxError>> GraphLinker::getErrors()
{
  return errors;
}
