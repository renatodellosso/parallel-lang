#include "astBuilder.hpp"
#include <format>

AstBuilder::AstBuilder(std::unique_ptr<std::vector<Token>> tokens)
{
  errors = std::make_shared<std::vector<SyntaxError>>();
  this->tokens = std::move(tokens);
  root = std::make_shared<BlockExpression>();
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
    throw std::runtime_error("Attempted to next() when there are no more tokens!");

  auto token = tokens.get()->operator[](nextTokenIndex);

  line = token.line;

  nextTokenIndex++;
  return token;
}

Token AstBuilder::peek()
{
  auto tokens = this->tokens.get();
  if (!hasNext())
    throw std::runtime_error("Attempted to peek() when there are no more tokens!");

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

std::optional<std::unique_ptr<Expression>> AstBuilder::parseLeadingExpression()
{ // Specify RootExpression as type to make_unique to ensure it doesn't become just an Expression
  if (match(TokenType::Identifier))
    return std::optional(std::make_unique<RootExpression>(RootExpression(InstructionType::GetIdentifier, line, next())));
  if (match(TokenType::Literal))
    return std::optional(std::make_unique<RootExpression>(RootExpression(InstructionType::GetLiteral, line, next())));

  throw std::runtime_error(std::format("Could not parse line: No valid starting expression for token '{}'", peek().raw));
}

std::optional<std::unique_ptr<Expression>> AstBuilder::parseCompoundExpression(std::optional<std::unique_ptr<Expression>> prev)
{
  InstructionType type;
  switch (peek().type)
  {
  case TokenType::Plus:
    type = InstructionType::Add;
    break;
  case TokenType::Minus:
    type = InstructionType::Subtract;
    break;
  case TokenType::Star:
    type = InstructionType::Multiply;
    break;
  case TokenType::Slash:
    type = InstructionType::Divide;
    break;
  case TokenType::Identifier:
  {
    // Special case with no middle token

    // Types are identifiers
    if (prev.value().get()->type != InstructionType::GetIdentifier)
      throw std::runtime_error(std::format("Expected type identifier before declaration. Previous Expression: '{}', Current Token: '{}'", prev.value().get()->toString(), peek().raw));

    // Parse name
    auto name = next();
    if (name.type != TokenType::Identifier)
      throw std::runtime_error(std::format("Expected identifer when naming declaration. Got: '{}'", name.raw));
    RootExpression nameExpr(InstructionType::GetIdentifier, line, name);

    return std::make_optional(
        std::make_unique<BinaryExpression>(BinaryExpression(InstructionType::Declare, line, std::move(prev.value()), std::make_shared<RootExpression>(nameExpr))));
  }
  case TokenType::Equals:
    type = InstructionType::Set;
    break;

  default:
    throw std::runtime_error(std::format("Could not parse line: No matching instruction type for token '{}'", peek().raw));
  }

  next(); // Be sure to consume the token!

  // Parse right operand
  auto nextExpr = extendExpression(std::nullopt);
  if (!nextExpr.has_value())
  {
    throw std::runtime_error(std::format("Could not parse line: Binary expression ('{}') has no right operand", peek().raw));
  }

  return std::make_optional(
      std::make_unique<BinaryExpression>(BinaryExpression(type, line, std::move(prev.value()), std::move(nextExpr.value()))));
}

std::optional<std::unique_ptr<Expression>> AstBuilder::extendExpression(std::optional<std::unique_ptr<Expression>> prev)
{
  if (!prev.has_value())
  {
    // No previous value
    return parseLeadingExpression();
  }

  // Binary operators
  if (!hasNext())
    throw std::runtime_error("Could not parse line: No matching instruction type as there is no next token");

  return parseCompoundExpression(std::move(prev));
}

std::optional<std::unique_ptr<Expression>> AstBuilder::buildLine()
{
  // Reset current expression
  std::optional<std::unique_ptr<Expression>> curr = std::nullopt;

  // Continually add onto expression
  try
  {
    while (!match(TokenType::Semicolon))
    {
      curr = extendExpression(std::move(curr));
      if (!curr.has_value())
        break;
    }

    next(); // Consume semicolon
  }
  catch (const std::runtime_error &e)
  {
    syntaxError(e.what());
    return std::nullopt;
  }

  return curr;
}

BlockExpression AstBuilder::buildBlock()
{
  BlockExpression block;
  block.lineNumber = line;

  // Loop until we have no more expressions
  while (hasNext())
  {
    auto expr = buildLine();
    if (expr.has_value())
      block.expressions.push_back(std::move(expr.value()));
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
  while (hasNext() && !match(TokenType::Semicolon))
    next();
}

std::shared_ptr<std::vector<SyntaxError>> AstBuilder::getErrors()
{
  return errors;
}

std::shared_ptr<BlockExpression> AstBuilder::getRoot()
{
  return root;
}