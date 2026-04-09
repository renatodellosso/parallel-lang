#pragma once

#include "../cliUtils.hpp"
#include "../exitCode.hpp"
#include <functional>
#include <optional>
#include <string>

ExitCode
compile(const CliArgs &args, std::istream &inputStream,
        std::function<std::optional<std::string>(std::string)> writeOutput);