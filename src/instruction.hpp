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

struct Value
{
  ArgType type;
  std::variant<std::string, int, bool> val;
};

std::string valToStr(Value val);
bool valToBool(Value val);

struct Instruction
{
  int instructionNumber;
  bool endsLine;
  InstructionType type;
  std::vector<Value> args;
};