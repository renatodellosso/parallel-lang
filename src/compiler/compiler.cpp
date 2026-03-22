#include "compiler.hpp"
#include "tokenizer.hpp"
#include <fstream>

ExitCode compile(const CliArgs &args)
{
  std::ifstream stream(args.target);
  // auto tokens = tokenize(std::move(stream));
}