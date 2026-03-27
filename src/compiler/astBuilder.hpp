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

  // Token reading

  int line;
  int nextTokenIndex;
  Token next();
  Token peek();

  // Token filtering

  // Consumes the next token and returns true if it is a line-ending token
  bool lineEnding();

  void syntaxError(std::string msg);

public:
  AstBuilder(std::unique_ptr<std::vector<Token>> tokens);
  void build();

  std::unique_ptr<std::vector<SyntaxError>> getErrors();
  std::unique_ptr<BlockExpression> getRoot();
};