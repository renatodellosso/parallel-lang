#include "graphLinker.hpp"

GraphLinker::GraphLinker(std::shared_ptr<BlockExpression> root) : root(root),
                                                                  errors(std::make_shared<std::vector<SyntaxError>>(std::vector<SyntaxError>())),
                                                                  resources(std::unordered_map<std::string, Resource>())
{
}

void GraphLinker::syntaxError(int line, std::string msg)
{
  errors.get()->push_back({line, msg});
}

void GraphLinker::linkGraph()
{
}

std::shared_ptr<std::vector<SyntaxError>> GraphLinker::getErrors()
{
  return errors;
}
