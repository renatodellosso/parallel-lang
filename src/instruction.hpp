#pragma once

#include <string>
#include <vector>
#include <variant>
enum class InstructionType
{
  Block,
  GetLiteral,
  GetIdentifier,
  Set,
  Add,
  Subtract,
  Multiply,
  Divide,
  Negate,
  CompareEquals,
  CompareLessThan,
  CompareLessThanEquals,
  CompareGreaterThan,
  CompareGreaterThanEquals
};

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

struct Instruction
{
  int lineNumber;
  bool endsLine;
  InstructionType type;
  std::vector<Arg> args;
};