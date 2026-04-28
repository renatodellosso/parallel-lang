#include "function.hpp"

Function::Function(Instruction &instr) {
  returnType = std::get<std::string>(instr.bytecodeArgs[0].val);
  name = std::get<std::string>(instr.bytecodeArgs[1].val);
}

std::string Function::getName() const { return name; }
std::string Function::getReturnType() const { return returnType; }