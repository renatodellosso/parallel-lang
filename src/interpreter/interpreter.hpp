#pragma once

#include "../exitCode.hpp"
#include "../cliUtils.hpp"
#include "../instruction.hpp"
#include <vector>
#include <fstream>

class Interpreter
{
  const CliArgs &args;
  std::istream &stream;
  std::vector<Instruction> instructions;

  void buildSingleInstruction();
  void buildInstructions();

  void execSingleInstruction(const Instruction &instr);
  void execInstructions();

public:
  Interpreter(const CliArgs &args, std::istream &stream);
  ExitCode interpret();
};