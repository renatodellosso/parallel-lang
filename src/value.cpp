#include "value.hpp"
#include <format>

std::string valToStr(Value val) {
  switch (val.type) {
  case ValueType::Integer:
    return std::to_string(std::get<int>(val.val));
  case ValueType::Bool:
    return std::to_string(std::get<bool>(val.val));
  case ValueType::String:
    return std::format("{}", std::get<std::string>(val.val));
  case ValueType::Identifier:
    return std::format("<identifier {}>", std::get<std::string>(val.val));

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
  return false;
}