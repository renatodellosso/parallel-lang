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
    BytecodeParser *parser = new BytecodeParser(args, instructions, stream);
    parser->buildInstructions();

    delete parser; // match new with free
  }
  catch (std::runtime_error err)
  {
    logError(LOCATION, "Encountered error parsing bytecode: {}", err.what());
    return ExitCode::BytecodeParseError;
  }

  try
  {
    Executor *executor = new Executor(args, instructions);
    executor->startExecution();

    delete executor;
  }
  catch (std::runtime_error err)
  {
    logError(LOCATION, "Encountered error executing bytecode: {}", err.what());
    return ExitCode::ExecutionError;
  }

  return ExitCode::Ok;
}