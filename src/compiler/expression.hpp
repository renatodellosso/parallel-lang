#pragma once

#include "../instruction.hpp"
#include "token.hpp"
#include <vector>
#include <string>
#include <memory>
#include <optional>

struct Expression
{
  InstructionType type;

  Expression(InstructionType type) : type(type) {}

  virtual std::string toString() const;
};

struct RootExpression : public Expression
{
  Token token;

  RootExpression(InstructionType type, Token token) : Expression(type), token(token) {}

  std::string toString() const override;
};

struct UnaryExpression : public Expression
{
  std::unique_ptr<Expression> root;

  UnaryExpression(InstructionType type, std::unique_ptr<Expression> root) : Expression(type), root(std::move(root)) {}

  std::string toString() const override;
};

struct BinaryExpression : public Expression
{
  std::unique_ptr<Expression> left, right;

  BinaryExpression(InstructionType type, std::unique_ptr<Expression> left, std::unique_ptr<Expression> right)
      : Expression(type), left(std::move(left)), right(std::move(right)) {}

  std::string toString() const override;
};

struct BlockExpression : public Expression
{
  std::vector<std::unique_ptr<Expression>> expressions;

  BlockExpression() : BlockExpression(std::vector<std::unique_ptr<Expression>>()) {}
  BlockExpression(std::vector<std::unique_ptr<Expression>> expressions)
      : Expression(InstructionType::Block), expressions(std::move(expressions)) {}

  std::string toString() const override;
};