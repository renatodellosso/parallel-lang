#pragma once

#include "../instruction.hpp"
#include "token.hpp"
#include <vector>

struct Expression
{
  InstructionType type;
};

struct RootExpression : public Expression
{
  Token token;
};

struct UnaryExpression : public Expression
{
  Expression root;
};

struct BinaryExpression : public Expression
{
  Expression left, right;
};

struct BlockExpression : public Expression
{
  std::vector<Expression> expressions;
};