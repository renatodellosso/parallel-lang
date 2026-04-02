#include "interpreter.hpp"
#include "../utils.hpp"
#include "../logging.hpp"

#define LOCATION "Interpreter"

ExitCode Interpreter::buildSingleInstruction()
{
  Line line = {
      .lineNumber = lineCount,
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
  line.type = (InstructionType)std::atoi(curr.c_str());

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
        if (c == ';')
          lineCount++;
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
      logError(LOCATION, "Unknown argument format on line {}: '{}'", lineCount, curr);
      return ExitCode::InvalidBytecode;
    }

    line.args.push_back(arg);
  }

  return ExitCode::Ok;
}

ExitCode Interpreter::buildLines()
{
  while (!stream.eof())
  {
    ExitCode code = buildSingleInstruction();
    if (code != ExitCode::Ok)
      return code;

    stream.get(); // Consume '\n'
  }

  log(LOCATION, "Parsed {} lines", lineCount);

  return ExitCode::Ok;
}

Interpreter::Interpreter(const CliArgs &args, std::istream &stream) : args(args), stream(stream), lines(std::vector<Line>()), lineCount(1) {}

ExitCode Interpreter::interpret()
{
  log(LOCATION, "Interpreting file '{}'...", args.target);

  ExitCode code = buildLines();
  if (code != ExitCode::Ok)
    return code;

  return ExitCode::Ok;
}