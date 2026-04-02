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
  case InstructionType::Add:
  {
    // Block so we can declare vars
    Value right = pop(), left = pop();

    if (left.type == ArgType::Integer && right.type == ArgType::Integer)
    {
      Value result = {
          .type = ArgType::Integer,
          .val = std::get<int>(left.val) + std::get<int>(right.val)};
      stack.push(result);
    }
  }
  break;
  case InstructionType::GetLiteral:
    stack.push(instr.args[0]);
    break;

  default:
    throw std::runtime_error(std::format("Unknown instruction on line {}: {}", instr.lineNumber, (int)instr.type));
  }

  // Log top of stack
  log(LOCATION, "Top of Stack: {}, {}", (int)stack.top().type, valToStr(stack.top()));

  // Clean up stack if at end of line
  if (instr.endsLine)
    stack = std::stack<Value>();
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