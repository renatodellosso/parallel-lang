#include "cli.hpp"
#include "logging.hpp"
#include <map>
#include <functional>
#include <filesystem>

const char *LOCATION = "CLI";

const std::unordered_map<std::string, std::string> shortcuts = {
    {"-t", "--target"},
    {"-c", "--compile"},
    {"-i", "--interpret"}};

#define OPTIONS_HANDLER_PARAMS CliArgs &args, int i, int count, char *argv[]

// key(options struct, current arg index, arg count, args) => extra args consumed
const std::unordered_map<std::string, std::function<int(CliArgs &, int, int, char *[])>>
    options = {{"--target", [](OPTIONS_HANDLER_PARAMS) -> int
                {
                  if (i == count - 1)
                  {
                    logError(LOCATION, "Missing value for --target");
                    return 0;
                  }

                  args.target = std::string(argv[i + 1]);

                  return 1;
                }},
               {"--compile", [](OPTIONS_HANDLER_PARAMS) -> int
                {
                  if (args.mode != CliMode::Unset)
                  {
                    logWarning(LOCATION, "CLI mode was set twice!");
                  }

                  args.mode = CliMode::Compile;

                  return 0;
                }},
               {"--interpret", [](OPTIONS_HANDLER_PARAMS) -> int
                {
                  if (args.mode != CliMode::Unset)
                  {
                    logWarning(LOCATION, "CLI mode was set twice!");
                  }

                  args.mode = CliMode::Interpret;

                  return 0;
                }}};

/**
 * If key is a shortcut, returns the expanded key. Otherwise, returns key.
 */
std::string resolveShortcuts(std::string key)
{
  auto shortcut = shortcuts.find(key);
  if (shortcut != shortcuts.end())
    return shortcut->second;
  return key;
}

CliArgs parseArgs(int argc, char *argv[])
{
  std::unordered_map<std::string, std::string> argMap;
  CliArgs args = {
      .mode = CliMode::Unset};

  // argv[0] is the executable path
  for (int i = 1; i < argc; i++)
  {
    std::string key = resolveShortcuts(std::string(argv[i]));

    auto handler = options.find(key);
    if (handler == options.end())
    {
      logError(LOCATION, "Unknown CLI argument: {:s}", key);
      continue;
    }

    int used = handler->second(args, i, argc, argv);
    i += used;
  }

  return args;
}

bool validateArgs(const CliArgs &args)
{
  bool valid = true;

  if (args.mode == CliMode::Unset)
  {
    logError(LOCATION, "CLI mode was not set to either compile or interpret!");
    valid = false;
  }

  if (args.target.length() == 0)
  {
    logError(LOCATION, "Target was not set!");
    valid = false;
  }

  if (!std::filesystem::exists(args.target))
  {
    logError(LOCATION, "Target file does not exist!");
    valid = false;
  }

  return valid;
}

ExitCode executeCommand(const CliArgs &args)
{
  return ExitCode::Ok;
}

ExitCode runCli(int argc, char *argv[])
{
  CliArgs args = parseArgs(argc, argv);

  if (!validateArgs(args))
  {
    logError(LOCATION, "CLI arguments are invalid!");
    return ExitCode::InvalidCli;
  }

  return executeCommand(args);
}