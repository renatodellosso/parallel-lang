#include "value.hpp"
#include "interpreter/function.hpp"
#include <format>
#include <memory>

std::string valToStr(Value val) {
  switch (val.type) {
  case ValueType::Integer:
    return std::to_string(std::get<int>(val.val));
  case ValueType::Bool:
    return std::get<bool>(val.val) ? "true" : "false";
  case ValueType::String:
    return std::format("{}", std::get<std::string>(val.val));
  case ValueType::Identifier:
    return std::format("<identifier {}>", std::get<std::string>(val.val));
  case ValueType::Function: {
    auto func = std::get<std::shared_ptr<Function>>(val.val);
    return std::format("<function {} {}()>", func->getReturnType(),
                       func->getName());
  }

  default:
    return std::format("<unknown type: {}>", (int)val.type);
  }
}

bool valToBool(Value val) {
  if (val.type == ValueType::Bool)
    return std::get<bool>(val.val);
  if (val.type == ValueType::Integer)
    return std::get<int>(val.val) != 0;
  if (val.type == ValueType::String)
    return std::get<std::string>(val.val).length() != 0;
  if (val.type == ValueType::Function)
    return true;
  return false;
}