#include "expression.hpp"
#include <algorithm>
#include <iterator>

void addDependency(Expression &expr, Expression &dependsOn, int argIndex = -1)
{
  expr.dependencies.push_back(dependsOn);
  dependsOn.dependents.push_back(Dependent(expr, argIndex != -1 ? std::make_optional(argIndex) : std::nullopt));
}

std::string Expression::toString() const
{
  switch (type)
  {
  case InstructionType::Block:
    return "Block";
  case InstructionType::GetLiteral:
    return "GetLiteral";
  case InstructionType::GetIdentifier:
    return "GetIdentifier";
  case InstructionType::ReferenceIdentifier:
    return "ReferenceIdentifier";
  case InstructionType::Declare:
    return "Declare";
  case InstructionType::Set:
    return "Set";
  case InstructionType::Add:
    return "Add";
  case InstructionType::Subtract:
    return "Subtract";
  case InstructionType::Multiply:
    return "Multiply";
  case InstructionType::Divide:
    return "Divide";
  case InstructionType::Negate:
    return "Negate";
  case InstructionType::CompareEquals:
    return "CompareEquals";
  case InstructionType::CompareLessThan:
    return "CompareLessThan";
  case InstructionType::CompareLessThanEquals:
    return "CompareLessThanEquals";
  case InstructionType::CompareGreaterThan:
    return "CompareGreaterThan";
  case InstructionType::CompareGreaterThanEquals:
    return "CompareGreaterThanEquals";
  default:
    return "Unknown Expression Type";
  }
}

std::string Expression::toByteCode() const
{
  std::string bytecode = "";

  // Write number of dependencies
  bytecode += std::to_string(dependencies.size());

  // Write dependents
  bytecode += " ";
  for (auto dep : dependents)
  {
    bytecode += dep.toString();
    bytecode += ",";
  }
  // Remove trailing ','
  bytecode = bytecode.substr(0, bytecode.length() - 1);

  bytecode += " " + std::to_string((int)type);

  return bytecode;
}

std::vector<std::reference_wrapper<Expression>> Expression::getWithSubExpressions() const
{
  std::vector<std::reference_wrapper<Expression>> vector{*(Expression *)this};
  return vector;
}

void Expression::linkInternally()
{
  return;
}

int Expression::numberExpressions(int startWith)
{
  this->id = startWith;
  return startWith + 1;
}

std::string RootExpression::toString() const
{
  return Expression::toString() + "(" + token.raw + ")";
}

std::string RootExpression::toByteCode() const
{
  return Expression::toByteCode() + " " + token.raw;
}

std::string UnaryExpression::toString() const
{
  return Expression::toString() + "(" + root.get()->toString() + ")";
}

std::string UnaryExpression::toByteCode() const
{
  return root->toByteCode() + "\n" + Expression::toByteCode();
}

std::vector<std::reference_wrapper<Expression>> UnaryExpression::getWithSubExpressions() const
{
  std::vector<std::reference_wrapper<Expression>> vector = root.get()->getWithSubExpressions();

  auto super = Expression::getWithSubExpressions();
  std::move(super.begin(), super.end(), std::back_inserter(vector));

  return vector;
}

void UnaryExpression::linkInternally()
{
  addDependency(*this, *root.get(), 0);
}

int UnaryExpression::numberExpressions(int startWith)
{
  startWith = root.get()->numberExpressions(startWith);
  return Expression::numberExpressions(startWith);
}

std::string BinaryExpression::toString() const
{
  return Expression::toString() + "(" + left.get()->toString() + ", " + right.get()->toString() + ")";
}

std::string BinaryExpression::toByteCode() const
{
  return left->toByteCode() + "\n" + right->toByteCode() + "\n" + Expression::toByteCode();
}

std::vector<std::reference_wrapper<Expression>> BinaryExpression::getWithSubExpressions() const
{
  std::vector<std::reference_wrapper<Expression>> vector = left.get()->getWithSubExpressions();

  auto rightVec = right.get()->getWithSubExpressions();
  std::move(rightVec.begin(), rightVec.end(), std::back_inserter(vector)); // Move rightVec into end of vector

  auto super = Expression::getWithSubExpressions();
  std::move(super.begin(), super.end(), std::back_inserter(vector));

  return vector;
}

void BinaryExpression::linkInternally()
{
  addDependency(*this, *left.get(), 0);
  addDependency(*this, *right.get(), 1);
}

int BinaryExpression::numberExpressions(int startWith)
{
  startWith = left.get()->numberExpressions(startWith);
  startWith = right.get()->numberExpressions(startWith);
  return Expression::numberExpressions(startWith);
}

std::string BlockExpression::toString() const
{
  auto str = Expression::toString() + " {\n";

  // Use references
  for (auto &line : expressions)
  {
    str += "\t" + line.get()->toString() + "\n";
  }

  return str + "}";
}

std::string BlockExpression::toByteCode() const
{
  std::string str;

  // Use references
  for (auto &line : expressions)
  {
    str += line->toByteCode() + ";\n";
  }

  return str.erase(str.length() - 1); // Erase trailing \n
}

std::vector<std::reference_wrapper<Expression>> BlockExpression::getWithSubExpressions() const
{
  std::vector<std::reference_wrapper<Expression>> vector;

  for (auto expr : expressions)
  {
    auto exprVec = expr.get()->getWithSubExpressions();
    std::move(exprVec.begin(), exprVec.end(), std::back_inserter(vector)); // Move rightVec into end of vector
  }

  auto super = Expression::getWithSubExpressions();
  std::move(super.begin(), super.end(), std::back_inserter(vector));

  return vector;
}

int BlockExpression::numberExpressions(int startWith)
{
  for (auto &line : expressions)
  {
    startWith = line.get()->numberExpressions(startWith);
  }

  return startWith;
}

Dependent::Dependent(Expression &expr, std::optional<int> argIndex) : expr(expr), argIndex(argIndex) {}

Dependent::Dependent(Expression &expr, int argIndex) : Dependent(expr, std::make_optional(argIndex)) {}

Dependent::Dependent(Expression &expr) : Dependent(expr, std::nullopt) {}

std::string Dependent::toString()
{
  std::string str = std::to_string(expr.get().id);
  if (argIndex.has_value())
    str += "-" + std::to_string(argIndex.value());
  return str;
}
