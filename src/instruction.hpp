#pragma once

#include "interpreter/scope.hpp"
#include "value.hpp"
#include <memory>
#include <optional>
#include <string>
#include <vector>

enum class InstructionType {
  Block,
  GetLiteral,
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
  CompareGreaterThanEquals
};

std::string instructionTypeToString(InstructionType type);

struct Instruction;

struct InstrDependent {
  Instruction* instr;
  std::optional<int> argIndex;

  InstrDependent(Instruction *instr, std::optional<int> argIndex);
  InstrDependent(Instruction *instr, int argIndex);
  InstrDependent(Instruction *instr);
};

struct Instruction {
  int id;
  bool endsLine;

  InstructionType type;

  bool executed;

  // Args inherent to the instruction
  std::vector<Value> bytecodeArgs;
  // Args from previous instructions
  std::vector<std::shared_ptr<Value>> depArgs;

  int depCount, depsFulfilled;
  std::vector<InstrDependent> dependents;

  std::shared_ptr<Scope> scope;

  Instruction(int id, std::shared_ptr<Scope> scope = nullptr);

  std::string toString();
};