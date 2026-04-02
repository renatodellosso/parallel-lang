#include "interpreter.hpp"
#include "../utils.hpp"
#include "../logging.hpp"

#define LOCATION "Interpreter"

void Interpreter::buildSingleInstruction()
{
  Instruction instr = {
      .lineNumber = (int)instructions.size(),
      .endsLine = false,
      .type = InstructionType::Add,
      .args = std::vector<Arg>()};

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

    Arg arg;
    if (curr.at(0) == '"' && curr.at(curr.length() - 1) == '"')
    {
      // String
      arg.type = ArgType::String,
      arg.val = curr;
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

  instructions.push_back(instr);
  return;
}

void Interpreter::buildInstructions()
{
  while (!stream.eof())
  {
    buildSingleInstruction();

    stream.get(); // Consume '\n'
  }

  log(LOCATION, "Parsed {} instructions", instructions.size());
}

void Interpreter::execSingleInstruction(const Instruction &instr)
{
}

void Interpreter::execInstructions()
{
  for (auto instr : instructions)
  {
    execSingleInstruction(instr);
  }

  log(LOCATION, "Done! Executed {} instructions.", instructions.size());
}

Interpreter::Interpreter(const CliArgs &args, std::istream &stream) : args(args), stream(stream), instructions(std::vector<Instruction>()) {}

ExitCode Interpreter::interpret()
{
  log(LOCATION, "Interpreting file '{}'...", args.target);

  try
  {
    buildInstructions();
    execInstructions();
  }
  catch (std::runtime_error err)
  {
    logError(LOCATION, "Encountered runtime error: {}", err.what());
    return ExitCode::RuntimeError;
  }

  return ExitCode::Ok;
}