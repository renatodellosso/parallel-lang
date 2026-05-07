#pragma once

#include "../instruction.hpp"
#include <memory>
#include <vector>

class Subprogram {
  std::shared_ptr<std::vector<Instruction>> instrs;

  void setSubprogramPointers(std::shared_ptr<Subprogram> self);

public:
  Subprogram();
  // Be sure to call setSubprogramPointers!
  Subprogram(Subprogram base, int startIndex, int size);
  Subprogram(std::shared_ptr<std::vector<Instruction>> instrs);

  Instruction &operator[](size_t i);

  // For use in range-based for loops (for each loops)
  std::vector<Instruction>::iterator begin();
  std::vector<Instruction>::iterator end();
  Instruction &at(int index);

  std::shared_ptr<Subprogram> clone() const;

  int size() const;
};