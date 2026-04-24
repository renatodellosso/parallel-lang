#pragma once

#include "../instruction.hpp"
#include "token.hpp"
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

struct Expression;

struct ExprDependent {
  std::reference_wrapper<Expression> expr;
  std::optional<int> argIndex;

  ExprDependent(Expression &expr, std::optional<int> argIndex);
  ExprDependent(Expression &expr, int argIndex);
  ExprDependent(Expression &expr);
  std::string toString();
};

struct Expression {
  InstructionType type;
  int lineNumber;
  int id;

  std::vector<std::reference_wrapper<Expression>> dependencies;
  std::vector<ExprDependent> dependents;
  Expression *dependentRedirect;

  Expression(InstructionType type, int lineNumber)
      : type(type), lineNumber(lineNumber), id(-1),
        dependencies(std::vector<std::reference_wrapper<Expression>>()),
        dependents(std::vector<ExprDependent>()), dependentRedirect(nullptr) {}

  virtual std::string toString() const;
  virtual std::string toByteCode() const;
  // Returns this expression and all its subexpressions, in the order they will
  // be executed
  virtual std::vector<std::reference_wrapper<Expression>>
  getWithSubExpressions() const;
  // Link any internal expressions with each other (e.g. a binary expressions
  // depends on its left and right subexpressions)
  virtual void linkInternally();
  /**
   * Sets IDs accordingly to which line the expression will be on in the
   * bytecode. Returns the next ID to use.
   */
  virtual int numberExpressions(int startWith);
  virtual int countInstructions() const;
};

struct RootExpression : public Expression {
  Token token;

  RootExpression(InstructionType type, int lineNumber, Token token)
      : Expression(type, lineNumber), token(token) {}

  std::string toString() const override;
  std::string toByteCode() const override;
};

struct UnaryExpression : public Expression {
  std::shared_ptr<Expression> root;

  UnaryExpression(InstructionType type, int lineNumber,
                  std::shared_ptr<Expression> root)
      : Expression(type, lineNumber), root(std::move(root)) {}

  std::string toString() const override;
  std::string toByteCode() const override;
  std::vector<std::reference_wrapper<Expression>>
  getWithSubExpressions() const override;
  void linkInternally() override;
  int numberExpressions(int startWith) override;
  int countInstructions() const override;
};

struct BinaryExpression : public Expression {
  std::shared_ptr<Expression> left, right;

  BinaryExpression(InstructionType type, int lineNumber,
                   std::shared_ptr<Expression> left,
                   std::shared_ptr<Expression> right)
      : Expression(type, lineNumber), left(left), right(right) {}

  std::string toString() const override;
  std::string toByteCode() const override;
  std::vector<std::reference_wrapper<Expression>>
  getWithSubExpressions() const override;
  void linkInternally() override;
  int numberExpressions(int startWith) override;
  int countInstructions() const override;
};

struct BlockExpression : public Expression {
  std::vector<std::shared_ptr<Expression>> expressions;

  BlockExpression()
      : BlockExpression(std::vector<std::shared_ptr<Expression>>(), 0) {}
  BlockExpression(std::vector<std::shared_ptr<Expression>> expressions,
                  int lineNumber)
      : Expression(InstructionType::Block, lineNumber),
        expressions(std::move(expressions)) {}
  BlockExpression(int lineNumber)
      : BlockExpression(std::vector<std::shared_ptr<Expression>>(),
                        lineNumber) {}

  std::string toString() const override;
  std::string toByteCode() const override;
  std::vector<std::reference_wrapper<Expression>>
  getWithSubExpressions() const override;
  int numberExpressions(int startWith) override;
  int countInstructions() const override;
};

struct FunctionExprParameter {
  std::string type;
  std::string name;
};

struct FunctionExpression : public Expression {
  std::string name;
  std::string returnType;
  std::vector<FunctionExprParameter> params;
  std::shared_ptr<Expression> body;

  // Each entry is a resource name and the set of expressions that first use it
  // before a set (if it's immediately set, this will just contain the setting
  // expression)
  std::unordered_map<std::string,
                     std::vector<std::reference_wrapper<Expression>>>
      firstUses;
  std::unordered_map<std::string,
                     std::vector<std::reference_wrapper<Expression>>>
      lastUses;
  std::unordered_map<std::string,
                     std::vector<std::reference_wrapper<Expression>>>
      lastWrites;

  FunctionExpression() : FunctionExpression("unnamed_func", "void", 0) {}
  FunctionExpression(std::string name, std::string returnType, int lineNumber)
      : Expression(InstructionType::Function, lineNumber), name(name),
        returnType(returnType), params(std::vector<FunctionExprParameter>()),
        body(nullptr), firstUses(), lastUses(), lastWrites() {}
  FunctionExpression(int lineNumber)
      : FunctionExpression("unnamed_func", "void", lineNumber) {}

  std::string toString() const override;
  std::string toByteCode() const override;
  std::vector<std::reference_wrapper<Expression>>
  getWithSubExpressions() const override;
  int numberExpressions(int startWith) override;
  int countInstructions() const override;
};