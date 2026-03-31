#include "compiler.hpp"
#include "tokenizer.hpp"
#include "astBuilder.hpp"
#include "../logging.hpp"
#include <fstream>

#define LOCATION "compiler"

ExitCode compile(const CliArgs &args)
{
  std::ifstream stream(args.target);
  Tokenizer *tokenizer = new Tokenizer(stream);

  tokenizer->parse();
  auto tokens = tokenizer->close();
  delete tokenizer;

  log(LOCATION, "Parsed {:d} tokens. Building AST...", tokens.get()->size());

  AstBuilder astBuilder(std::move(tokens));
  astBuilder.build();

  auto errors = astBuilder.getErrors();
  if (!errors->empty())
  {
    logError(LOCATION, "Found {} syntax errors", errors->size());
    for (auto error : *errors.get())
    {
      logError(LOCATION, "{}", error.toString());
    }

    return ExitCode::SyntaxErrors;
  }

  log(LOCATION, "Built AST");
  return ExitCode::Ok;
}