#pragma once

#include "../instruction.hpp"
#include "token.hpp"
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct Expression;
struct BlockExpression;

struct ExprDependent {
  std::reference_wrapper<Expression> expr;
  std::optional<int> argIndex;

  ExprDependent(Expression &expr, std::optional<int> argIndex);
  ExprDependent(Expression &expr, int argIndex);
  ExprDependent(Expression &expr);
  std::string toString();
};

// Custom hasher and equality
namespace std {
template <> struct hash<ExprDependent> {
  std::size_t operator()(const ExprDependent &defer) const noexcept {
    // Reinterpret the memory address as a size_t
    return reinterpret_cast<size_t>(&defer.expr.get());
  }
};

template <> struct equal_to<ExprDependent> {
  bool operator()(const ExprDependent &a,
                  const ExprDependent &b) const noexcept {
    return &a.expr.get() == &b.expr.get() &&
           a.argIndex.value_or(-1) == b.argIndex.value_or(-1);
  }
};
} // namespace std

struct Expression {
  InstructionType type;
  int lineNumber;
  int id;

  std::vector<std::reference_wrapper<Expression>> dependencies;
  std::unordered_set<ExprDependent> dependents;
  Expression *dependentRedirect;

  Expression(InstructionType type, int lineNumber)
      : type(type), lineNumber(lineNumber), id(-1),
        dependencies(std::vector<std::reference_wrapper<Expression>>()),
        dependents(std::unordered_set<ExprDependent>()),
        dependentRedirect(nullptr) {}

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

struct IfExpression : public UnaryExpression {
  std::shared_ptr<BlockExpression> thenBlock;
  std::shared_ptr<Expression> elseInstruction;
  std::shared_ptr<BlockExpression> elseBlock;
  std::shared_ptr<Expression> mergeInstruction;

  IfExpression(int lineNumber, std::shared_ptr<Expression> condition,
               std::shared_ptr<BlockExpression> thenBlock,
               std::shared_ptr<BlockExpression> elseBlock = nullptr);

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

// Don't link internally in a method, we handle that in GraphLinker since it's
// more complicated than just adding deps for The block's expressions
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

struct Resource;

struct FunctionExpression : public Expression {
  std::string name;
  std::string returnType;
  std::vector<FunctionExprParameter> params;
  std::shared_ptr<Expression> body;

  bool finishedLinking, deferred;
  std::shared_ptr<Scope<Resource>> scope;

  // Each entry is a resource name and the set of expressions that first use it
  // before a set (if it's immediately set, this will just contain the setting
  // expression)

  // Contains params
  std::unordered_map<std::string,
                     std::vector<std::reference_wrapper<Expression>>>
      firstUses;
  // Contains params
  std::unordered_map<std::string, std::reference_wrapper<Expression>>
      firstWrites;
  // No params
  std::unordered_map<std::string,
                     std::vector<std::reference_wrapper<Expression>>>
      lastUses;
  // No params
  std::unordered_map<std::string, std::reference_wrapper<Expression>>
      lastWrites;

  FunctionExpression() : FunctionExpression("unnamed_func", "void", 0) {}
  FunctionExpression(std::string name, std::string returnType, int lineNumber)
      : Expression(InstructionType::Function, lineNumber), name(name),
        returnType(returnType), params(std::vector<FunctionExprParameter>()),
        body(nullptr), firstUses(), firstWrites(), lastUses(), lastWrites(),
        finishedLinking(false) {}
  FunctionExpression(int lineNumber)
      : FunctionExpression("unnamed_func", "void", lineNumber) {}

  std::string toString() const override;
  std::string toByteCode() const override;
  std::vector<std::reference_wrapper<Expression>>
  getWithSubExpressions() const override;
  void linkInternally() override;
  int numberExpressions(int startWith) override;
  int countInstructions() const override;
};

struct CallExpression;

struct UnaryCallExpression : public UnaryExpression {
  // Dependent remappings - maps dependent index to subprogram IDs that that
  // dep should depend on instead of this call
  std::unordered_map<int, std::vector<std::reference_wrapper<Expression>>>
      depRemaps;

  std::optional<std::reference_wrapper<FunctionExpression>> function;
  CallExpression &block;

  UnaryCallExpression(CallExpression &block, int lineNumber,
                      std::shared_ptr<Expression> root)
      : UnaryExpression(type, lineNumber, root), block(block) {
    type = InstructionType::Call;
  }

  std::string toByteCode() const override;
};

// First expression in expressions is the function identifier
struct CallExpression : public BlockExpression {

  std::optional<std::reference_wrapper<FunctionExpression>> function;

  CallExpression(std::shared_ptr<Expression> funcName, int lineNumber)
      : BlockExpression(lineNumber), function(std::nullopt) {
    expressions.push_back(
        std::make_shared<UnaryCallExpression>(*this, lineNumber, funcName));
  }

  std::string getFunctionName() const;

  // Sets function and converts argument expressions into declarations
  void setFunction(std::reference_wrapper<FunctionExpression> function);

  UnaryCallExpression &getActualCall();

  void linkInternally() override;

  void addArgument(std::shared_ptr<Expression> arg);
};
