#pragma once

#include "../exitCode.hpp"
#include "../cliUtils.hpp"
#include "../instruction.hpp"
#include <vector>
#include <queue>

class Executor
{
  const CliArgs &cliArgs;
  std::vector<Instruction> &instructions;
  // std::queue<Instruction &> queue;

  void pushResult(Instruction instr, Value result);
  void execSingleInstruction(Instruction instr);

public:
  Executor(const CliArgs &cliArgs, std::vector<Instruction> &instructions);
  void execInstructions();
};