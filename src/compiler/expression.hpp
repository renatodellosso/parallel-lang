#pragma once

#include "../instruction.hpp"
#include "token.hpp"
#include <vector>

struct Expression
{
  InstructionType type;

  Expression(InstructionType type) : type(type) {}
};

struct RootExpression : public Expression
{
  Token token;

  RootExpression(Token token) : Expression(InstructionType::GetLiteral), token(token) {}
};

struct UnaryExpression : public Expression
{
  Expression root;

  UnaryExpression(InstructionType type, Expression root) : Expression(type), root(root) {}
};

struct BinaryExpression : public Expression
{
  Expression left, right;

  BinaryExpression(InstructionType type, Expression left, Expression right) : Expression(type), left(left), right(right) {}
};

struct BlockExpression : public Expression
{
  std::vector<Expression> expressions;

  BlockExpression() : BlockExpression(std::vector<Expression>()) {}
  BlockExpression(std::vector<Expression> expressions) : Expression(InstructionType::Block), expressions(expressions) {}
};