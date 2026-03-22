#pragma once

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