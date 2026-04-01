#pragma once

#include "../exitCode.hpp"
#include "../cliUtils.hpp"
#include <functional>
#include <string>
#include <optional>

ExitCode compile(const CliArgs &args, std::istream &inputStream, std::function<std::optional<std::string>(std::string)> writeOutput);