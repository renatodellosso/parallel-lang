#pragma once

#include <optional>
#include <string>
#include <vector>
#include <variant>

enum class InstructionType
{
  Block,
  GetLiteral,
  GetIdentifier,
  ReferenceIdentifier,
  Declare,
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

std::string instructionTypeToString(InstructionType type);

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

struct Instruction;

struct InstrDependent
{
  int instrId;
  std::optional<int> argIndex;

  InstrDependent(int instrId, std::optional<int> argIndex);
  InstrDependent(int instrId, int argIndex);
  InstrDependent(int instrId);
};

struct Instruction
{
  int id;
  bool endsLine;

  InstructionType type;

  // Args inherent to the instruction
  std::vector<Value> bytecodeArgs;
  // Args from previous instructions
  std::vector<Value> depArgs;

  int depCount, depsFulfilled;
  std::vector<InstrDependent> dependents;

  Instruction(int id);

  std::string toString();
};