#pragma once

#include "../instruction.hpp"
#include <memory>
#include <unordered_map>

class Scope {
  // May be nullptr!
  std::shared_ptr<Scope> enclosing;
  std::unordered_map<std::string, std::shared_ptr<Value>> vars;

public:
  Scope(std::shared_ptr<Scope> enclosing,
        std::unordered_map<std::string, std::shared_ptr<Value>> vars);
  Scope(std::unordered_map<std::string, std::shared_ptr<Value>> vars);
  Scope(std::shared_ptr<Scope> enclosing);
  Scope();

  std::shared_ptr<Value> alloc(std::string key, Value val);
  std::shared_ptr<Value> get(std::string key);
};