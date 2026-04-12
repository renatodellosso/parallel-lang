#include "astBuilder.hpp"
#include "expression.hpp"
#include "token.hpp"
#include <format>
#include <memory>
#include <optional>

AstBuilder::AstBuilder(std::unique_ptr<std::vector<Token>> tokens) {
  errors = std::make_shared<std::vector<SyntaxError>>();
  this->tokens = std::move(tokens);
  root = std::make_shared<BlockExpression>();
  line = 0;
  nextTokenIndex = 0;
}

bool AstBuilder::hasNext() { return nextTokenIndex < tokens.get()->size(); }

Token AstBuilder::next() {
  if (!hasNext())
    throw std::runtime_error(
        "Attempted to next() when there are no more tokens!");

  auto token = tokens.get()->operator[](nextTokenIndex);

  line = token.line;

  nextTokenIndex++;
  return token;
}

Token AstBuilder::peek() {
  auto tokens = this->tokens.get();
  if (!hasNext())
    throw std::runtime_error(
        "Attempted to peek() when there are no more tokens!");

  return tokens->operator[](nextTokenIndex);
}

bool AstBuilder::match(TokenType type, std::optional<TokenSubtype> subtype) {
  if (!hasNext())
    return false;

  Token nextToken = peek();
  if (nextToken.type != type)
    return false;
  if (subtype.has_value() && nextToken.subtype != subtype.value())
    return false;
  return true;
}

std::optional<std::unique_ptr<Expression>>
AstBuilder::parseLeadingExpression() {
  // Specify RootExpression as type to make_unique to ensure it doesn't become
  // just an Expression
  if (match(TokenType::Identifier))
    return std::make_optional(std::make_unique<RootExpression>(
        RootExpression(InstructionType::GetIdentifier, line, next())));
  if (match(TokenType::Literal))
    return std::make_optional(std::make_unique<RootExpression>(
        RootExpression(InstructionType::GetLiteral, line, next())));
  if (match(TokenType::LeftBrace))
    return parseBlock();
  if (match(TokenType::If)) {
    next(); // Consume 'if'

    if (!match(TokenType::LeftParen))
      throw std::runtime_error(
          std::format("Expected '(' after 'if'. Got: '{}'", peek().raw));
    next(); // Consume '('

    auto condition = parseExpression(TokenType::RightParen);
    if (!condition.has_value())
      throw std::runtime_error(
          std::format("Expected condition in if statement!"));

    if (!match(TokenType::RightParen))
      throw std::runtime_error(
          std::format("Expected ')' after condition in if statement. Got: '{}'",
                      peek().raw));
    next(); // Consume '('

    return std::make_optional(std::make_unique<UnaryExpression>(UnaryExpression(
        InstructionType::If, line, std::move(condition.value()))));
  }

  throw std::runtime_error(std::format(
      "Could not parse line: No valid starting expression for token '{}'",
      peek().raw));
}

std::optional<std::unique_ptr<Expression>> AstBuilder::parseCompoundExpression(
    std::optional<std::unique_ptr<Expression>> prev, TokenType endOn) {
  InstructionType type;
  switch (peek().type) {
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
  case TokenType::Identifier: {
    // Special case with no middle token

    // Types are identifiers
    if (prev.value().get()->type != InstructionType::GetIdentifier)
      throw std::runtime_error(
          std::format("Expected type identifier before declaration. Previous "
                      "Expression: '{}', Current Token: '{}'",
                      prev.value().get()->toString(), peek().raw));

    // Parse name
    auto name = next();
    if (name.type != TokenType::Identifier)
      throw std::runtime_error(std::format(
          "Expected identifer when naming declaration. Got: '{}'", name.raw));
    // Use GetLiteral instead of ReferenceIdentifier to get the name so we can
    // allocate it
    RootExpression nameExpr(InstructionType::GetLiteral, line, name);

    return std::make_optional(std::make_unique<BinaryExpression>(
        BinaryExpression(InstructionType::Declare, line,
                         std::move(prev.value()),
                         std::make_shared<RootExpression>(nameExpr))));
  }
  case TokenType::Equals:
    type = InstructionType::Set;
    break;

  default:
    throw std::runtime_error(std::format(
        "Could not parse line: No matching instruction type for token '{}'",
        peek().raw));
  }

  next(); // Be sure to consume the token!

  // Parse right operand
  auto nextExpr = extendExpression(std::nullopt, TokenType::Semicolon);
  if (!nextExpr.has_value()) {
    throw std::runtime_error(std::format(
        "Could not parse line: Binary expression ('{}') has no right operand",
        peek().raw));
  }

  return std::make_optional(std::make_unique<BinaryExpression>(BinaryExpression(
      type, line, std::move(prev.value()), std::move(nextExpr.value()))));
}

std::optional<std::unique_ptr<BlockExpression>> AstBuilder::parseBlock() {
  BlockExpression block(line);

  next(); // Consume '{'

  while (!match(TokenType::RightBrace)) {
    auto expr = parseExpression(TokenType::Semicolon);
    if (!expr.has_value())
      break;

    if (hasNext() && match(TokenType::Semicolon))
      next(); // Consume semicolon

    // Use move to convert unique to shared (other way around doesn't work
    // though)
    block.expressions.push_back(std::move(expr.value()));
  }

  next(); // Consume '}'

  if (block.expressions.size())
    return std::make_optional(std::make_unique<BlockExpression>(block));
  return std::nullopt;
}

std::optional<std::unique_ptr<Expression>>
AstBuilder::extendExpression(std::optional<std::unique_ptr<Expression>> prev,
                             TokenType endOn) {
  if (match(endOn))
    return std::nullopt;

  if (!prev.has_value()) {
    // No previous value
    prev = parseLeadingExpression();

    if (!prev.has_value())
      return prev;
  }

  auto type = prev->get()->type;
  bool autoEndExpr =
      type == InstructionType::If || type == InstructionType::Block;
  if (!autoEndExpr && !match(endOn) && hasNext()) {
    prev = parseCompoundExpression(std::move(prev), endOn);
  }

  return prev;
}

std::optional<std::unique_ptr<Expression>>
AstBuilder::parseExpression(TokenType endOn) {
  try {
    std::optional<std::unique_ptr<Expression>> curr =
        extendExpression(std::nullopt, endOn);

    return curr;
  } catch (const std::runtime_error &e) {
    syntaxError(e.what());
    return std::nullopt;
  }
}

BlockExpression AstBuilder::buildRoot() {
  BlockExpression block;
  block.lineNumber = line;

  // Loop until we have no more expressions
  while (hasNext()) {
    auto expr = parseExpression(TokenType::Semicolon);
    if (expr.has_value())
      block.expressions.push_back(std::move(expr.value()));

    if (match(TokenType::Semicolon))
      next(); // Consume semicolon
  }

  return block;
}

void AstBuilder::build() {
  root = std::make_unique<BlockExpression>(buildRoot());
}

void AstBuilder::syntaxError(std::string msg) {
  errors.get()->push_back({line, msg});

  // Read until we hit semicolon to end the line
  while (hasNext() && !match(TokenType::Semicolon))
    next();
}

std::shared_ptr<std::vector<SyntaxError>> AstBuilder::getErrors() {
  return errors;
}

std::shared_ptr<BlockExpression> AstBuilder::getRoot() { return root; }