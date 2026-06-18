#include "executor.hpp"
#include "../logging.hpp"
#include "../scope.hpp"
#include "function.hpp"
#include "subprogram.hpp"
#include <chrono>
#include <cstdint>
#include <format>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <thread>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

#define LOCATION "Executor"

ExecutionStats::ExecutionStats() : executedInstructions(0) {}

// Dep remaps are in the form <remap count> <dependent index> <new dependency
// count> <new dependency ID in subprogram>. Returns <remaps, new offset>
static std::pair<std::unordered_map<int, std::vector<int>>, int>
parseDependencyRemappings(std::vector<Value> &args) {
  int offset = 0;
  int count = std::get<int>(args[offset++].val);

  std::unordered_map<int, std::vector<int>> remaps;

  for (int i = 0; i < count; i++) {
    int dependentIndex = std::get<int>(args[offset++].val);

    int dependencyCount = std::get<int>(args[offset++].val);

    std::vector<int> newDependencies;
    for (int j = 0; j < dependencyCount; j++)
      newDependencies.push_back(std::get<int>(args[offset++].val));

    remaps[dependentIndex] = newDependencies;
  }

  return std::make_pair(remaps, offset);
}

static std::pair<std::unordered_map<int, std::vector<int>>, int>
parseArgumentRemappings(std::vector<Value> &args, int offset) {
  int count = std::get<int>(args[offset++].val);

  std::unordered_map<int, std::vector<int>> remaps;

  for (int i = 0; i < count; i++) {
    int dependentIndex = std::get<int>(args[offset++].val);

    int dependencyCount = std::get<int>(args[offset++].val);

    std::vector<int> newDependencies;
    for (int j = 0; j < dependencyCount; j++)
      newDependencies.push_back(std::get<int>(args[offset++].val));

    remaps[dependentIndex] = newDependencies;
  }

  return std::make_pair(remaps, offset);
}

static std::pair<std::vector<int>, int>
parseArgumentDeclarations(std::vector<Value> &args, int offset) {
  int count = std::get<int>(args[offset++].val);

  std::vector<int> declarations;

  for (int i = 0; i < count; i++) {
    int decIndex = std::get<int>(args[offset++].val);
    declarations.push_back(decIndex);
  }

  return std::make_pair(declarations, offset);
}

static std::pair<std::vector<int>, int>
parseReturnIdsFromBytecodeArgs(std::vector<Value> &args, int offset) {
  int count = std::get<int>(args[offset++].val);

  std::vector<int> returnIds;

  for (int i = 0; i < count; i++) {
    int returnId = std::get<int>(args[offset++].val);
    returnIds.push_back(returnId);
  }

  return std::make_pair(returnIds, offset);
}

static std::vector<int>
getCompletionInstructionIds(std::shared_ptr<Subprogram> program) {
  std::vector<int> ids;

  for (int i = 0; i < program->size(); i++) {
    auto &instr = program->at(i);
    bool hasInternalDependent = false;

    for (auto dep : instr.dependents) {
      if (!dep.disabled && dep.instr->program == program) {
        hasInternalDependent = true;
        break;
      }
    }

    if (instr.type == InstructionType::Call || !hasInternalDependent)
      ids.push_back(i);
  }

  return ids;
}

static std::runtime_error
invalidArgTypesError(Instruction &instr, ValueType left, ValueType right) {
  return std::runtime_error(
      std::format("Invalid arg types on instruction {}: {}, {}", instr.id,
                  (int)left, (int)right));
}

void Executor::updateDependency(InstrDependent dep,
                                std::shared_ptr<Value> result) {
  if (dep.disabled)
    return;

  if (dep.returnLatch && dep.returnLatch->exchange(true))
    return;

  // Don't re-set dep args
  if (dep.argIndex.has_value() &&
      dep.instr->depArgs.size() > dep.argIndex.value() &&
      dep.instr->depArgs[dep.argIndex.value()].get() != nullptr)
    return;

  if (dep.argIndex.has_value()) {
    std::lock_guard<std::mutex> argLock(depArgsMutexes[dep.instr->id]);

    auto &depVec = dep.instr->depArgs;
    int i = dep.argIndex.value();

    // Ensure vector has an index i
    if (depVec.size() < i + 1)
      depVec.resize(i + 1);

    depVec[i] = result;
  }

  // Update fulfilled after setting args so other threads don't trigger
  int fulfilled;

  // Smaller scope so the lock guard is cleaned up
  {
    std::lock_guard<std::mutex> fulfilledLock(
        depsFulfilledMutexes[dep.instr->id]);

    dep.instr->depsFulfilled++;
    fulfilled = dep.instr->depsFulfilled;
  }

  if (fulfilled == dep.instr->depCount) {
    queue.push(*dep.instr);
  }
}

void Executor::skipInstruction(Instruction &instr) {
  if (cliArgs.verbose)
    log(LOCATION, "Skipping instruction: {}", instr.toString());

  instr.depCount = -1;

  int skipUntil = -1;
  if (instr.type == InstructionType::Block) {
    int toSkip = std::get<int>(instr.bytecodeArgs[0].val);
    skipUntil = instr.id + toSkip;

    // Set everything's depCount to -1, then skip everything
    for (int i = 1; i <= toSkip; i++) {
      // auto& not auto - don't forget the &!
      auto &skipped = instr.program->at(instr.id + i);
      skipped.depCount = -1;

      // Skipping the block skips its body, so don't skip again
      if (skipped.type == InstructionType::Block) {
        int blockSize = std::get<int>(skipped.bytecodeArgs[0].val);
        i += blockSize;
      }
    }

    for (int i = 1; i <= toSkip; i++) {
      // auto& not auto - don't forget the &!
      auto &skipped = instr.program->at(instr.id + i);
      skipInstruction(skipped);

      // Skipping the block skips its body, so don't skip again
      if (skipped.type == InstructionType::Block) {
        int blockSize = std::get<int>(skipped.bytecodeArgs[0].val);
        i += blockSize;
      }
    }
  }

  // Skip all deps first, then update so we don't accidently push onto queue
  for (auto dep : instr.dependents) {
    // Deps with indices are intra-line deps and the entire line will be
    // skipped, so we don't have to worry about them

    if (skipUntil != -1 && dep.instr->id <= skipUntil)
      continue;

    if (!dep.argIndex.has_value()) {
      updateDependency(dep, nullptr);
    }
  }
}

void Executor::execSingleInstruction(Instruction &instr) {
  if (stats)
    stats->executedInstructions.fetch_add(1, std::memory_order_relaxed);

  if (cliArgs.verbose)
    log(LOCATION, "Executing instruction: {}", instr.toString());
  std::shared_ptr<Value> result;

  bool updateDeps = true;

  switch (instr.type) {
  case InstructionType::Block: {
    // Bytecode args are [block size, call count, call 1 offset, ..., call n
    // offset]

    // Create new scope
    std::shared_ptr<Scope<Value>> scope = std::make_shared<Scope<Value>>(
        instr.scope); // make_shared() uses the constructor
    int size = std::get<int>(instr.bytecodeArgs[0].val);

    int skip = 0;
    for (int i = 0; i < size; i++) {
      if (skip > 0) {
        skip--;
        continue;
      }

      auto &inner = instr.program->at(instr.id + i + 1);
      if (inner.type == InstructionType::Block) {
        skip = std::get<int>(inner.bytecodeArgs[0].val);
      }
      inner.scope = scope;
    }

    // Remap dependents to calls
    if (instr.bytecodeArgs.size() < 2)
      break;

    int callCount = std::get<int>(instr.bytecodeArgs[1].val);
    if (callCount == 0)
      break;

    std::vector<std::reference_wrapper<Instruction>> calls;

    for (int i = 0; i < callCount; i++) {
      int id = instr.id + std::get<int>(instr.bytecodeArgs[2 + i].val);
      calls.push_back(instr.program->at(id));
    }

    for (auto &dep : instr.dependents) {
      if (dep.disabled || !dep.argIndex)
        continue;

      for (auto &call : calls) {
        call.get().dependents.push_back(dep);
      }

      dep.disabled = true;
    }

    break;
  }
  case InstructionType::GetLiteral:
    result = std::make_shared<Value>(instr.bytecodeArgs[0]);
    break;
  case InstructionType::ReferenceIdentifier: {
    auto ptr =
        instr.scope->get(std::get<std::string>(instr.bytecodeArgs[0].val));
    if (!ptr)
      throw std::runtime_error(std::format(
          "Attempted to read identifier '{}', but it did not exist!",
          std::get<std::string>(instr.bytecodeArgs[0].val)));
    result = ptr;
    break;
  }
  case InstructionType::GetIdentifier: {
    auto ptr =
        instr.scope->get(std::get<std::string>(instr.bytecodeArgs[0].val));
    if (!ptr)
      throw std::runtime_error(std::format(
          "Attempted to read identifier '{}', but it did not exist!",
          std::get<std::string>(instr.bytecodeArgs[0].val)));
    result = std::make_shared<Value>(ptr->type, ptr->val);
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
          std::format("Invalid arg types on instruction {}: {}, {}", instr.id,
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
          std::format("Invalid arg types on instruction {}: {}, {}", instr.id,
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
          std::format("Invalid arg types on instruction {}: {}, {}", instr.id,
                      (int)left->type, (int)right->type));

    break;
  }
  case InstructionType::CompareEquals:
  case InstructionType::CompareNotEquals: {
    std::shared_ptr<Value> left = instr.depArgs[0], right = instr.depArgs[1];

    bool equal;
    if (left->type != right->type) {
      equal = false;
    } else if (left->type == ValueType::Integer) {
      equal = std::get<int>(left->val) == std::get<int>(right->val);
    } else if (left->type == ValueType::String) {
      equal =
          std::get<std::string>(left->val) == std::get<std::string>(right->val);
    } else if (left->type == ValueType::Bool) {
      equal = std::get<bool>(left->val) == std::get<bool>(right->val);
    } else {
      throw invalidArgTypesError(instr, left->type, right->type);
    }

    result = std::make_shared<Value>(
        ValueType::Bool,
        instr.type == InstructionType::CompareEquals ? equal : !equal);

    break;
  }
  case InstructionType::CompareLessThan:
  case InstructionType::CompareLessThanEquals:
  case InstructionType::CompareGreaterThan:
  case InstructionType::CompareGreaterThanEquals: {
    std::shared_ptr<Value> left = instr.depArgs[0], right = instr.depArgs[1];

    if (left->type != right->type)
      throw invalidArgTypesError(instr, left->type, right->type);

    bool comparison;
    if (left->type == ValueType::Integer) {
      int leftVal = std::get<int>(left->val);
      int rightVal = std::get<int>(right->val);

      if (instr.type == InstructionType::CompareLessThan)
        comparison = leftVal < rightVal;
      else if (instr.type == InstructionType::CompareLessThanEquals)
        comparison = leftVal <= rightVal;
      else if (instr.type == InstructionType::CompareGreaterThan)
        comparison = leftVal > rightVal;
      else
        comparison = leftVal >= rightVal;
    } else if (left->type == ValueType::String) {
      auto leftVal = std::get<std::string>(left->val);
      auto rightVal = std::get<std::string>(right->val);

      if (instr.type == InstructionType::CompareLessThan)
        comparison = leftVal < rightVal;
      else if (instr.type == InstructionType::CompareLessThanEquals)
        comparison = leftVal <= rightVal;
      else if (instr.type == InstructionType::CompareGreaterThan)
        comparison = leftVal > rightVal;
      else
        comparison = leftVal >= rightVal;
    } else {
      throw invalidArgTypesError(instr, left->type, right->type);
    }

    result = std::make_shared<Value>(ValueType::Bool, comparison);

    break;
  }
  case InstructionType::If: {
    bool condition = valToBool(*instr.depArgs[0]);
    result = instr.depArgs[0];
    if (condition)
      break;

    // Skip next instruction
    skipInstruction(instr.program->at(instr.id + 1));
    break;
  }
  case InstructionType::Else: {
    bool condition = valToBool(*instr.depArgs[0]);
    if (!condition)
      break;

    // Skip next instruction
    skipInstruction(instr.program->at(instr.id + 1));
    break;
  }
  case InstructionType::While: {
    bool condition = valToBool(*instr.depArgs[0]);
    if (condition) {
      // Only update if condition is false, as we won't loop back
      updateDeps = false;

      // Manually update block
      for (auto dep : instr.dependents) {
        // Block always comes immediately after
        if (dep.instr->id == instr.id + 1)
          updateDependency(dep, result);
      }
      break;
    }

    // Skip next instruction
    skipInstruction(instr.program->at(instr.id + 1));
    break;
  }
  case InstructionType::GoTo: {
    int dist = std::get<int>(instr.bytecodeArgs[0].val);

    if (dist > 0)
      throw std::runtime_error(
          "Using GoTo with a positive distance is not supported!");

    int returnTo = instr.id + dist;

    for (int i = returnTo; i <= instr.id; i++) {
      instr.program->at(i).depArgs.clear();
      for (auto dep : instr.program->at(i).dependents) {
        if (dep.instr->id > returnTo && dep.instr->id <= instr.id)
          dep.instr->depsFulfilled--;
      }
    }

    for (int i = returnTo; i <= instr.id; i++) {
      auto &loopInstr = instr.program->at(i);
      if (loopInstr.depsFulfilled == loopInstr.depCount)
        queue.push(loopInstr);
    }

    break;
  }
  case InstructionType::BranchMerge:
    break;
  case InstructionType::Print: {
    result = instr.depArgs[0];

    // Don't use << in cout since it's only atomic at the level of individual
    // <<'s
    std::string str = valToStr(*result) + "\n";
    std::unique_lock<std::mutex> lock(coutMutex);
    std::cout << str;
    break;
  }
  case InstructionType::Function: {
    auto func = std::make_shared<Function>(instr, *instr.program.get());
    instr.scope->alloc(func->getName(), {ValueType::Function, func});

    // Skip body
    skipInstruction(instr.program->at(instr.id + 1));

    break;
  }
  case InstructionType::Call: {
    updateDeps = false;

    auto func = std::get<std::shared_ptr<Function>>(instr.depArgs[0]->val);
    auto body = func->getBody().clone();

    if (cliArgs.verbose) {
      log(LOCATION, "Calling function '{}' with {} instructions",
          func->getName(), body->size());
    }

    auto &block = body->at(0);
    block.scope = std::make_shared<Scope<Value>>(block.scope);
    block.depsFulfilled++;

    // Handle dependency remapping
    auto res = parseDependencyRemappings(instr.bytecodeArgs);
    auto remaps = res.first;

    for (auto remap : remaps) {
      auto &dependent = instr.dependents[remap.first];

      for (auto dependencyIndex : remap.second)
        body->at(dependencyIndex).dependents.push_back(dependent);

      // Adjust depCount accordingly
      {
        std::lock_guard<std::mutex> fulfilledLock(
            depsFulfilledMutexes[dependent.instr->id]);
        dependent.instr->depCount += remap.second.size() - 1;
      }

      dependent.disabled = true;
    }

    // Remap arguments
    res = parseArgumentRemappings(instr.bytecodeArgs, res.second);
    remaps = res.first;

    for (auto remap : remaps) {
      auto &arg = instr.program->at(instr.id + remap.first);

      for (auto dependentIndex : remap.second) {
        arg.dependents.emplace_back(&body->at(dependentIndex));
        {
          std::lock_guard<std::mutex> fulfilledLock(
              depsFulfilledMutexes[body->at(dependentIndex).id]);
          body->at(dependentIndex)
              .depCount++; // Be sure to adjust depCount accordingly!
        }

        if (cliArgs.verbose)
          log(LOCATION, "Made {} depend on {}",
              body->at(dependentIndex).toString(), arg.toString());
      }
    }

    // Update arg declaration scopes
    auto argDecRes = parseArgumentDeclarations(instr.bytecodeArgs, res.second);
    auto argDeclarationIds = std::unordered_set<int>();
    for (auto argDecIndex : argDecRes.first) {
      argDeclarationIds.insert(instr.id + argDecIndex);
      instr.program->at(instr.id + argDecIndex).scope = block.scope;
    }

    // Remap subprogram return statements
    auto returnIdsRes =
        parseReturnIdsFromBytecodeArgs(instr.bytecodeArgs, argDecRes.second);
    auto returnIds = returnIdsRes.first;

    auto returnLatch = std::make_shared<std::atomic_bool>(false);
    for (auto &dep : instr.dependents) {
      if (dep.disabled || !dep.argIndex)
        continue;

      for (auto returnId : returnIds) {
        auto returnDep = dep;
        returnDep.returnLatch = returnLatch;
        body->at(returnId).dependents.push_back(returnDep);
      }

      dep.disabled = true;
    }

    for (int i = 1; i < returnIds.size(); i++) {
      auto &prevReturn = body->at(returnIds[i - 1]);
      auto &nextReturn = body->at(returnIds[i]);

      prevReturn.dependents.emplace_back(&nextReturn);
      {
        std::lock_guard<std::mutex> fulfilledLock(
            depsFulfilledMutexes[nextReturn.id]);
        nextReturn.depCount++;
      }
    }

    auto completionIds = getCompletionInstructionIds(body);
    for (auto dep : instr.dependents) {
      if (dep.disabled)
        continue;

      if (dep.argIndex.has_value() ||
          argDeclarationIds.contains(dep.instr->id)) {
        updateDependency(dep, result);
        continue;
      }

      if (completionIds.empty()) {
        updateDependency(dep, result);
        continue;
      }

      {
        std::lock_guard<std::mutex> fulfilledLock(
            depsFulfilledMutexes[dep.instr->id]);
        dep.instr->depCount += completionIds.size() - 1;
      }
      for (auto completionId : completionIds)
        body->at(completionId).dependents.push_back(dep);
    }

    // Only push once we've relinked everything
    queue.push(block);

    break;
  }
  case InstructionType::Return: {
    result = instr.depArgs[0];
    break;
  }
  default:
    throw std::runtime_error(
        std::format("Unknown instruction type on instruction {}: {}", instr.id,
                    (int)instr.type));
  }

  if (updateDeps) {
    for (auto dep : instr.dependents) {
      updateDependency(dep, result);
    }
  }

  if (cliArgs.verbose)
    log(LOCATION, "[instruction {}]: {}", instr.id,
        result ? valToStr(*result) : "<no result>");
}

void Executor::execWorker(int id) {
  auto location = LOCATION + std::string(":worker") + std::to_string(id);
  if (cliArgs.verbose)
    log(location.c_str(), "Worker {} awake", id);

  while (!halt) {
    stalls[id].store(false);
    auto popped = queue.pop();

    // Check popped holds a nullptr_t
    if (std::holds_alternative<nullptr_t>(popped)) {
      stalls[id].store(true);
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      continue;
    }

    auto &instr = std::get<std::reference_wrapper<Instruction>>(popped).get();

    try {
      execSingleInstruction(instr);
    } catch (std::runtime_error err) {
      logError(location.c_str(), "[instruction {}] {}", instr.id, err.what());
      haltCause = std::format("Runtime Error in worker {} [instruction {}]: {}",
                              id, instr.id, err.what());
      failed = true;
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

    for (auto &stall : stalls) {
      if (!stall.load()) {
        isDone = false;
        break;
      }
    }

    if (isDone && queue.size() > 0)
      isDone = false;

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
  for (int i = 0; i < program.size(); i++) {
    auto &instr = program[i];
    if (instr.depsFulfilled == instr.depCount)
      queue.push(instr);
  }

  if (cliArgs.verbose)
    log(LOCATION, "Pushed {} instructions onto the queue.", queue.size());
}

void Executor::initScopes() {
  auto root = std::make_shared<Scope<Value>>();
  auto global = std::make_shared<Scope<Value>>(root);

  for (auto &instr : program) {
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
        program.size(), haltCause);

  if (failed)
    throw std::runtime_error(haltCause);
}

Executor::Executor(const CliArgs &cliArgs, Subprogram &program,
                   ExecutionStats *stats)
    : cliArgs(cliArgs), program(program), stats(stats),
      stalls(std::vector<std::atomic_bool>(cliArgs.threads)),
      depArgsMutexes(std::vector<std::mutex>(program.size())),
      depsFulfilledMutexes(std::vector<std::mutex>(program.size())),
      halt(false), failed(false), haltCause("Unknown") {
  for (auto &stall : stalls)
    stall.store(false);
}
