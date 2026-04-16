#include "instruction.hpp"
#include <format>
#include <memory>

std::string instructionTypeToString(InstructionType type) {
  switch (type) {
  case InstructionType::Block:
    return "Block";
  case InstructionType::GetLiteral:
    return "GetLiteral";
  case InstructionType::GetIdentifier:
    return "GetIdentifier";
  case InstructionType::Declare:
    return "Declare";
  case InstructionType::Set:
    return "Set";
  case InstructionType::Add:
    return "Add";
  case InstructionType::Subtract:
    return "Subtract";
  case InstructionType::Multiply:
    return "Multiply";
  case InstructionType::Divide:
    return "Divide";
  case InstructionType::Negate:
    return "Negate";
  case InstructionType::CompareEquals:
    return "CompareEquals";
  case InstructionType::CompareLessThan:
    return "CompareLessThan";
  case InstructionType::CompareLessThanEquals:
    return "CompareLessThanEquals";
  case InstructionType::CompareGreaterThan:
    return "CompareGreaterThan";
  case InstructionType::CompareGreaterThanEquals:
    return "CompareGreaterThanEquals";
  case InstructionType::If:
    return "If";
  case InstructionType::GoTo:
    return "GoTo";
  default:
    return "UnknownInstructionType";
  }
}

InstrDependent::InstrDependent(Instruction *instr, std::optional<int> argIndex)
    : instr(instr), argIndex(argIndex) {}

InstrDependent::InstrDependent(Instruction *instr, int argIndex)
    : InstrDependent(instr, std::make_optional(argIndex)) {}

InstrDependent::InstrDependent(Instruction *instr)
    : InstrDependent(instr, std::nullopt) {}

Instruction::Instruction(int id, std::shared_ptr<Scope<Value>> scope)
    : id(id), type((InstructionType)0), bytecodeArgs(std::vector<Value>()),
      depArgs(std::vector<std::shared_ptr<Value>>()), depCount(0),
      depsFulfilled(0), dependents(std::vector<InstrDependent>()),
      scope(scope) {}

std::string Instruction::toString() {
  std::string str = std::format(
      "({}){}(dependencies: {}/{}, scope depth: {}, bytecode args: [", id,
      instructionTypeToString(type), depsFulfilled, depCount,
      scope ? scope->getDepth() : -1);

  for (auto arg : bytecodeArgs)
    str += valToStr(arg) + ", ";
  if (bytecodeArgs.size() > 0)
    str = str.substr(0, str.length() - 2);
  str += "], dep args: [";

  for (auto arg : depArgs)
    str += valToStr(*arg) + ", ";
  if (depArgs.size() > 0)
    str = str.substr(0, str.length() - 2);
  str += "], dependents: [";

  for (auto dep : dependents) {
    str += std::to_string(dep.instr->id);
    if (dep.argIndex.has_value())
      str += "." + std::to_string(dep.argIndex.value());
    str += ", ";
  }
  if (dependents.size() > 0)
    str = str.substr(0, str.length() - 2);
  str += "]";

  return str + ")";
}
