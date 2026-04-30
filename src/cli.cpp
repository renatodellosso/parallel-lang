#include "cli.hpp"
#include "cliUtils.hpp"
#include "compiler/compiler.hpp"
#include "exitCode.hpp"
#include "interpreter/interpreter.hpp"
#include "logging.hpp"
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <optional>
#include <sstream>
#include <string>

const char *LOCATION = "CLI";

const std::unordered_map<std::string, std::string> shortcuts = {
    {"-t", "--target"}, {"-c", "--compile"}, {"-i", "--interpret"},
    {"-o", "--out"},    {"-e", "--verbose"}, {"-h", "--threads"}};

#define OPTIONS_HANDLER_PARAMS CliArgs &args, int i, int count, char *argv[]

// key(options struct, current arg index, arg count, args) => extra args
// consumed
const std::unordered_map<std::string,
                         std::function<int(CliArgs &, int, int, char *[])>>
    options = {
        {"--target",
         [](OPTIONS_HANDLER_PARAMS) -> int {
           if (i == count - 1) {
             logError(LOCATION, "Missing value for --target");
             return 0;
           }

           args.target = std::string(argv[i + 1]);

           return 1;
         }},
        {"--out",
         [](OPTIONS_HANDLER_PARAMS) -> int {
           if (i == count - 1) {
             logError(LOCATION, "Missing value for --out");
             return 0;
           }

           args.outputFile = std::make_optional(std::string(argv[i + 1]));

           return 1;
         }},
        {"--compile",
         [](OPTIONS_HANDLER_PARAMS) -> int {
           if (args.mode != CliMode::Unset) {
             logWarning(LOCATION, "CLI mode was set twice!");
           }

           args.mode = CliMode::Compile;

           return 0;
         }},
        {"--interpret",
         [](OPTIONS_HANDLER_PARAMS) -> int {
           if (args.mode != CliMode::Unset) {
             logWarning(LOCATION, "CLI mode was set twice!");
           }

           args.mode = CliMode::Interpret;

           return 0;
         }},
        {"--buildAndRun",
         [](OPTIONS_HANDLER_PARAMS) -> int {
           if (args.mode != CliMode::Unset) {
             logWarning(LOCATION, "CLI mode was set twice!");
           }

           args.mode = CliMode::CompileAndInterpret;

           return 0;
         }},
        {"--verbose",
         [](OPTIONS_HANDLER_PARAMS) -> int {
           args.verbose = true;

           return 0;
         }},
        {"--threads",
         [](OPTIONS_HANDLER_PARAMS) -> int {
           if (i == count - 1) {
             logError(LOCATION, "Missing value for --out");
             return 0;
           }

           args.threads = std::atoi(argv[i + 1]);

           return 1;
         }},
};

/**
 * If key is a shortcut, returns the expanded key. Otherwise, returns key.
 */
std::string resolveShortcuts(std::string key) {
  auto shortcut = shortcuts.find(key);
  if (shortcut != shortcuts.end())
    return shortcut->second;
  return key;
}

CliArgs parseArgs(int argc, char *argv[]) {
  std::unordered_map<std::string, std::string> argMap;
  CliArgs args = {
      .outputFile = std::nullopt, .mode = CliMode::Unset, .threads = 1};

  // argv[0] is the executable path
  for (int i = 1; i < argc; i++) {
    std::string key = resolveShortcuts(std::string(argv[i]));

    auto handler = options.find(key);
    if (handler == options.end()) {
      logError(LOCATION, "Unknown CLI argument: {:s}", key);
      continue;
    }

    int used = handler->second(args, i, argc, argv);
    i += used;
  }

  if (args.mode == CliMode::Unset)
    args.mode = CliMode::CompileAndInterpret;

  return args;
}

bool validateArgs(const CliArgs &args) {
  bool valid = true;

  if (args.mode == CliMode::Unset) {
    logError(LOCATION, "CLI mode was not set to either compile or interpret!");
    valid = false;
  }

  if (args.target.length() == 0) {
    logError(LOCATION, "Target was not set!");
    valid = false;
  }

  if (!std::filesystem::exists(args.target)) {
    logError(LOCATION, "Target file does not exist!");
    valid = false;
  }

  if (args.mode == CliMode::Compile && args.outputFile == std::nullopt) {
    logError(
        LOCATION,
        "Must specify an output file location using --out when compiling!");
    valid = false;
  }

  return valid;
}

std::optional<std::string> writeFile(const CliArgs &args, std::string text) {
  if (!args.outputFile.has_value())
    return std::make_optional("No output file specified!");

  std::ofstream file(args.outputFile.value());

  if (!file.is_open())
    return std::make_optional(
        std::format("Could not open file '{}'", args.outputFile.value()));

  file.clear();
  file << text;

  file.close();
  return std::nullopt;
}

ExitCode executeCommand(const CliArgs &args) {
  if (args.mode == CliMode::Compile) {
    std::ifstream stream(args.target);
    return compile(args, stream,
                   [args](std::string text) { return writeFile(args, text); });
  }

  if (args.mode == CliMode::Interpret) {
    std::ifstream stream(args.target);
    Interpreter interpreter(args);
    interpreter.interpret(stream);
  }

  if (args.mode == CliMode::CompileAndInterpret) {
    std::ifstream fileStream(args.target);
    std::string bytecode;

    std::function<std::optional<std::string>(std::string)> writeBytecode =
        [&bytecode](std::string text) {
          bytecode = text;
          return std::nullopt;
        };

    ExitCode exitCode = compile(args, fileStream, writeBytecode);
    fileStream.close();
    if (exitCode != ExitCode::Ok)
      return exitCode;

    std::istringstream bytecodeStream(bytecode);

    Interpreter *interpreter = new Interpreter(args);
    exitCode = interpreter->interpret(bytecodeStream);
    delete interpreter;

    return exitCode;
  }

  return ExitCode::InvalidCli;
}

ExitCode runCli(int argc, char *argv[]) {
  CliArgs args = parseArgs(argc, argv);

  if (!validateArgs(args)) {
    logError(LOCATION, "CLI arguments are invalid!");
    return ExitCode::InvalidCli;
  }

  return executeCommand(args);
}