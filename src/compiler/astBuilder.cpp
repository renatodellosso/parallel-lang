#include "astBuilder.hpp"
#include "expression.hpp"
#include "token.hpp"
#include <format>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

TokenFilter::TokenFilter(TokenType type, std::optional<TokenSubtype> subtype)
    : type(type), subtype(subtype) {}

bool TokenFilter::match(Token token) {
  if (token.type != type)
    return false;

  if (subtype.has_value() && token.subtype != subtype.value())
    return false;

  return true;
}

AstBuilder::AstBuilder(std::unique_ptr<std::vector<Token>> tokens) {
  errors = std::make_shared<std::vector<SyntaxError>>();
  this->tokens = std::move(tokens);
  expressions = std::make_shared<std::vector<std::shared_ptr<Expression>>>(
      std::vector<std::shared_ptr<Expression>>());
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

bool AstBuilder::match(std::initializer_list<TokenFilter> filters) {
  if (!hasNext())
    return false;

  if (filters.size() == 0)
    return true;

  auto token = peek();
  for (auto filter : filters) {
    if (filter.match(token))
      return true;
  }

  return false;
}

bool AstBuilder::match(TokenFilter filter) { return match({filter}); }

bool AstBuilder::match(TokenType type, std::optional<TokenSubtype> subtype) {
  return match(TokenFilter(type, subtype));
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
  if (match(TokenType::Return)) {
    next(); // Be sure to consume the leading token itself!
    auto output = parseExpression({TokenType::Semicolon});
    return std::make_optional(std::make_unique<UnaryExpression>(
        InstructionType::Return, line, std::move(output.value())));
  }
  if (match(TokenType::If))
    return parseIf();
  if (match(TokenType::Else))
    throw std::runtime_error("Cannot have an else statement without an if!");
  if (match(TokenType::While)) {
    next(); // Consume 'while'

    if (!match(TokenType::LeftParen))
      throw std::runtime_error(
          std::format("Expected '(' after 'while'. Got: '{}'", peek().raw));
    next(); // Consume '('

    auto condition = parseExpression({TokenType::RightParen});
    if (!condition.has_value())
      throw std::runtime_error(
          std::format("Expected condition in while statement!"));

    if (!match(TokenType::RightParen))
      throw std::runtime_error(std::format(
          "Expected ')' after condition in while statement. Got: '{}'",
          peek().raw));
    next(); // Consume ')'

    return std::make_optional(std::make_unique<UnaryExpression>(
        InstructionType::While, line, std::move(condition.value())));
  }
  if (match(TokenType::Print)) {
    next(); // Be sure to consume the leading token itself!
    auto output = parseExpression({TokenType::Semicolon});
    return std::make_optional(std::make_unique<UnaryExpression>(
        InstructionType::Print, line, std::move(output.value())));
  }

  throw std::runtime_error(std::format(
      "Could not parse line: No valid starting expression for token '{}'",
      peek().raw));
}

std::optional<std::unique_ptr<Expression>> AstBuilder::parseCompoundExpression(
    std::optional<std::unique_ptr<Expression>> prev,
    std::initializer_list<TokenFilter> endOn) {
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
  case TokenType::EqualsEquals:
    type = InstructionType::CompareEquals;
    break;
  case TokenType::NotEquals:
    type = InstructionType::CompareNotEquals;
    break;
  case TokenType::LessThan:
    type = InstructionType::CompareLessThan;
    break;
  case TokenType::LessThanEquals:
    type = InstructionType::CompareLessThanEquals;
    break;
  case TokenType::GreaterThan:
    type = InstructionType::CompareGreaterThan;
    break;
  case TokenType::GreaterThanEquals:
    type = InstructionType::CompareGreaterThanEquals;
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

    auto declare =
        std::make_optional(std::make_unique<BinaryExpression>(BinaryExpression(
            InstructionType::Declare, line, std::move(prev.value()),
            std::make_shared<RootExpression>(nameExpr))));

    if (!match(TokenType::Equals)) {
      if (match(TokenType::LeftParen))
        return parseFunction(std::move(declare.value()));
      return declare;
    }

    prev = std::make_optional(std::unique_ptr<Expression>(
        static_cast<Expression *>(declare->release())));
    type = InstructionType::Set;
    break;
  }
  case TokenType::Equals:
    type = InstructionType::Set;
    prev.value()->type = InstructionType::ReferenceIdentifier;
    break;
  case TokenType::LeftParen: {
    // Function call!

    next(); // Consume '('

    if (!prev.has_value())
      throw std::runtime_error(
          "Attempted to call a function without providing an identifier!");
    if (prev.value()->type != InstructionType::GetIdentifier)
      throw std::runtime_error(std::format(
          "Cannot call a function without an identifier. Got '{}' as previous "
          "instead of identifer",
          prev.value()->toString()));

    // Check cast to verify it's not a function declaration
    auto casted = dynamic_cast<RootExpression *>(prev.value().get());
    if (casted) {

      auto call = std::make_unique<CallExpression>(
          std::move(prev.value()),
          line); // Can't use prev->lineNumber since it's been released

      while (!match(TokenType::RightParen)) {
        // Parse arguments
        auto arg = parseExpression({TokenType::Comma, TokenType::RightParen});

        if (!arg.has_value())
          break;

        if (match(TokenType::Comma))
          next();

        call->addArgument(std::move(arg.value()));
      }

      next(); // Consume ')'

      // We return early, so we have to handle extending manually
      auto final = std::make_optional(std::move(call));
      if (!match(endOn))
        return extendExpression(std::move(final), endOn);
      return final;
    }
  }
  default:
    throw std::runtime_error(std::format(
        "Could not parse line: No matching instruction type for token '{}'",
        peek().raw));
  }

  next(); // Be sure to consume the token!

  // Parse right operand
  auto nextExpr = extendExpression(std::nullopt, endOn);
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

  while (hasNext() && !match(TokenType::RightBrace)) {
    auto expr = parseExpression({TokenType::Semicolon});
    if (!expr.has_value())
      break;

    if (hasNext() && match(TokenType::Semicolon))
      next(); // Consume semicolon

    // Use move to convert unique to shared (other way around doesn't work
    // though)
    block.expressions.push_back(std::move(expr.value()));
  }

  if (!hasNext())
    throw std::runtime_error("Expected '}' after block!");

  next(); // Consume '}'

  return std::make_optional(std::make_unique<BlockExpression>(block));
}

FunctionExprParameter AstBuilder::parseFuncParam() {
  if (!match(TokenType::Identifier)) {
    auto n = next();
    throw std::runtime_error(std::format(
        "Expected parameter type, but got {} (type {})!", n.raw, (int)n.type));
  }
  auto type = next().raw;

  if (!match(TokenType::Identifier)) {
    auto n = next();
    throw std::runtime_error(std::format(
        "Expected parameter name, but got {} (type {})!", n.raw, (int)n.type));
  }
  auto name = next().raw;

  return {type, name};
}

std::optional<std::unique_ptr<FunctionExpression>>
AstBuilder::parseFunction(std::unique_ptr<BinaryExpression> declaration) {
  next(); // Consume '('

  auto returnType =
      std::static_pointer_cast<RootExpression>(declaration->left)->token.raw;
  auto name =
      std::static_pointer_cast<RootExpression>(declaration->right)->token.raw;

  auto func = std::make_unique<FunctionExpression>(name, returnType,
                                                   declaration->lineNumber);

  // Parse parameters
  while (!match(TokenType::RightParen)) {
    func->params.push_back(parseFuncParam());
    if (match(TokenType::Comma))
      next(); // Consume ','
  }

  next(); // Consume ')'

  return std::make_optional(std::move(func));
}

std::optional<std::unique_ptr<BlockExpression>>
AstBuilder::parseStatementBlock(std::string context) {
  if (!hasNext())
    throw std::runtime_error(
        std::format("Expected body after {} statement!", context));

  if (match(TokenType::LeftBrace))
    return parseBlock();

  if (match({TokenType::Else, TokenType::RightBrace, TokenType::Semicolon}))
    throw std::runtime_error(std::format(
        "Expected body after {} statement. Got: '{}'", context, peek().raw));

  auto expr = parseExpression({TokenType::Semicolon});
  if (!expr.has_value())
    throw std::runtime_error(
        std::format("Expected body after {} statement!", context));

  if (hasNext() && match(TokenType::Semicolon))
    next();

  auto block = std::make_unique<BlockExpression>(expr.value()->lineNumber);
  block->expressions.push_back(std::move(expr.value()));

  return std::make_optional(std::move(block));
}

std::optional<std::unique_ptr<IfExpression>> AstBuilder::parseIf() {
  int ifLine = next().line; // Consume 'if'

  if (!match(TokenType::LeftParen))
    throw std::runtime_error(
        std::format("Expected '(' after 'if'. Got: '{}'", peek().raw));
  next(); // Consume '('

  auto condition = parseExpression({TokenType::RightParen});
  if (!condition.has_value())
    throw std::runtime_error(
        std::format("Expected condition in if statement!"));

  if (!match(TokenType::RightParen))
    throw std::runtime_error(std::format(
        "Expected ')' after condition in if statement. Got: '{}'", peek().raw));
  next(); // Consume ')'

  auto thenBlock = parseStatementBlock("if");
  if (!thenBlock.has_value())
    throw std::runtime_error("Expected body after if statement!");

  std::shared_ptr<BlockExpression> elseBlock = nullptr;
  if (hasNext() && match(TokenType::Else)) {
    next(); // Consume 'else'

    auto parsedElse = parseStatementBlock("else");
    if (!parsedElse.has_value())
      throw std::runtime_error("Expected body after else statement!");

    elseBlock = std::move(parsedElse.value());
  }

  return std::make_optional(
      std::make_unique<IfExpression>(ifLine, std::move(condition.value()),
                                     std::move(thenBlock.value()), elseBlock));
}

std::optional<std::unique_ptr<Expression>>
AstBuilder::extendExpression(std::optional<std::unique_ptr<Expression>> prev,
                             std::initializer_list<TokenFilter> endOn) {
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
      type == InstructionType::If || type == InstructionType::While ||
      (type == InstructionType::Block &&
       dynamic_cast<CallExpression *>(prev.value().get()) == nullptr) ||
      type == InstructionType::Function;
  if (!autoEndExpr && !match(endOn) && hasNext()) {
    prev = parseCompoundExpression(std::move(prev), endOn);
  }

  return prev;
}

std::optional<std::unique_ptr<Expression>>
AstBuilder::parseExpression(std::initializer_list<TokenFilter> endOn) {
  try {
    std::optional<std::unique_ptr<Expression>> curr =
        extendExpression(std::nullopt, endOn);

    return curr;
  } catch (const std::runtime_error &e) {
    syntaxError(e.what());
    return std::nullopt;
  }
}

// Convert "while, body, goto" into "function, while, call, goto"
void AstBuilder::postProcessWhileLoop(
    std::vector<std::shared_ptr<Expression>> *expressions, int i,
    std::shared_ptr<Expression> loop) {
  auto body = std::static_pointer_cast<BlockExpression>(expressions->at(i + 1));

  // Add goto
  Token jumpToken = {TokenType::Literal, TokenSubtype::Integer,
                     std::to_string(-1), loop->lineNumber};
  auto jump = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GoTo, loop->lineNumber, jumpToken));
  jump->dependentRedirect = loop.get();

  auto origLoop = std::make_shared<UnaryExpression>(
      *static_cast<UnaryExpression *>(loop.get()));

  // Make function
  auto func =
      std::make_unique<FunctionExpression>("_body", "void", loop->lineNumber);

  // Make call
  auto funcName = std::make_unique<RootExpression>(
      InstructionType::GetIdentifier, loop->lineNumber,
      Token(TokenType::Identifier, TokenSubtype::None, "_body",
            loop->lineNumber));
  auto call =
      std::make_unique<CallExpression>(std::move(funcName), loop->lineNumber);

  auto innerBlock = std::make_unique<BlockExpression>(
      std::vector<std::shared_ptr<Expression>>{std::move(call), jump},
      loop->lineNumber);

  // Update jump distance
  std::static_pointer_cast<RootExpression>(jump)->token.raw = std::to_string(
      -1 * (loop->countInstructions() + innerBlock->countInstructions() - 1));

  // Make outer block
  auto block = std::make_unique<BlockExpression>(loop->lineNumber);
  block->expressions.push_back(std::move(func));
  block->expressions.push_back(body); // func->body will be set in postProcess
  block->expressions.push_back(std::make_shared<UnaryExpression>(
      *static_cast<UnaryExpression *>(loop.get())));
  block->expressions.push_back(std::move(innerBlock));

  // Replace the expression at loop with block
  (*expressions)[i] = std::move(block);

  // Can't use block anymore
  postProcess(&std::static_pointer_cast<BlockExpression>(expressions->at(i))
                   ->expressions);
}

void AstBuilder::postProcess(
    std::vector<std::shared_ptr<Expression>> *expressions) {
  for (int i = 0; i < expressions->size(); i++) {
    auto &expr = *expressions->at(i).get();

    if (expr.postprocessed)
      continue;

    expr.postprocessed = true;

    auto ifExpr = dynamic_cast<IfExpression *>(&expr);
    if (ifExpr) {
      postProcess(&ifExpr->thenBlock->expressions);
      if (ifExpr->elseBlock)
        postProcess(&ifExpr->elseBlock->expressions);
      continue;
    }

    // If statements should be followed by blocks
    if (expr.type == InstructionType::If ||
        expr.type == InstructionType::While ||
        expr.type == InstructionType::Function) {
      if (i == expressions->size() - 1) {
        syntaxError("Cannot have an if/while/function statement as the final "
                    "expression!");
        return;
      }

      std::shared_ptr<BlockExpression> block;
      if (expressions->at(i + 1).get()->type != InstructionType::Block) {
        auto next = expressions->at(i + 1);

        block = std::make_shared<BlockExpression>(
            BlockExpression({next}, next->lineNumber));
        (*expressions)[i + 1] = block;
      }

      if (!block) {
        block =
            std::static_pointer_cast<BlockExpression>(expressions->at(i + 1));
      }

      postProcess(&block->expressions);

      if (expr.type == InstructionType::While) {
        postProcessWhileLoop(expressions, i, expressions->at(i));

        i--; // Reprocess the new expression
      }
    }

    if (expr.type == InstructionType::Function) {
      auto &func = static_cast<FunctionExpression &>(expr);
      func.body = expressions->at(i + 1);

      expressions->erase(expressions->begin() + i + 1);

      func.findReturnStatements();
    }
  }
}

void AstBuilder::build() {
  while (hasNext()) {
    auto expr = parseExpression({TokenType::Semicolon});
    if (expr.has_value())
      expressions.get()->push_back(std::move(expr.value()));

    if (match(TokenType::Semicolon))
      next(); // Consume semicolon
  }

  postProcess(expressions.get());
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

std::shared_ptr<std::vector<std::shared_ptr<Expression>>>
AstBuilder::getExpressions() {
  return expressions;
}
