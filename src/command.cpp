#include "command.hpp"
#include "logging.hpp"
#include <map>
#include <functional>

const char *LOCATION = "CLI";

const std::unordered_map<std::string, std::string> shortcuts = {
    {"-t", "--target"}};

#define OPTIONS_HANDLER_PARAMS CommandLineArgs &args, int i, int count, char *argv[]

// key(options struct, current arg index, arg count, args) => void
const std::unordered_map<std::string, std::function<void(CommandLineArgs &, int, int, char *[])>> options = {
    {"--target", [](OPTIONS_HANDLER_PARAMS)
     {
       if (i == count - 1)
       {
         logError(LOCATION, "Missing value for --target");
         return;
       }

       args.target = std::string(argv[i + 1]);
     }}};

CommandLineArgs
parseArgs(int argc, char *argv[])
{
  std::unordered_map<std::string, std::string> argMap;

  // argv[0] is the executable path
  for (int i = 1; i < argc; i++)
  {
    char *key = argv[i];
  }

  CommandLineArgs args = {};
}