#include "subprogram.hpp"
#include <memory>
#include <vector>

Subprogram::Subprogram()
    : instrs(std::make_shared<std::vector<Instruction>>()) {}

Subprogram::Subprogram(Subprogram base, int startIndex, int size)
    : instrs(std::make_shared<std::vector<Instruction>>()) {
  for (int i = startIndex; i < startIndex + size; i++) {
    instrs->push_back(base[i]);
  }

  // Update dependents
  for (auto &instr : *instrs.get()) {
    for (auto &dep : instr.dependents) {
      if (dep.instr->id >= startIndex && dep.instr->id < startIndex + size)
        dep.instr = &instrs->at(dep.instr->id - startIndex);
    }
  }
}

Subprogram::Subprogram(std::shared_ptr<std::vector<Instruction>> instrs)
    : instrs(instrs) {
  for (auto &instr : *instrs) {
    for (auto &dep : instr.dependents)
      dep.instr = &instrs->at(dep.instr->id);
  }
}

Instruction &Subprogram::operator[](size_t i) { return instrs->at(i); }

std::vector<Instruction>::iterator Subprogram::begin() {
  return instrs->begin();
}

std::vector<Instruction>::iterator Subprogram::end() { return instrs->end(); }

int Subprogram::size() const { return instrs->size(); }