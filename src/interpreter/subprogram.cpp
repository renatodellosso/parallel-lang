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

  // Update IDs and dependents
  for (int i = 0; i < size; i++) {
    auto &instr = instrs->at(i);
    instr.id = i;
    for (auto &dep : instr.dependents) {
      if (dep.instr->id >= startIndex && dep.instr->id < startIndex + size) {
        dep.instr = &instrs->at(dep.instr->id - startIndex);
      }
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

void Subprogram::setSubprogramPointers(std::shared_ptr<Subprogram> self) {
  for (auto &instr : *instrs)
    instr.program = self;
}

Instruction &Subprogram::operator[](size_t i) { return instrs->at(i); }

std::vector<Instruction>::iterator Subprogram::begin() {
  return instrs->begin();
}

std::vector<Instruction>::iterator Subprogram::end() { return instrs->end(); }

Instruction &Subprogram::at(int index) { return instrs->at(index); }

int Subprogram::size() const { return instrs->size(); }

std::shared_ptr<Subprogram> Subprogram::clone() const {
  auto clone = std::make_shared<Subprogram>(*this, 0, size());
  clone->setSubprogramPointers(clone);
  return clone;
}