#pragma once

#include "token.hpp"
#include "syntaxError.hpp"
#include "expression.hpp"
#include <vector>
#include <memory>

class AstBuilder
{
private:
  std::unique_ptr<std::vector<SyntaxError>> errors;
  std::unique_ptr<std::vector<Token>> tokens;
  std::unique_ptr<BlockExpression> root;
  std::unique_ptr<std::optional<Expression>> currExpr;

  // Token reading

  int line;
  int nextTokenIndex;
  // Returns true if there is another token after the current one
  bool hasNext();
  Token next();
  Token peek();
  // Returns true if the next token has the specified type. NOTE: Returns false if there is no next token
  bool match(TokenType type, std::optional<TokenSubtype> subtype);

  // Token filtering

  // Building methods

  std::unique_ptr<std::optional<Expression>> buildLine();
  std::unique_ptr<BlockExpression> buildBlock();

  void syntaxError(std::string msg);

public:
  AstBuilder(std::unique_ptr<std::vector<Token>> tokens);
  void build();

  std::unique_ptr<std::vector<SyntaxError>> getErrors();
  std::unique_ptr<BlockExpression> getRoot();
};