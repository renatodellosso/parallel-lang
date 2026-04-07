#pragma once

#include <string>
#include <optional>

enum class CliMode
{
  Unset,
  Compile,
  Interpret
};

struct CliArgs
{
  std::string target;
  std::optional<std::string> outputFile;
  CliMode mode;
  bool verbose;
};