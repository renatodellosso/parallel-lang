#include "compiler.hpp"
#include "../logging.hpp"
#include "astBuilder.hpp"
#include "graphLinker.hpp"
#include "tokenizer.hpp"

#define LOCATION "compiler"

ExitCode
compile(const CliArgs &args, std::istream &inputStream,
        std::function<std::optional<std::string>(std::string)> writeOutput) {
  Tokenizer *tokenizer = new Tokenizer(inputStream);

  tokenizer->parse();
  auto tokens = tokenizer->close();
  delete tokenizer;

  log(LOCATION, "Parsed {:d} tokens. Building AST...", tokens.get()->size());

  AstBuilder astBuilder(std::move(tokens));
  astBuilder.build();

  auto errors = astBuilder.getErrors();
  if (!errors->empty()) {
    logError(LOCATION, "Found {} syntax errors while building AST",
             errors->size());
    for (auto error : *errors.get()) {
      logError(LOCATION, "{}", error.toString());
    }

    return ExitCode::SyntaxErrors;
  }

  log(LOCATION, "Built AST");

  astBuilder.getRoot().get()->numberExpressions(0);
  log(LOCATION, "Numbered expressions");

  GraphLinker graphLinker(astBuilder.getRoot());
  graphLinker.linkGraph();

  errors = graphLinker.getErrors();
  if (!errors->empty()) {
    logError(LOCATION, "Found {} syntax errors while linking graph",
             errors->size());
    for (auto error : *errors.get()) {
      logError(LOCATION, "{}", error.toString());
    }

    return ExitCode::SyntaxErrors;
  }

  log(LOCATION, "Linked graph");

  std::string bytecode = "";
  for (auto expr : astBuilder.getRoot()->expressions) {
    bytecode += expr->toByteCode() + "\n";
  }
  bytecode = bytecode.substr(0, bytecode.length() - 1); // Remove trailing '\n'
  auto result = writeOutput(bytecode);

  if (result.has_value()) {
    logError(LOCATION, "Failed to write bytecode to file: {}", result.value());
    return ExitCode::FailedToWriteFile;
  }

  log(LOCATION, "Wrote bytecode to file!");
  return ExitCode::Ok;
}