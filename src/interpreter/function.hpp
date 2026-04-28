#pragma once

#include "../instruction.hpp"

class Function {
  std::string name;
  std::string returnType;

public:
  Function(Instruction &instr);

  std::string getName() const;
  std::string getReturnType() const;
};