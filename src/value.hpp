#pragma once

#include <memory>
#include <string>
#include <variant>

class Function;

enum class ValueType { String, Integer, Bool, Identifier, Function };

struct Value {
  ValueType type;
  std::variant<std::string, int, bool, std::shared_ptr<Function>>
      val; // Can't just do Function since forward declarations cause issues
           // with it
};

std::string valToStr(Value val);
bool valToBool(Value val);