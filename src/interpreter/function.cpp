#include "function.hpp"
#include <vector>

template <class T> struct ParseResult {
  int newOffset;
  T res;
};

static ParseResult<std::unordered_map<
    std::string, std::vector<std::reference_wrapper<Instruction>>>>
parseUses(std::vector<Value> args, int offset,
          std::vector<Instruction> &instructions) {
  int keyCount = std::get<int>(args[offset++].val);

  std::unordered_map<std::string,
                     std::vector<std::reference_wrapper<Instruction>>>
      result;

  for (int i = 0; i < keyCount; i++) {
    auto key = std::get<std::string>(args[offset++].val);

    auto valCount = std::get<int>(args[offset++].val);
    std::vector<std::reference_wrapper<Instruction>> vals;
    for (int j = 0; j < valCount; j++) {
      auto id = std::get<int>(args[offset++].val);
      vals.push_back(instructions[id]);
    }

    result[key] = vals;
  }

  return {offset, result};
}

static ParseResult<
    std::unordered_map<std::string, std::reference_wrapper<Instruction>>>
parseWrites(std::vector<Value> args, int offset,
            std::vector<Instruction> &instructions) {
  int keyCount = std::get<int>(args[offset++].val);

  std::unordered_map<std::string, std::reference_wrapper<Instruction>> result;

  for (int i = 0; i < keyCount; i++) {
    auto key = std::get<std::string>(args[offset++].val);

    // Can't do map[key] = val since there's no default constructor for
    // reference wrappers
    result.emplace(key, instructions[std::get<int>(args[offset++].val)]);
  }

  return {offset, result};
}

Function::Function(Instruction &instr, std::vector<Instruction> &instructions) {
  returnType = std::get<std::string>(instr.bytecodeArgs[0].val);
  name = std::get<std::string>(instr.bytecodeArgs[1].val);

  int offset = 2;

  auto usesRes = parseUses(instr.bytecodeArgs, offset, instructions);
  firstUses = usesRes.res;
  offset = usesRes.newOffset;

  auto writesRes = parseWrites(instr.bytecodeArgs, offset, instructions);
  firstWrites = writesRes.res;
  offset = writesRes.newOffset;

  usesRes = parseUses(instr.bytecodeArgs, offset, instructions);
  lastUses = usesRes.res;
  offset = usesRes.newOffset;

  writesRes = parseWrites(instr.bytecodeArgs, offset, instructions);
  lastWrites = writesRes.res;
  offset = writesRes.newOffset;
}

std::string Function::getName() const { return name; }
std::string Function::getReturnType() const { return returnType; }

std::unordered_map<std::string,
                   std::vector<std::reference_wrapper<Instruction>>>
Function::getFirstUses() const {
  return firstUses;
}

std::unordered_map<std::string,
                   std::vector<std::reference_wrapper<Instruction>>>
Function::getLastUses() const {
  return lastUses;
}

std::unordered_map<std::string, std::reference_wrapper<Instruction>>
Function::getFirstWrites() const {
  return firstWrites;
}

std::unordered_map<std::string, std::reference_wrapper<Instruction>>
Function::getLastWrites() const {
  return lastWrites;
}