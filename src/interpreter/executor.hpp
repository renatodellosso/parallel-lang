#pragma once

#include "../cliUtils.hpp"
#include "../concurrentQueue.hpp"
#include "../instruction.hpp"
#include "subprogram.hpp"
#include <mutex>
#include <thread>
#include <vector>

class Executor {
  const CliArgs &cliArgs;
  Subprogram &program;
  ConcurrentQueue<std::reference_wrapper<Instruction>> queue;
  std::vector<std::thread> workers;
  std::vector<bool> stalls;

  // We have to put the mutexes in here since we can't move them
  std::vector<std::mutex> depArgsMutexes, depsFulfilledMutexes;

  std::mutex coutMutex;

  // Set to true to end workers
  bool halt;
  std::string haltCause;

  // Increment depsFulfilled and, if relevant, sets depArgs[i]
  void updateDependency(InstrDependent dep, std::shared_ptr<Value> result);
  // Use recurse = true at the root level for skipping blocks
  void skipInstruction(Instruction &instr, bool recurse = true);
  void execSingleInstruction(Instruction &instr);

  // Multithreaded worker that actually executes instructions
  void execWorker(int id);
  // Waits until all instructions have been executed and then halts workers
  void supervisor();

  // Reads instructions and pushes everything that's ready onto the queue
  void initQueue();
  void initScopes();

public:
  Executor(const CliArgs &cliArgs, Subprogram &program);
  void startExecution();
};