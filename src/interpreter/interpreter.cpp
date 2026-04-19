#include "interpreter.hpp"
#include "../logging.hpp"
#include "../utils.hpp"
#include "bytecodeParser.hpp"
#include "executor.hpp"
#include <chrono>

#define LOCATION "Interpreter"

Interpreter::Interpreter(const CliArgs &args)
    : args(args), instructions(std::vector<Instruction>()) {}

ExitCode Interpreter::interpret(std::istream &stream) {
  auto start = std::chrono::steady_clock::now();

  if (args.verbose)
    log(LOCATION, "Interpreting file '{}'...", args.target);

  try {
    BytecodeParser *parser = new BytecodeParser(args, instructions, stream);
    parser->buildInstructions();

    delete parser; // match new with delete
  } catch (std::runtime_error err) {
    logError(LOCATION, "Encountered error parsing bytecode: {}", err.what());
    return ExitCode::BytecodeParseError;
  }

  try {
    Executor *executor = new Executor(args, instructions);
    executor->startExecution();

    delete executor;
  } catch (std::runtime_error err) {
    logError(LOCATION, "Encountered error executing bytecode: {}", err.what());
    return ExitCode::ExecutionError;
  }

  auto end = std::chrono::steady_clock::now();
  auto duration = end - start;

  if (args.verbose)
    log(LOCATION, "Finished in {}", formatNs(end - start));

  return ExitCode::Ok;
}