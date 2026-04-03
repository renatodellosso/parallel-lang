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

  // Does not consume whatever char signifies the end of the arg (' ' or ';' for example)
  Value buildArg();
  void buildSingleInstruction();

public:
  BytecodeParser(std::vector<Instruction> &instructions, std::istream &stream);
  void buildInstructions();
};