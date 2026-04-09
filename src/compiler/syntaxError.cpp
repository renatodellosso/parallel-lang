#include "syntaxError.hpp"
#include <format>

std::string SyntaxError::toString() const {
  return std::format("Syntax error at line {}: {}", line, msg);
}