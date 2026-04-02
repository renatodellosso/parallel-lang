#pragma once

#include "../exitCode.hpp"
#include "../cliUtils.hpp"
#include "../instruction.hpp"
#include <vector>
#include <stack>

class Executor
{
  std::vector<Instruction> &instructions;
  std::stack<Value> stack;

  // Pops and returns the top of the stack
  Value pop();
  void execSingleInstruction(Instruction instr);

public:
  Executor(std::vector<Instruction> &instructions);
  void execInstructions();
};