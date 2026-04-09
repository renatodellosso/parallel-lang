#pragma once

#include "../cliUtils.hpp"
#include "../exitCode.hpp"
#include "../instruction.hpp"
#include <fstream>
#include <vector>

class Interpreter {
  const CliArgs &args;
  std::vector<Instruction> instructions;

public:
  Interpreter(const CliArgs &args);
  ExitCode interpret(std::istream &stream);
};