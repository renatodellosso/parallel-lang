// Generated with Codex

#include "../src/cliUtils.hpp"
#include "../src/compiler/compiler.hpp"
#include "../src/exitCode.hpp"
#include "../src/interpreter/bytecodeParser.hpp"
#include "../src/interpreter/executor.hpp"
#include "../src/interpreter/subprogram.hpp"
#include "../src/instruction.hpp"
#include "../src/utils.hpp"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

using Clock = std::chrono::steady_clock;
using Ns = std::chrono::nanoseconds;

const std::filesystem::path benchmarkRoot = "benchmark/benchmarks";

struct BenchmarkProgram {
  std::string group;
  std::string name;
  std::filesystem::path path;
};

struct BenchmarkOptions {
  int trials = 10;
  std::vector<int> threads = {1, 2, 4, 8, 16};
};

struct TrialResult {
  Ns compileTime = Ns::zero();
  Ns runTime = Ns::zero();
  std::uint64_t executedInstructions = 0;
};

struct ProgramSummary {
  Ns totalCompileTime = Ns::zero();
  Ns maxCompileTime = Ns::zero();
  Ns totalRunTime = Ns::zero();
  Ns maxRunTime = Ns::zero();
  Ns totalTime = Ns::zero();
  Ns maxTotalTime = Ns::zero();
  std::uint64_t executedInstructions = 0;
};

class CoutSilencer {
  std::ostringstream sink;
  std::streambuf *original;

public:
  CoutSilencer() : original(std::cout.rdbuf(sink.rdbuf())) {}
  ~CoutSilencer() { std::cout.rdbuf(original); }
};

[[noreturn]] void usageError(const std::string &message) {
  throw std::runtime_error(std::format(
      "{}\nUsage: benchmark [--trials N] [--threads LIST]\n"
      "  --trials N      Positive number of trials per program/thread count\n"
      "  --threads LIST  Comma-separated positive thread counts, e.g. 1,2,4,8",
      message));
}

int parsePositiveInt(const std::string &value, const std::string &optionName) {
  if (value.empty())
    usageError(std::format("{} must not be empty", optionName));

  std::size_t consumed = 0;
  int parsed = 0;
  try {
    parsed = std::stoi(value, &consumed);
  } catch (const std::exception &) {
    usageError(std::format("{} must be a positive integer", optionName));
  }

  if (consumed != value.size() || parsed <= 0)
    usageError(std::format("{} must be a positive integer", optionName));

  return parsed;
}

std::vector<int> parseThreadList(const std::string &value) {
  if (value.empty())
    usageError("--threads must not be empty");

  std::vector<int> threads;
  std::stringstream stream(value);
  std::string item;
  while (std::getline(stream, item, ',')) {
    threads.push_back(parsePositiveInt(item, "--threads"));
  }

  if (threads.empty())
    usageError("--threads must contain at least one value");

  return threads;
}

BenchmarkOptions parseOptions(int argc, char *argv[]) {
  BenchmarkOptions options;

  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];

    if (arg == "--trials") {
      if (i == argc - 1)
        usageError("--trials requires a value");
      options.trials = parsePositiveInt(argv[++i], "--trials");
    } else if (arg == "--threads") {
      if (i == argc - 1)
        usageError("--threads requires a value");
      options.threads = parseThreadList(argv[++i]);
    } else {
      usageError(std::format("Unknown option '{}'", arg));
    }
  }

  return options;
}

std::vector<BenchmarkProgram> discoverBenchmarks() {
  if (!std::filesystem::exists(benchmarkRoot)) {
    throw std::runtime_error(
        std::format("Benchmark folder '{}' does not exist",
                    benchmarkRoot.string()));
  }

  std::vector<BenchmarkProgram> benchmarks;
  for (const auto &groupEntry :
       std::filesystem::directory_iterator(benchmarkRoot)) {
    if (!groupEntry.is_directory())
      continue;

    std::string group = groupEntry.path().filename().string();
    for (const auto &programEntry :
         std::filesystem::directory_iterator(groupEntry.path())) {
      if (!programEntry.is_regular_file() ||
          programEntry.path().extension() != ".p")
        continue;

      benchmarks.push_back({
          .group = group,
          .name = programEntry.path().stem().string(),
          .path = programEntry.path(),
      });
    }
  }

  std::ranges::sort(benchmarks, [](const auto &left, const auto &right) {
    if (left.group != right.group)
      return left.group < right.group;
    return left.name < right.name;
  });

  if (benchmarks.empty()) {
    throw std::runtime_error(
        std::format("No .p benchmark programs found under '{}'",
                    benchmarkRoot.string()));
  }

  return benchmarks;
}

std::string compileProgram(const BenchmarkProgram &program, const CliArgs &args) {
  std::ifstream source(program.path);
  if (!source.is_open()) {
    throw std::runtime_error(
        std::format("Could not open benchmark source '{}'",
                    program.path.string()));
  }

  std::string bytecode;
  ExitCode exitCode = compile(args, source, [&bytecode](std::string text) {
    bytecode = std::move(text);
    return std::nullopt;
  });

  if (exitCode != ExitCode::Ok) {
    throw std::runtime_error(std::format(
        "Compilation failed for '{}' with exit code {}", program.path.string(),
        static_cast<int>(exitCode)));
  }

  return bytecode;
}

std::uint64_t runProgram(const std::string &bytecode, const CliArgs &args) {
  std::istringstream bytecodeStream(bytecode);
  std::vector<Instruction> instructions;
  BytecodeParser parser(args, instructions, bytecodeStream);
  parser.buildInstructions();

  auto instrs = std::make_shared<std::vector<Instruction>>(instructions);
  auto program = std::make_shared<Subprogram>(instrs);
  program->setSubprogramPointers(program);

  ExecutionStats stats;
  Executor executor(args, *program, &stats);

  CoutSilencer silencer;
  executor.startExecution();

  return stats.executedInstructions.load(std::memory_order_relaxed);
}

TrialResult runTrial(const BenchmarkProgram &program, int threads) {
  CliArgs args = {
      .target = program.path.string(),
      .outputFile = std::nullopt,
      .mode = CliMode::CompileAndInterpret,
      .verbose = false,
      .threads = threads,
  };

  auto compileStart = Clock::now();
  std::string bytecode = compileProgram(program, args);
  auto compileEnd = Clock::now();

  auto runStart = Clock::now();
  std::uint64_t executedInstructions = runProgram(bytecode, args);
  auto runEnd = Clock::now();

  return {
      .compileTime = std::chrono::duration_cast<Ns>(compileEnd - compileStart),
      .runTime = std::chrono::duration_cast<Ns>(runEnd - runStart),
      .executedInstructions = executedInstructions,
  };
}

std::string formatAverage(Ns total, int count) {
  return formatNs(total / count);
}

void printSummary(const BenchmarkProgram &program, int threads, int trials,
                  const ProgramSummary &summary) {
  std::cout << std::format(
      "  {:<24} threads={:<3} trials={:<4} "
      "compile_avg={:<12} compile_max={:<12} "
      "run_avg={:<12} run_max={:<12} "
      "total_avg={:<12} total_max={:<12} instructions={}\n",
      program.name, threads, trials, formatAverage(summary.totalCompileTime,
                                                   trials),
      formatNs(summary.maxCompileTime), formatAverage(summary.totalRunTime,
                                                      trials),
      formatNs(summary.maxRunTime), formatAverage(summary.totalTime, trials),
      formatNs(summary.maxTotalTime), summary.executedInstructions);
}

} // namespace

int main(int argc, char *argv[]) {
  try {
    BenchmarkOptions options = parseOptions(argc, argv);
    std::vector<BenchmarkProgram> benchmarks = discoverBenchmarks();

    std::uint64_t aggregateInstructions = 0;
    Ns aggregateRunTime = Ns::zero();
    std::string currentGroup;

    std::cout << std::format("Benchmark root: {}\n", benchmarkRoot.string());
    std::cout << std::format("Trials: {}\n", options.trials);
    std::cout << "Threads:";
    for (auto threads : options.threads)
      std::cout << " " << threads;
    std::cout << "\n\n";

    for (const auto &program : benchmarks) {
      if (program.group != currentGroup) {
        currentGroup = program.group;
        std::cout << std::format("{}\n", currentGroup);
      }

      for (auto threads : options.threads) {
        ProgramSummary summary;

        for (int trial = 0; trial < options.trials; trial++) {
          TrialResult result = runTrial(program, threads);

          summary.totalCompileTime += result.compileTime;
          summary.maxCompileTime = std::max(summary.maxCompileTime,
                                            result.compileTime);
          summary.totalRunTime += result.runTime;
          summary.maxRunTime = std::max(summary.maxRunTime, result.runTime);
          Ns totalTime = result.compileTime + result.runTime;
          summary.totalTime += totalTime;
          summary.maxTotalTime = std::max(summary.maxTotalTime, totalTime);
          summary.executedInstructions += result.executedInstructions;
        }

        aggregateInstructions += summary.executedInstructions;
        aggregateRunTime += summary.totalRunTime;
        printSummary(program, threads, options.trials, summary);
      }
    }

    if (aggregateInstructions == 0) {
      throw std::runtime_error(
          "No bytecode instructions were executed by the benchmarks");
    }

    double nsPerInstruction =
        static_cast<double>(aggregateRunTime.count()) /
        static_cast<double>(aggregateInstructions);

    std::cout << std::format(
        "\ntime_per_bytecode_instruction={:.3f}ns "
        "total_run_time={} total_executed_instructions={}\n",
        nsPerInstruction, formatNs(aggregateRunTime), aggregateInstructions);

    return 0;
  } catch (const std::exception &err) {
    std::cerr << "benchmark: " << err.what() << "\n";
    return 1;
  }
}
