#include "executor.hpp"
#include "../logging.hpp"

#define LOCATION "Executor"

void Executor::pushResult(Instruction instr, Value result)
{
  for (auto dep : instr.dependents)
  {
    if (!dep.argIndex.has_value())
      continue;

    auto &depVec = instructions[dep.instrId].depArgs;
    int i = dep.argIndex.value();

    // Ensure vector has an index i
    if (depVec.size() < i + 1)
      depVec.resize(i + 1);

    depVec[i] = result;
  }
}

void Executor::execSingleInstruction(Instruction instr)
{
  Value result;

  switch (instr.type)
  {
  case InstructionType::GetLiteral:
    pushResult(instr, instr.bytecodeArgs[0]);
    break;
  case InstructionType::Add:
  {
    // Block so we can declare vars
    Value left = instr.depArgs[0], right = instr.depArgs[1];

    if (left.type == ArgType::Integer && right.type == ArgType::Integer)
    {
      result = {
          .type = ArgType::Integer,
          .val = std::get<int>(left.val) + std::get<int>(right.val)};
    }
    else if (left.type == ArgType::String || right.type == ArgType::String)
    {
      result = {
          .type = ArgType::String,
          .val = valToStr(left) + valToStr(right)};
    }
    else if (left.type == ArgType::Bool || right.type == ArgType::Bool)
    {
      result = {
          .type = ArgType::Bool,
          .val = valToBool(left) || valToBool(right)};
    }

    pushResult(instr, result);
    break;
  }
  case InstructionType::Subtract:
  {
    // Block so we can declare vars
    Value left = instr.depArgs[0], right = instr.depArgs[1];

    if (left.type == ArgType::Integer && right.type == ArgType::Integer)
    {
      result = {
          .type = ArgType::Integer,
          .val = std::get<int>(left.val) - std::get<int>(right.val)};
      pushResult(instr, result);
    }
    else
      throw std::runtime_error(std::format("Invalid arg types on instruction {}: {}", instr.id, (int)left.type, (int)right.type));

    break;
  }
  case InstructionType::Multiply:
  {
    // Block so we can declare vars
    Value left = instr.depArgs[0], right = instr.depArgs[1];

    if (left.type == ArgType::Integer && right.type == ArgType::Integer)
    {
      result = {
          .type = ArgType::Integer,
          .val = std::get<int>(left.val) * std::get<int>(right.val)};
      pushResult(instr, result);
    }
    else
      throw std::runtime_error(std::format("Invalid arg types on instruction {}: {}", instr.id, (int)left.type, (int)right.type));

    break;
  }
  case InstructionType::Divide:
  {
    // Block so we can declare vars
    Value left = instr.depArgs[0], right = instr.depArgs[1];

    if (left.type == ArgType::Integer && right.type == ArgType::Integer)
    {
      result = {
          .type = ArgType::Integer,
          .val = std::get<int>(left.val) / std::get<int>(right.val)};
      pushResult(instr, result);
    }
    else
      throw std::runtime_error(std::format("Invalid arg types on instruction {}: {}", instr.id, (int)left.type, (int)right.type));

    break;
  }

  default:
    throw std::runtime_error(std::format("Unknown instruction type on instruction {}: {}", instr.id, (int)instr.type));
  }

  for (auto dep : instr.dependents)
    instructions[dep.instrId].depsFulfilled++;

  // Clean up stack if at end of line
  if (instr.endsLine)
  {
    log(LOCATION, "{}", valToStr(result));
  }
}

void Executor::execInstructions()
{
  for (auto instr : instructions)
  {
    execSingleInstruction(instr);
  }

  log(LOCATION, "Done! Executed {} instructions.", instructions.size());
}

Executor::Executor(std::vector<Instruction> &instructions) : instructions(instructions) {}