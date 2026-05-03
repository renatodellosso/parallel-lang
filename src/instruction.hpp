#pragma once

#include "scope.hpp"
#include "value.hpp"
#include <memory>
#include <optional>
#include <string>
#include <vector>

enum class InstructionType {
  Block,
  GetLiteral,
  ReferenceIdentifier,
  GetIdentifier,
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
  CompareGreaterThanEquals,
  If,
  While,
  GoTo,
  Print,
  Function,
  Call
};

std::string instructionTypeToString(InstructionType type);

struct Instruction;

struct InstrDependent {
  Instruction *instr;
  std::optional<int> argIndex;

  InstrDependent(Instruction *instr, std::optional<int> argIndex);
  InstrDependent(Instruction *instr, int argIndex);
  InstrDependent(Instruction *instr);
};

struct Instruction {
  int id;

  InstructionType type;

  // Args inherent to the instruction
  std::vector<Value> bytecodeArgs;
  // Args from previous instructions
  std::vector<std::shared_ptr<Value>> depArgs;

  int depCount, depsFulfilled;
  std::vector<InstrDependent> dependents;

  std::shared_ptr<Scope<Value>> scope;

  Instruction(int id, std::shared_ptr<Scope<Value>> scope = nullptr);

  std::string toString();
};