#pragma once

#include <optional>
#include <string>

enum class CliMode { Unset, Compile, Interpret, CompileAndInterpret };

struct CliArgs {
  std::string target;
  std::optional<std::string> outputFile;
  CliMode mode;
  bool verbose;
  int threads;
};