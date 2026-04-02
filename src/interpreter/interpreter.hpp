#pragma once

#include "../exitCode.hpp"
#include "../cliUtils.hpp"
#include "../instruction.hpp"
#include <vector>
#include <fstream>

class Interpreter
{
  const CliArgs &args;
  std::vector<Instruction> instructions;

public:
  Interpreter(const CliArgs &args);
  ExitCode interpret(std::istream &stream);
};