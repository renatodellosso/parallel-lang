#pragma once

#include "expression.hpp"
#include "syntaxError.hpp"
#include "token.hpp"
#include <initializer_list>
#include <memory>
#include <optional>
#include <vector>

class TokenFilter {
  TokenType type;
  std::optional<TokenSubtype> subtype;

public:
  TokenFilter(TokenType type,
              std::optional<TokenSubtype> subtype = std::nullopt);

  bool match(Token token);
};

class AstBuilder {
private:
  std::shared_ptr<std::vector<SyntaxError>> errors;
  std::unique_ptr<std::vector<Token>> tokens;
  std::shared_ptr<std::vector<std::shared_ptr<Expression>>> expressions;

  // Token reading

  int line;
  int nextTokenIndex;
  // Returns true if there is another token after the current one
  bool hasNext();
  Token next();
  Token peek();

  /**
   * Returns true if the next token has any of the specified types. NOTE:
   * Returns false if there is no next token. Does not consume the token.
   */
  bool match(std::initializer_list<TokenFilter> filters);
  bool match(TokenFilter filter);
  bool match(TokenType type, std::optional<TokenSubtype> subtype = std::nullopt);

  // Building methods

  // Adds (usually) one token to the expression
  std::optional<std::unique_ptr<Expression>> parseLeadingExpression();
  std::optional<std::unique_ptr<Expression>>
  parseCompoundExpression(std::optional<std::unique_ptr<Expression>> prev,
                          std::initializer_list<TokenFilter> endOn);
  FunctionExprParameter parseFuncParam();
  std::optional<std::unique_ptr<FunctionExpression>>
  parseFunction(std::unique_ptr<BinaryExpression> declaration);
  // Parses a block, including the { }
  std::optional<std::unique_ptr<BlockExpression>> parseBlock();

  std::optional<std::unique_ptr<Expression>>
  extendExpression(std::optional<std::unique_ptr<Expression>> prev,
                   std::initializer_list<TokenFilter> endOn);
  std::optional<std::unique_ptr<Expression>>
  parseExpression(std::initializer_list<TokenFilter> endOn);

  void postProcess(std::vector<std::shared_ptr<Expression>> *expressions);

  void syntaxError(std::string msg);

public:
  AstBuilder(std::unique_ptr<std::vector<Token>> tokens);
  void build();

  std::shared_ptr<std::vector<SyntaxError>> getErrors();
  std::shared_ptr<std::vector<std::shared_ptr<Expression>>> getExpressions();
};