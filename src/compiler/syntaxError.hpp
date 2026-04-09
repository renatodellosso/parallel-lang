#pragma once

#include <string>

struct SyntaxError {
  int line;
  std::string msg;

  std::string toString() const;
};