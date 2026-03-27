#include "astBuilder.hpp"

AstBuilder::AstBuilder(std::unique_ptr<std::vector<Token>> tokens)
{
  errors = std::make_unique<std::vector<SyntaxError>>();
  this->tokens = std::move(tokens);
  root = std::make_unique<BlockExpression>();
  line = 0;
  nextTokenIndex = 0;
}

Token AstBuilder::next()
{
  if (nextTokenIndex >= tokens.get()->size())
    throw "No more tokens!";

  auto token = tokens.get()->operator[](nextTokenIndex);

  line = token.line;

  nextTokenIndex++;
  return token;
}

Token AstBuilder::peek()
{
  if (nextTokenIndex >= tokens.get()->size())
    throw "No more tokens!";

  return tokens.get()->operator[](nextTokenIndex);
}

bool AstBuilder::lineEnding()
{
  return next().type == TokenType::Semicolon;
}

void AstBuilder::build()
{
}

void AstBuilder::syntaxError(std::string msg)
{
  errors.get()->push_back({line, msg});

  // Read until we hit semicolon to end the line
  while (!lineEnding())
    continue;
}

std::unique_ptr<std::vector<SyntaxError>> AstBuilder::getErrors()
{
  return std::move(errors);
}

std::unique_ptr<BlockExpression> AstBuilder::getRoot()
{
  return std::move(root);
}