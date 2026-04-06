#include "executor.hpp"
#include "../logging.hpp"

#define LOCATION "Executor"

Value Executor::pop()
{
  Value val = stack.top();
  stack.pop();
  return val;
}

void Executor::execSingleInstruction(Instruction instr)
{
  switch (instr.type)
  {
  case InstructionType::GetLiteral:
    stack.push(instr.bytecodeArgs[0]);
    break;
  case InstructionType::Add:
  {
    // Block so we can declare vars
    Value right = pop(), left = pop();

    Value result;
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
    stack.push(result);
    break;
  }
  case InstructionType::Subtract:
  {
    // Block so we can declare vars
    Value right = pop(), left = pop();

    if (left.type == ArgType::Integer && right.type == ArgType::Integer)
    {
      Value result = {
          .type = ArgType::Integer,
          .val = std::get<int>(left.val) - std::get<int>(right.val)};
      stack.push(result);
    }
    else
      throw std::runtime_error(std::format("Invalid arg types on instruction {}: {}", instr.id, (int)left.type, (int)right.type));

    break;
  }
  case InstructionType::Multiply:
  {
    // Block so we can declare vars
    Value right = pop(), left = pop();

    if (left.type == ArgType::Integer && right.type == ArgType::Integer)
    {
      Value result = {
          .type = ArgType::Integer,
          .val = std::get<int>(left.val) * std::get<int>(right.val)};
      stack.push(result);
    }
    else
      throw std::runtime_error(std::format("Invalid arg types on instruction {}: {}", instr.id, (int)left.type, (int)right.type));

    break;
  }
  case InstructionType::Divide:
  {
    // Block so we can declare vars
    Value right = pop(), left = pop();

    if (left.type == ArgType::Integer && right.type == ArgType::Integer)
    {
      Value result = {
          .type = ArgType::Integer,
          .val = std::get<int>(left.val) / std::get<int>(right.val)};
      stack.push(result);
    }
    else
      throw std::runtime_error(std::format("Invalid arg types on instruction {}: {}", instr.id, (int)left.type, (int)right.type));

    break;
  }

  default:
    throw std::runtime_error(std::format("Unknown instruction type on instruction {}: {}", instr.id, (int)instr.type));
  }

  // Clean up stack if at end of line
  if (instr.endsLine)
  {
    log(LOCATION, "{}", valToStr(stack.top()));
    stack = std::stack<Value>();
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

Executor::Executor(std::vector<Instruction> &instructions) : instructions(instructions), stack(std::stack<Value>()) {}