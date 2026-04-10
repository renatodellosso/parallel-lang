#include "scope.hpp"
#include <memory>

Scope::Scope(std::shared_ptr<Scope> enclosing,
             std::unordered_map<std::string, std::shared_ptr<Value>> vars)
    : enclosing(enclosing), vars(vars) {}

Scope::Scope(std::unordered_map<std::string, std::shared_ptr<Value>> vars)
    : Scope(nullptr, vars) {}

Scope::Scope(std::shared_ptr<Scope> enclosing)
    : Scope(enclosing,
            std::unordered_map<std::string, std::shared_ptr<Value>>()) {}

Scope::Scope() : Scope(nullptr) {}

std::shared_ptr<Value> Scope::get(std::string key) {
  auto it = vars.find(key);
  if (it == vars.end()) {
    // Not found here
    if (enclosing)
      return enclosing->get(key);
    else
      return nullptr;
  }

  return it->second;
}