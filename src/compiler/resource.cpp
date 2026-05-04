#include "resource.hpp"
#include <memory>
#include <optional>

Resource::Resource(std::string name)
    : name(name), lastWrittenBy(nullptr),
      currAccesses(std::vector<std::reference_wrapper<Expression>>()),
      function(std::nullopt) {}

Resource::Resource(std::string name,
                   std::reference_wrapper<FunctionExpression> function)
    : Resource(name) {
  this->function = std::make_optional(function);
}

std::shared_ptr<Scope<Resource>>
cloneResourceScope(std::shared_ptr<Scope<Resource>> scope) {
  auto enclosing = scope->getEnclosing()
                       ? cloneResourceScope(scope->getEnclosing())
                       : nullptr;

  std::shared_ptr<Scope<Resource>> clone =
      std::make_shared<Scope<Resource>>(enclosing);

  for (auto entry : scope->getVarTable()) {
    clone->alloc(entry.first, Resource(entry.first));
  }

  return clone;
}