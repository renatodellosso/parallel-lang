#include "executor.hpp"
#include "../logging.hpp"

#define LOCATION "Executor"

void Executor::updateDependency(InstrDependent dep, Value result) {
  auto &depInstr = instructions[dep.instrId];

  int fulfilled;

  // Smaller scope so the lock guard is cleaned up
  {
    std::lock_guard<std::mutex> fulfilledLock(
        depsFulfilledMutexes[dep.instrId]);

    depInstr.depsFulfilled++;
    fulfilled = depInstr.depsFulfilled;
  }

  if (dep.argIndex.has_value()) {
    std::lock_guard<std::mutex> argLock(depArgsMutexes[dep.instrId]);

    auto &depVec = depInstr.depArgs;
    int i = dep.argIndex.value();

    // Ensure vector has an index i
    if (depVec.size() < i + 1)
      depVec.resize(i + 1);

    depVec[i] = result;
  }

  if (fulfilled == depInstr.depCount)
    queue.push(depInstr);
}

void Executor::execSingleInstruction(Instruction &instr) {
  Value result;

  switch (instr.type) {
  case InstructionType::GetLiteral:
    result = instr.bytecodeArgs[0];
    break;
  case InstructionType::Add: {
    // Block so we can declare vars
    Value left = instr.depArgs[0], right = instr.depArgs[1];

    if (left.type == ValueType::Integer && right.type == ValueType::Integer) {
      result = {.type = ValueType::Integer,
                .val = std::get<int>(left.val) + std::get<int>(right.val)};
    } else if (left.type == ValueType::String ||
               right.type == ValueType::String) {
      result = {.type = ValueType::String,
                .val = valToStr(left) + valToStr(right)};
    } else if (left.type == ValueType::Bool || right.type == ValueType::Bool) {
      result = {.type = ValueType::Bool,
                .val = valToBool(left) || valToBool(right)};
    }
    break;
  }
  case InstructionType::Subtract: {
    // Block so we can declare vars
    Value left = instr.depArgs[0], right = instr.depArgs[1];

    if (left.type == ValueType::Integer && right.type == ValueType::Integer) {
      result = {.type = ValueType::Integer,
                .val = std::get<int>(left.val) - std::get<int>(right.val)};
    } else
      throw std::runtime_error(
          std::format("Invalid arg types on instruction {}: {}", instr.id,
                      (int)left.type, (int)right.type));

    break;
  }
  case InstructionType::Multiply: {
    // Block so we can declare vars
    Value left = instr.depArgs[0], right = instr.depArgs[1];

    if (left.type == ValueType::Integer && right.type == ValueType::Integer) {
      result = {.type = ValueType::Integer,
                .val = std::get<int>(left.val) * std::get<int>(right.val)};
    } else
      throw std::runtime_error(
          std::format("Invalid arg types on instruction {}: {}", instr.id,
                      (int)left.type, (int)right.type));

    break;
  }
  case InstructionType::Divide: {
    // Block so we can declare vars
    Value left = instr.depArgs[0], right = instr.depArgs[1];

    if (left.type == ValueType::Integer && right.type == ValueType::Integer) {
      result = {.type = ValueType::Integer,
                .val = std::get<int>(left.val) / std::get<int>(right.val)};
    } else
      throw std::runtime_error(
          std::format("Invalid arg types on instruction {}: {}", instr.id,
                      (int)left.type, (int)right.type));

    break;
  }

  default:
    throw std::runtime_error(
        std::format("Unknown instruction type on instruction {}: {}", instr.id,
                    (int)instr.type));
  }

  for (auto dep : instr.dependents) {
    if (cliArgs.verbose)
      log(LOCATION, "Updating dependency {} -> {}", instr.id, dep.instrId);
    updateDependency(dep, result);
  }

  instr.executed = true;
  if (cliArgs.verbose)
    log(LOCATION, "Executed instruction {}", instr.id);

  // Clean up stack if at end of line
  if (instr.endsLine) {
    log(LOCATION, "{}", valToStr(result));
  }
}

void Executor::execWorker(int id) {
  std::string location = LOCATION + std::string(":worker") + std::to_string(id);
  if (cliArgs.verbose)
    log(location.c_str(), "Worker {} awake", id);

  try {
    while (!halt) {
      if (queue.size() == 0)
        continue;
      auto &instr = queue.pop().get();
      execSingleInstruction(instr);
    }
  } catch (std::runtime_error err) {
    logError(LOCATION, "{}", err.what());
    halt = true;
    success = false;
  }
}

void Executor::supervisor() {
  if (cliArgs.verbose)
    log(LOCATION, "Supervisor awake");

  bool isDone = true;
  do {
    isDone = true;

    for (int i = 0; i < instructions.size(); i++) {
      if (cliArgs.verbose)
        log(LOCATION, "Instruction {} done: {}", i, instructions[i].executed);
      if (!instructions[i].executed) {
        isDone = false;
        break;
      }
    }
  } while (!isDone && !halt);

  halt = true;

  if (cliArgs.verbose)
    log(LOCATION, "Executor halted! Success: {}", success);

  for (int i = 0; i < workers.size(); i++) {
    // Can only detach from joinable threads
    if (workers[i].joinable())
      workers[i].join();
  }
}

void Executor::initQueue() {
  for (int i = 0; i < instructions.size(); i++) {
    auto &instr = instructions[i];
    if (instr.depsFulfilled == instr.depCount)
      queue.push(instr);
  }

  if (cliArgs.verbose)
    log(LOCATION, "Pushed {} instructions onto the queue.", queue.size());
}

void Executor::startExecution() {
  initQueue();

  if (cliArgs.verbose)
    log(LOCATION, "Starting {} execution workers...", cliArgs.threads);

  for (int i = 0; i < cliArgs.threads; i++)
    workers.emplace_back(&Executor::execWorker, this,
                         i); // Can't just pass execWorker since it's a method

  supervisor();

  if (success && cliArgs.verbose)
    log(LOCATION, "Done! Executed {} instructions.", instructions.size());

  if (!success)
    logError(LOCATION, "Encountered error during execution.");
}

Executor::Executor(const CliArgs &cliArgs,
                   std::vector<Instruction> &instructions)
    : cliArgs(cliArgs), instructions(instructions),
      depArgsMutexes(std::vector<std::mutex>(instructions.size())),
      depsFulfilledMutexes(std::vector<std::mutex>(instructions.size())),
      success(true) {}