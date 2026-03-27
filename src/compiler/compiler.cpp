#include "compiler.hpp"
#include "tokenizer.hpp"
#include "astBuilder.hpp"
#include "../logging.hpp"
#include <fstream>

ExitCode compile(const CliArgs &args)
{
  std::ifstream stream(args.target);
  Tokenizer *tokenizer = new Tokenizer(stream);

  tokenizer->parse();
  auto tokens = tokenizer->close();
  delete tokenizer;

  log("Compiler", "Parsed {:d} tokens. Building AST...", tokens.get()->size());

  AstBuilder astBuilder(std::move(tokens));
  astBuilder.build();

  log("Compiler", "Built AST");
  return ExitCode::Ok;
}