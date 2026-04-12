#include "scope.hpp"
#include <memory>
#include <mutex>
#include <shared_mutex>

Scope::Scope(std::shared_ptr<Scope> enclosing,
             std::unordered_map<std::string, std::shared_ptr<Value>> vars)
    : enclosing(enclosing), vars(vars) {}

Scope::Scope(std::unordered_map<std::string, std::shared_ptr<Value>> vars)
    : Scope(nullptr, vars) {}

Scope::Scope(std::shared_ptr<Scope> enclosing)
    : Scope(enclosing,
            std::unordered_map<std::string, std::shared_ptr<Value>>()) {}

Scope::Scope() : Scope(nullptr) {}

std::shared_ptr<Value> Scope::alloc(std::string key, Value val) {
  std::unique_lock<std::shared_mutex> lock(mutex);

  auto ptr = std::make_shared<Value>(val);
  vars[key] = ptr;

  return ptr;
}

std::shared_ptr<Value> Scope::get(std::string key) {
  std::shared_lock<std::shared_mutex> lock(mutex);
  auto it = vars.find(key);
  lock.unlock(); // Unlock early since we're done reading from vars

  if (it == vars.end()) {
    // Not found here
    if (enclosing)
      return enclosing->get(key);
    else
      return nullptr;
  }

  return it->second;
}

int Scope::getDepth() const {
  if (enclosing)
    return enclosing->getDepth() + 1;
  return 1;
}