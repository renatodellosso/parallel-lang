#pragma once

#include "../instruction.hpp"
#include "token.hpp"
#include <vector>
#include <string>
#include <memory>
#include <optional>
#include <functional>

struct Expression
{
  InstructionType type;
  int lineNumber;
  int id;

  std::vector<std::reference_wrapper<Expression>> dependencies;
  std::vector<std::reference_wrapper<Expression>> dependents;

  Expression(InstructionType type, int lineNumber) : type(type),
                                                     lineNumber(lineNumber),
                                                     id(-1),
                                                     dependencies(std::vector<std::reference_wrapper<Expression>>()),
                                                     dependents(std::vector<std::reference_wrapper<Expression>>()) {}

  virtual std::string toString() const;
  virtual std::string toByteCode() const;
  // Returns this expression and all its subexpressions, in the order they will be executed
  virtual std::vector<std::reference_wrapper<Expression>> getWithSubExpressions() const;
  // Link any internal expressions with each other (e.g. a binary expressions depends on its left and right subexpressions)
  virtual void linkInternally();
  /**
   * Sets IDs accordingly to which line the expression will be on in the bytecode.
   * Returns the next ID to use.
   */
  virtual int numberExpressions(int startWith);
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
  std::vector<std::reference_wrapper<Expression>> getWithSubExpressions() const override;
  void linkInternally() override;
  int numberExpressions(int startWith) override;
};

struct BinaryExpression : public Expression
{
  std::shared_ptr<Expression> left, right;

  BinaryExpression(InstructionType type, int lineNumber, std::shared_ptr<Expression> left, std::shared_ptr<Expression> right)
      : Expression(type, lineNumber), left(left), right(right) {}

  std::string toString() const override;
  std::string toByteCode() const override;
  std::vector<std::reference_wrapper<Expression>> getWithSubExpressions() const override;
  void linkInternally() override;
  int numberExpressions(int startWith) override;
};

struct BlockExpression : public Expression
{
  std::vector<std::shared_ptr<Expression>> expressions;

  BlockExpression() : BlockExpression(std::vector<std::shared_ptr<Expression>>(), 0) {}
  BlockExpression(std::vector<std::shared_ptr<Expression>> expressions, int lineNumber)
      : Expression(InstructionType::Block, lineNumber), expressions(std::move(expressions)) {}

  std::string toString() const override;
  std::string toByteCode() const override;
  std::vector<std::reference_wrapper<Expression>> getWithSubExpressions() const override;
  int numberExpressions(int startWith) override;
};