#include "cli.hpp"
#include "logging.hpp"
#include <map>
#include <functional>

const char *LOCATION = "CLI";

const std::unordered_map<std::string, std::string> shortcuts = {
    {"-t", "--target"},
    {"-c", "--compile"},
    {"-i", "--interpret"}};

#define OPTIONS_HANDLER_PARAMS CommandLineArgs &args, int i, int count, char *argv[]

// key(options struct, current arg index, arg count, args) => extra args consumed
const std::unordered_map<std::string, std::function<int(CommandLineArgs &, int, int, char *[])>>
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

CommandLineArgs parseArgs(int argc, char *argv[])
{
  std::unordered_map<std::string, std::string> argMap;
  CommandLineArgs args = {
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