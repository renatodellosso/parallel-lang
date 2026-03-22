#pragma once

#include <string>

enum class CliMode
{
  Unset,
  Compile,
  Interpret
};

struct CommandLineArgs
{
  std::string target;
  CliMode mode;
};

CommandLineArgs parseArgs(int argc, char *argv[]);