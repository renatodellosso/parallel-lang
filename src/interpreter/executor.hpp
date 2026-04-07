#pragma once

#include "../exitCode.hpp"
#include "../cliUtils.hpp"
#include "../instruction.hpp"
#include <vector>
#include <stack>

class Executor
{
  std::vector<Instruction> &instructions;

  void pushResult(Instruction instr, Value result);
  void execSingleInstruction(Instruction instr);

public:
  Executor(std::vector<Instruction> &instructions);
  void execInstructions();
};