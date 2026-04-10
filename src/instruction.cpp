#include "instruction.hpp"
#include <format>

std::string instructionTypeToString(InstructionType type) {
  switch (type) {
  case InstructionType::Block:
    return "Block";
  case InstructionType::GetLiteral:
    return "GetLiteral";
  case InstructionType::GetIdentifier:
    return "GetIdentifier";
  case InstructionType::ReferenceIdentifier:
    return "ReferenceIdentifier";
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
  default:
    return "Unknown Expression Type";
  }
}

InstrDependent::InstrDependent(int instrId, std::optional<int> argIndex)
    : instrId(instrId), argIndex(argIndex) {}

InstrDependent::InstrDependent(int instrId, int argIndex)
    : InstrDependent(instrId, std::make_optional(argIndex)) {}

InstrDependent::InstrDependent(int instrId)
    : InstrDependent(instrId, std::nullopt) {}

Instruction::Instruction(int id)
    : id(id), endsLine(false), type((InstructionType)0), executed(false),
      bytecodeArgs(std::vector<Value>()), depArgs(std::vector<Value>()),
      depCount(0), depsFulfilled(0), dependents(std::vector<InstrDependent>()) {
}

std::string Instruction::toString() {
  std::string str = std::format(
      "{}: {}(endsLine: {}, dependencies: {}/{}, bytecode args: [", id,
      instructionTypeToString(type), endsLine, depsFulfilled, depCount);

  for (auto arg : bytecodeArgs)
    str += valToStr(arg) + ", ";
  if (bytecodeArgs.size() > 0)
    str = str.substr(0, str.length() - 2);
  str += "], dep args: [";

  for (auto arg : depArgs)
    str += valToStr(arg) + ", ";
  if (depArgs.size() > 0)
    str = str.substr(0, str.length() - 2);
  str += "], dependents: [";

  for (auto dep : dependents) {
    str += std::to_string(dep.instrId);
    if (dep.argIndex.has_value())
      str += "." + std::to_string(dep.argIndex.value());
    str += ", ";
  }
  if (dependents.size() > 0)
    str = str.substr(0, str.length() - 2);
  str += "]";

  return str + ")";
}
