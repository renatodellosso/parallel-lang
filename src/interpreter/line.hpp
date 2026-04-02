#pragma once

#include "../instruction.hpp"
#include <string>
#include <vector>
#include <variant>

enum ArgType
{
  String,
  Integer,
  Bool
};

struct Arg
{
  ArgType type;
  std::variant<std::string, int, bool> val;
};

struct Line
{
  int lineNumber;
  bool endsLine;
  InstructionType type;
  std::vector<Arg> args;
};