#pragma once

#include "../instruction.hpp"
#include "subprogram.hpp"
#include <string>
#include <unordered_map>

class Function {
  std::string name;
  std::string returnType;
  std::unordered_map<std::string,
                     std::vector<std::reference_wrapper<Instruction>>>
      firstUses, lastUses;
  std::unordered_map<std::string, std::reference_wrapper<Instruction>>
      firstWrites, lastWrites;

  // Maps names to types
  std::unordered_map<std::string, std::string> params;

  Subprogram body;

public:
  Function(Instruction &instr, Subprogram &instructions);

  std::string getName() const;
  std::string getReturnType() const;
  std::unordered_map<std::string,
                     std::vector<std::reference_wrapper<Instruction>>>
  getFirstUses() const;
  std::unordered_map<std::string,
                     std::vector<std::reference_wrapper<Instruction>>>
  getLastUses() const;
  std::unordered_map<std::string, std::reference_wrapper<Instruction>>
  getFirstWrites() const;
  std::unordered_map<std::string, std::reference_wrapper<Instruction>>
  getLastWrites() const;
  std::unordered_map<std::string, std::string> getParams() const;
};