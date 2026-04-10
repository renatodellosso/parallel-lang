#pragma once

#include <string>
#include <variant>

enum ValueType { String, Integer, Bool };

struct Value {
  ValueType type;
  std::variant<std::string, int, bool> val;
};

std::string valToStr(Value val);
bool valToBool(Value val);