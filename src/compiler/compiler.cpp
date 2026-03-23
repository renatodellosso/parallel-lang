#include "compiler.hpp"
#include "tokenizer.hpp"
#include "../logging.hpp"
#include <fstream>

ExitCode compile(const CliArgs &args)
{
  std::ifstream stream(args.target);
  Tokenizer *tokenizer = new Tokenizer(stream);

  tokenizer->parse();
  auto tokens = tokenizer->close();
  delete tokenizer;

  log("Compiler", "Compiled {:d} tokens", tokens.get()->size());
  return ExitCode::Ok;
}