#pragma once

#include "../exitCode.hpp"
#include "../cliUtils.hpp"
#include "../instruction.hpp"
#include "../concurrentQueue.hpp"
#include <vector>
#include <thread>

class Executor
{
  const CliArgs &cliArgs;
  std::vector<Instruction> &instructions;
  ConcurrentQueue<std::reference_wrapper<Instruction>> queue;
  std::vector<std::thread> workers;

  // Set to true to end workers
  bool halt;

  // Increment depsFulfilled and, if relevant, sets depArgs[i]
  void updateDependency(InstrDependent dep, Value result);
  void execSingleInstruction(Instruction &instr);

  // Multithreaded worker that actually executes instructions
  void execWorker(int id);
  // Waits until all instructions have been executed and then halts workers
  void supervisor();

  // Reads instructions and pushes everything that's ready onto the queue
  void initQueue();

public:
  Executor(const CliArgs &cliArgs, std::vector<Instruction> &instructions);
  void startExecution();
};