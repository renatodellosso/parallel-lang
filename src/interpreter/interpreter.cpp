#include "interpreter.hpp"
#include "bytecodeParser.hpp"
#include "executor.hpp"
#include "../utils.hpp"
#include "../logging.hpp"

#define LOCATION "Interpreter"

Interpreter::Interpreter(const CliArgs &args) : args(args), instructions(std::vector<Instruction>()) {}

ExitCode Interpreter::interpret(std::istream &stream)
{
  if (args.verbose)
    log(LOCATION, "Interpreting file '{}'...", args.target);

  try
  {
    BytecodeParser parser(args, instructions, stream);
    parser.buildInstructions();
  }
  catch (std::runtime_error err)
  {
    logError(LOCATION, "Encountered error parsing bytecode: {}", err.what());
    return ExitCode::BytecodeParseError;
  }

  try
  {
    Executor executor(args, instructions);
    executor.execInstructions();
  }
  catch (std::runtime_error err)
  {
    logError(LOCATION, "Encountered error executing bytecode: {}", err.what());
    return ExitCode::ExecutionError;
  }

  return ExitCode::Ok;
}