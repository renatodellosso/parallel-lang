#include "bytecodeParser.hpp"
#include "../utils.hpp"
#include "../logging.hpp"

#define LOCATION "BytecodeParser"

void BytecodeParser::buildSingleInstruction()
{
  Instruction instr = {
      .lineNumber = (int)instructions.size(),
      .endsLine = false,
      .type = InstructionType::Add,
      .args = std::vector<Value>()};

  // Parse type
  std::string curr = "";
  for (char c = stream.peek(); c != ' '; c = stream.peek())
  {
    stream.get();
    curr += c;
  }
  instr.type = (InstructionType)std::atoi(curr.c_str());

  stream.get(); // Consume ' '

  // Parse args
  while (stream.peek() != '\n' && stream.peek() != '\r' && !stream.eof())
  {
    while (stream.peek() == ' ')
    {
      stream.get();
    }

    // Found arg, consume it
    curr = "";

    bool inStr = false;
    for (char c = stream.get(); true; c = stream.get())
    {
      if (c == '"')
      {
        if (!inStr)
          inStr = true;
        else
        {
          curr += c;
          break;
        }
      }
      if (!inStr && (c == ' ' || c == '\n' || c == '\r' || c == ';'))
      {
        break;
      }
      curr += c;
    }

    Value arg;
    if (curr.at(0) == '"' && curr.at(curr.length() - 1) == '"')
    {
      // String
      arg.type = ArgType::String,
      arg.val = curr.substr(1, curr.length() - 2); // Remove ""
    }
    else if (curr == "true")
    {
      // bool (true)
      arg.type = ArgType::Bool,
      arg.val = true;
    }
    else if (curr == "false")
    {
      // bool (true)
      arg.type = ArgType::Bool,
      arg.val = false;
    }
    else if (isInteger(curr))
    {
      // int
      arg.type = ArgType::Integer,
      arg.val = std::atoi(curr.c_str());
    }
    else
    {
      throw std::runtime_error(std::format("Unknown argument format on instruction {}: '{}'", instructions.size(), curr));
    }

    instr.args.push_back(arg);
  }

  log(LOCATION, "Parsed instruction: type: {}, arg count: {}", (int)instr.type, instr.args.size());
  instructions.push_back(instr);
  return;
}

void BytecodeParser::buildInstructions()
{
  while (!stream.eof())
  {
    buildSingleInstruction();

    stream.get(); // Consume '\n'
  }

  log(LOCATION, "Parsed {} instructions", instructions.size());
}

BytecodeParser::BytecodeParser(std::vector<Instruction> &instructions, std::istream &stream) : instructions(instructions), stream(stream) {}