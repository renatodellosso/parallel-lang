#include "executor.hpp"
#include "../logging.hpp"
#include "scope.hpp"
#include <chrono>
#include <format>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <thread>
#include <vector>

#define LOCATION "Executor"

void Executor::updateDependency(InstrDependent dep,
                                std::shared_ptr<Value> result) {
  int fulfilled;

  // Smaller scope so the lock guard is cleaned up
  {
    std::lock_guard<std::mutex> fulfilledLock(
        depsFulfilledMutexes[dep.instr->id]);

    dep.instr->depsFulfilled++;
    fulfilled = dep.instr->depsFulfilled;
  }

  if (dep.argIndex.has_value()) {
    std::lock_guard<std::mutex> argLock(depArgsMutexes[dep.instr->id]);

    auto &depVec = dep.instr->depArgs;
    int i = dep.argIndex.value();

    // Ensure vector has an index i
    if (depVec.size() < i + 1)
      depVec.resize(i + 1);

    depVec[i] = result;
  }

  if (fulfilled == dep.instr->depCount)
    queue.push(*dep.instr);
}

void Executor::skipInstruction(Instruction &instr, bool recurse) {
  for (auto dep : instr.dependents) {
    // Deps with indices are intra-line deps and the entire line will be
    // skipped, so we don't have to worry about them
    if (!dep.argIndex.has_value())
      updateDependency(dep, nullptr);
  }

  if (!recurse || instr.type != InstructionType::Block)
    return;

  int toSkip = std::get<int>(instr.depArgs[0]->val);
  for (int i = 1; i < toSkip; i++) {
    skipInstruction(instructions[instr.id + i], false);
  }
}

void Executor::execSingleInstruction(Instruction &instr) {
  if (cliArgs.verbose)
    log(LOCATION, "Executing instruction: {}", instr.toString());
  std::shared_ptr<Value> result;

  switch (instr.type) {
  case InstructionType::Block: {
    // Create new scope
    std::shared_ptr<Scope> scope = std::make_shared<Scope>(
        instr.scope); // make_shared() uses the constructor
    int size = std::get<int>(instr.bytecodeArgs[0].val);

    int skip = 0;
    for (int i = 0; i < size; i++) {
      if (skip > 0) {
        skip--;
        continue;
      }

      auto &inner = instructions[instr.id + i + 1];
      if (inner.type == InstructionType::Block) {
        skip = std::get<int>(inner.bytecodeArgs[0].val);
      }
      inner.scope = scope;
    }
    break;
  }
  case InstructionType::GetLiteral:
    result = std::make_shared<Value>(instr.bytecodeArgs[0]);
    break;
  case InstructionType::GetIdentifier: {
    auto ptr =
        instr.scope->get(std::get<std::string>(instr.bytecodeArgs[0].val));
    if (!ptr)
      throw std::runtime_error(std::format(
          "Attempted to read identifier '{}', but it did not exist!",
          std::get<std::string>(instr.bytecodeArgs[0].val)));
    result = ptr;
    break;
  }
  case InstructionType::Declare: {
    result = instr.scope->alloc(std::get<std::string>(instr.depArgs[1]->val));
    break;
  }
  case InstructionType::Set: {
    result = instr.depArgs[0];
    auto val = instr.depArgs[1];
    result->type = val->type;
    result->val = val->val;
    break;
  }
  case InstructionType::Add: {
    // Block so we can declare vars
    std::shared_ptr<Value> left = instr.depArgs[0], right = instr.depArgs[1];

    if (left->type == ValueType::Integer && right->type == ValueType::Integer) {
      result = std::make_shared<Value>(ValueType::Integer,
                                       std::get<int>(left->val) +
                                           std::get<int>(right->val));
    } else if (left->type == ValueType::String ||
               right->type == ValueType::String) {
      result = std::make_shared<Value>(ValueType::String,
                                       valToStr(*left) + valToStr(*right));

    } else if (left->type == ValueType::Bool ||
               right->type == ValueType::Bool) {
      result = std::make_shared<Value>(ValueType::Bool,
                                       valToBool(*left) || valToBool(*right));
    }
    break;
  }
  case InstructionType::Subtract: {
    // Block so we can declare vars
    std::shared_ptr<Value> left = instr.depArgs[0], right = instr.depArgs[1];

    if (left->type == ValueType::Integer && right->type == ValueType::Integer) {
      result = std::make_shared<Value>(ValueType::Integer,
                                       std::get<int>(left->val) -
                                           std::get<int>(right->val));
    } else
      throw std::runtime_error(
          std::format("Invalid arg types on instruction {}: {}", instr.id,
                      (int)left->type, (int)right->type));

    break;
  }
  case InstructionType::Multiply: {
    // Block so we can declare vars
    std::shared_ptr<Value> left = instr.depArgs[0], right = instr.depArgs[1];

    if (left->type == ValueType::Integer && right->type == ValueType::Integer) {
      result = std::make_shared<Value>(ValueType::Integer,
                                       std::get<int>(left->val) *
                                           std::get<int>(right->val));
    } else
      throw std::runtime_error(
          std::format("Invalid arg types on instruction {}: {}", instr.id,
                      (int)left->type, (int)right->type));

    break;
  }
  case InstructionType::Divide: {
    // Block so we can declare vars
    std::shared_ptr<Value> left = instr.depArgs[0], right = instr.depArgs[1];

    if (left->type == ValueType::Integer && right->type == ValueType::Integer) {
      result = std::make_shared<Value>(ValueType::Integer,
                                       std::get<int>(left->val) /
                                           std::get<int>(right->val));
    } else
      throw std::runtime_error(
          std::format("Invalid arg types on instruction {}: {}", instr.id,
                      (int)left->type, (int)right->type));

    break;
  }
  case InstructionType::If: {
    bool condition = valToBool(*instr.depArgs[0]);
    if (condition)
      break;

    // Skip next instruction
    skipInstruction(instructions[instr.id + 1]);
    break;
  }
  default:
    throw std::runtime_error(
        std::format("Unknown instruction type on instruction {}: {}", instr.id,
                    (int)instr.type));
  }

  for (auto dep : instr.dependents) {
    updateDependency(dep, result);
  }

  log(LOCATION, "[instruction {}]: {}", instr.id,
      result ? valToStr(*result) : "<no result>");
}

void Executor::execWorker(int id) {
  auto location = LOCATION + std::string(":worker") + std::to_string(id);
  if (cliArgs.verbose)
    log(location.c_str(), "Worker {} awake", id);

  while (!halt) {
    if (queue.size() == 0) {
      stalls[id] = true;
      continue;
    }
    stalls[id] = false;

    auto &instr = queue.pop().get();
    try {
      execSingleInstruction(instr);
    } catch (std::runtime_error err) {
      logError(location.c_str(), "[instruction {}] {}", instr.id, err.what());
      haltCause = std::format("Runtime Error in worker {} [instruction {}]: {}",
                              id, instr.id, err.what());
      halt = true;
    }
  }
}

void Executor::supervisor() {
  if (cliArgs.verbose)
    log(LOCATION, "Supervisor awake");

  bool isDone;
  do {
    isDone = true;

    for (auto stall : stalls) {
      if (!stall) {
        isDone = false;
        break;
      }
    }

    // Removing this breaks tests on Ubuntu. Not sure why, but it reduces the
    // cycles used by the supervisor
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  } while (!isDone && !halt);

  if (!halt)
    haltCause = "All workers stalled";
  halt = true;

  if (cliArgs.verbose)
    log(LOCATION, "Executor halted! Cause: {}", haltCause);

  for (int i = 0; i < workers.size(); i++) {
    // Can only detach from joinable threads
    if (workers[i].joinable()) {
      workers[i].join();
    }
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

void Executor::initScopes() {
  std::shared_ptr<Scope> root = std::make_shared<Scope>();
  std::shared_ptr<Scope> global = std::make_shared<Scope>(root);

  for (auto &instr : instructions) {
    instr.scope = global;
  }

  // Init default vars
  root->alloc("int");
  root->alloc("bool");
  root->alloc("string");

  if (cliArgs.verbose)
    log(LOCATION, "Initialized scopes.");
}

void Executor::startExecution() {
  initScopes();
  initQueue();

  if (cliArgs.verbose)
    log(LOCATION, "Starting {} execution workers...", cliArgs.threads);

  for (int i = 0; i < cliArgs.threads; i++)
    workers.emplace_back(&Executor::execWorker, this,
                         i); // Can't just pass execWorker since it's a method

  supervisor();

  if (cliArgs.verbose)
    log(LOCATION, "Done! Executed {} instructions. Halt cause: {}",
        instructions.size(), haltCause);
}

Executor::Executor(const CliArgs &cliArgs,
                   std::vector<Instruction> &instructions)
    : cliArgs(cliArgs), instructions(instructions),
      stalls(std::vector<bool>(cliArgs.threads)),
      depArgsMutexes(std::vector<std::mutex>(instructions.size())),
      depsFulfilledMutexes(std::vector<std::mutex>(instructions.size())),
      halt(false), haltCause("Unknown") {}