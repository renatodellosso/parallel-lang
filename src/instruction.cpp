#include "instruction.hpp"
#include <format>

std::string valToStr(Value val)
{
  switch (val.type)
  {
  case ArgType::Integer:
    return std::to_string(std::get<int>(val.val));
  case ArgType::Bool:
    return std::to_string(std::get<bool>(val.val));
  case ArgType::String:
    return std::format("\"{}\"", std::get<std::string>(val.val));

  default:
    return std::format("<unknown type: {}>", (int)val.type);
  }
}