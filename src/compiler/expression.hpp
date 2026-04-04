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
  int lineNumber;

  int id;
  std::vector<Expression *> dependencies;
  std::vector<Expression *> dependents;

  Expression(InstructionType type, int lineNumber) : type(type), lineNumber(lineNumber), dependencies(std::vector<Expression *>()), dependents(std::vector<Expression *>()) {}

  virtual std::string toString() const;
  virtual std::string toByteCode() const;
};

struct RootExpression : public Expression
{
  Token token;

  RootExpression(InstructionType type, int lineNumber, Token token) : Expression(type, lineNumber), token(token) {}

  std::string toString() const override;
  std::string toByteCode() const override;
};

struct UnaryExpression : public Expression
{
  std::shared_ptr<Expression> root;

  UnaryExpression(InstructionType type, int lineNumber, std::shared_ptr<Expression> root) : Expression(type, lineNumber), root(std::move(root)) {}

  std::string toString() const override;
  std::string toByteCode() const override;
};

struct BinaryExpression : public Expression
{
  std::shared_ptr<Expression> left, right;

  BinaryExpression(InstructionType type, int lineNumber, std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
      : Expression(type, lineNumber), left(left), right(right) {}

  std::string toString() const override;
  std::string toByteCode() const override;
};

struct BlockExpression : public Expression
{
  std::vector<std::shared_ptr<Expression>> expressions;

  BlockExpression() : BlockExpression(std::vector<std::shared_ptr<Expression>>(), 0) {}
  BlockExpression(std::vector<std::shared_ptr<Expression>> expressions, int lineNumber)
      : Expression(InstructionType::Block, lineNumber), expressions(std::move(expressions)) {}

  std::string toString() const override;
  std::string toByteCode() const override;
};