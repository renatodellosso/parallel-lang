#pragma once

#include <string>

struct CommandLineArgs
{
  std::string target;
};

CommandLineArgs parseArgs(int argc, char *argv[]);