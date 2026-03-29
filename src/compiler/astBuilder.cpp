#include "astBuilder.hpp"

AstBuilder::AstBuilder(std::unique_ptr<std::vector<Token>> tokens)
{
  errors = std::make_unique<std::vector<SyntaxError>>();
  this->tokens = std::move(tokens);
  root = std::make_unique<BlockExpression>();
  line = 0;
  nextTokenIndex = 0;
}

bool AstBuilder::hasNext()
{
  return nextTokenIndex < tokens.get()->size();
}

Token AstBuilder::next()
{
  if (!hasNext())
    throw "No more tokens!";

  auto token = tokens.get()->operator[](nextTokenIndex);

  line = token.line;

  nextTokenIndex++;
  return token;
}

Token AstBuilder::peek()
{
  auto tokens = this->tokens.get();
  if (!hasNext())
    throw "No more tokens!";

  return tokens->operator[](nextTokenIndex);
}

bool AstBuilder::match(TokenType type, std::optional<TokenSubtype> subtype)
{
  if (!hasNext())
    return false;

  Token nextToken = peek();
  if (nextToken.type != type)
    return false;
  if (subtype.has_value() && nextToken.subtype != subtype.value())
    return false;
  return true;
}

std::optional<Expression> AstBuilder::buildExpression(std::optional<Expression> prev)
{
  
}

std::optional<Expression> AstBuilder::buildLine()
{
  // Reset current expression
  auto curr = std::optional<Expression>();

  // Continually add onto expression
  while (!match(TokenType::Semicolon))
  {
    curr = buildExpression(curr);
  }

  return curr;
}

BlockExpression AstBuilder::buildBlock()
{
  BlockExpression block;

  // Loop until we have no more expressions
  while (hasNext())
  {
    auto expr = buildLine();
    if (expr.has_value())
      block.expressions.push_back(expr.value());
  }

  return block;
}

void AstBuilder::build()
{
  root = std::make_unique<BlockExpression>(buildBlock());
}

void AstBuilder::syntaxError(std::string msg)
{
  errors.get()->push_back({line, msg});

  // Read until we hit semicolon to end the line
  while (hasNext() && next().type != TokenType::Semicolon)
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