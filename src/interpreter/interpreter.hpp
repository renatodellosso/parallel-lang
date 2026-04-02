#pragma once

#include "../exitCode.hpp"
#include "../cliUtils.hpp"
#include "line.hpp"
#include <vector>
#include <fstream>

class Interpreter
{
  const CliArgs &args;
  std::istream &stream;
  std::vector<Line> lines;
  int lineCount;

  ExitCode buildSingleInstruction();
  ExitCode buildLines();

public:
  Interpreter(const CliArgs &args, std::istream &stream);
  ExitCode interpret();
};