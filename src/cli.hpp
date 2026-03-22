#pragma once

#include "exitCode.hpp"
#include <string>

enum class CliMode
{
  Unset,
  Compile,
  Interpret
};

struct CliArgs
{
  std::string target;
  CliMode mode;
};

CliArgs parseArgs(int argc, char *argv[]);
/**
 * Returns true if the arguments are valid, false otherwise.
 */
bool validateArgs(const CliArgs &args);
/**
 * Returns process exit code
 */
ExitCode executeCommand(const CliArgs &args);
/**
 * Returns process exit code
 */
ExitCode runCli(int argc, char *argv[]);