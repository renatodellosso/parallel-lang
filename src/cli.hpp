#pragma once

#include "exitCode.hpp"
#include "cliUtils.hpp"

CliArgs parseArgs(int argc, char *argv[]);
/**
 * Returns true if the arguments are valid, false otherwise.
 */
bool validateArgs(const CliArgs &args);
ExitCode executeCommand(const CliArgs &args);
ExitCode runCli(int argc, char *argv[]);