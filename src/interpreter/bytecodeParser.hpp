#pragma once

#include "../exitCode.hpp"
#include "../cliUtils.hpp"
#include "../instruction.hpp"
#include <vector>
#include <fstream>

class BytecodeParser
{
  std::vector<Instruction> &instructions;
  std::istream &stream;

  void buildSingleInstruction();

public:
  BytecodeParser(std::vector<Instruction> &instructions, std::istream &stream);
  void buildInstructions();
};