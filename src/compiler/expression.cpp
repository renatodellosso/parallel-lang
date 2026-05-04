#include "expression.hpp"
#include <algorithm>
#include <format>
#include <iterator>
#include <memory>
#include <string>

void addDependency(Expression &expr, Expression &dependsOn, int argIndex = -1) {
  expr.dependencies.push_back(dependsOn);
  dependsOn.dependents.push_back(ExprDependent(
      expr, argIndex != -1 ? std::make_optional(argIndex) : std::nullopt));
}

ExprDependent::ExprDependent(Expression &expr, std::optional<int> argIndex)
    : expr(expr), argIndex(argIndex) {}

ExprDependent::ExprDependent(Expression &expr, int argIndex)
    : ExprDependent(expr, std::make_optional(argIndex)) {}

ExprDependent::ExprDependent(Expression &expr)
    : ExprDependent(expr, std::nullopt) {}

std::string ExprDependent::toString() {
  std::string str = std::to_string(expr.get().id);
  if (argIndex.has_value())
    str += "." + std::to_string(argIndex.value());
  return str;
}

std::string Expression::toString() const {
  return std::format("({}){}", id, instructionTypeToString(type));
}

std::string Expression::toByteCode() const {
  std::string bytecode = "";

  // Write number of dependencies
  bytecode += std::to_string(dependencies.size());

  // Write dependents
  bytecode += " ";
  for (auto dep : dependents) {
    bytecode += dep.toString();
    bytecode += ",";
  }

  // Remove trailing ','
  if (dependents.size() > 0)
    bytecode = bytecode.substr(0, bytecode.length() - 1);

  bytecode += " " + std::to_string((int)type);

  return bytecode;
}

std::vector<std::reference_wrapper<Expression>>
Expression::getWithSubExpressions() const {
  std::vector<std::reference_wrapper<Expression>> vector{*(Expression *)this};
  return vector;
}

void Expression::linkInternally() { return; }

int Expression::numberExpressions(int startWith) {
  this->id = startWith;
  return startWith + 1;
}

int Expression::countInstructions() const { return 1; }

std::string RootExpression::toString() const {
  return Expression::toString() + "(" + token.raw + ")";
}

std::string RootExpression::toByteCode() const {
  return Expression::toByteCode() + " " + token.raw;
}

std::string UnaryExpression::toString() const {
  return Expression::toString() + "(" + root.get()->toString() + ")";
}

std::string UnaryExpression::toByteCode() const {
  return root->toByteCode() + "\n" + Expression::toByteCode();
}

std::vector<std::reference_wrapper<Expression>>
UnaryExpression::getWithSubExpressions() const {
  std::vector<std::reference_wrapper<Expression>> vector =
      root.get()->getWithSubExpressions();

  auto super = Expression::getWithSubExpressions();
  std::move(super.begin(), super.end(), std::back_inserter(vector));

  return vector;
}

void UnaryExpression::linkInternally() { addDependency(*this, *root.get(), 0); }

int UnaryExpression::numberExpressions(int startWith) {
  startWith = root.get()->numberExpressions(startWith);
  return Expression::numberExpressions(startWith);
}

int UnaryExpression::countInstructions() const {
  return 1 + root->countInstructions();
}

std::string BinaryExpression::toString() const {
  return Expression::toString() + "(" + left.get()->toString() + ", " +
         right.get()->toString() + ")";
}

std::string BinaryExpression::toByteCode() const {
  return left->toByteCode() + "\n" + right->toByteCode() + "\n" +
         Expression::toByteCode();
}

std::vector<std::reference_wrapper<Expression>>
BinaryExpression::getWithSubExpressions() const {
  std::vector<std::reference_wrapper<Expression>> vector =
      left.get()->getWithSubExpressions();

  auto rightVec = right.get()->getWithSubExpressions();
  std::move(rightVec.begin(), rightVec.end(),
            std::back_inserter(vector)); // Move rightVec into end of vector

  auto super = Expression::getWithSubExpressions();
  std::move(super.begin(), super.end(), std::back_inserter(vector));

  return vector;
}

void BinaryExpression::linkInternally() {
  addDependency(*this, *left.get(), 0);
  addDependency(*this, *right.get(), 1);
}

int BinaryExpression::numberExpressions(int startWith) {
  startWith = left.get()->numberExpressions(startWith);
  startWith = right.get()->numberExpressions(startWith);
  return Expression::numberExpressions(startWith);
}

int BinaryExpression::countInstructions() const {
  return 1 + left->countInstructions() + right->countInstructions();
}

std::string BlockExpression::toString() const {
  auto str = Expression::toString() + " {\n";

  // Use references
  for (auto &line : expressions) {
    str += "\t" + line.get()->toString() + "\n";
  }

  return str + "}";
}

std::string BlockExpression::toByteCode() const {
  // Subtract 1 from count to exclude this instruction
  std::string str = Expression::toByteCode() + " " +
                    std::to_string(countInstructions() - 1) + "\n";

  // Use references
  for (auto &line : expressions) {
    str += line->toByteCode() + "\n";
  }

  return str.erase(str.length() - 1); // Erase trailing \n
}

std::vector<std::reference_wrapper<Expression>>
BlockExpression::getWithSubExpressions() const {
  std::vector<std::reference_wrapper<Expression>> vector =
      Expression::getWithSubExpressions();

  for (auto expr : expressions) {
    auto exprVec = expr.get()->getWithSubExpressions();
    std::move(exprVec.begin(), exprVec.end(),
              std::back_inserter(vector)); // Move rightVec into end of vector
  }

  return vector;
}

int BlockExpression::numberExpressions(int startWith) {
  id = startWith;
  startWith++;

  for (auto &line : expressions) {
    startWith = line.get()->numberExpressions(startWith);
  }

  return startWith;
}

int BlockExpression::countInstructions() const {
  int count = 1;
  for (auto expr : expressions)
    count += expr->countInstructions();
  return count;
}

std::string FunctionExpression::toString() const {
  auto str = Expression::toString() + " " + returnType + " " + name + "(";

  for (auto param : params)
    str += param.type + " " + param.name + ", ";

  if (params.size())
    str.erase(str.end() - 2, str.end()); // Remove trailing ', '

  str += ") {\n";

  str += "\t" + body.get()->toString() + "\n";

  return str + "}";
}

// Formats as "[size] [key] [valueCount] [value1] [value2] [value3] " (note the
// trailing space!)
static std::string stringifyUses(
    std::unordered_map<std::string,
                       std::vector<std::reference_wrapper<Expression>>>
        uses) {
  std::string str = std::to_string(uses.size());

  str += " ";
  for (auto entry : uses) {
    str += entry.first + " " + std::to_string(entry.second.size()) + " ";

    for (auto expr : entry.second)
      str += std::to_string(expr.get().id) + " ";
  }

  return str;
}

// Formats as "[size] [key] [value] " (note the trailing space!)
static std::string stringifyWrites(
    std::unordered_map<std::string, std::reference_wrapper<Expression>>
        writes) {
  std::string str = std::to_string(writes.size()) + " ";

  for (auto entry : writes) {
    str += entry.first + " ";
    str += std::to_string(entry.second.get().id);
    str += " ";
  }

  return str;
}

// Bytecode params are in format "[returnType] [name] [# of params] [param i
// type] [param i name] [first uses] [first writes] [last uses] [last
// writes]"
std::string FunctionExpression::toByteCode() const {
  // Subtract 1 from count to exclude this instruction
  std::string str =
      Expression::toByteCode() + " " + returnType + " " + name + " ";

  str += std::to_string(params.size()) + " ";
  for (auto param : params) {
    str += param.type + " " + param.name + " ";
  }

  str += stringifyUses(firstUses);
  str += stringifyWrites(firstWrites);
  str += stringifyUses(lastUses);
  str += stringifyWrites(lastWrites);
  str.erase(str.end() - 1); // Remove trailing ' '

  str += "\n";

  str += body->toByteCode() + "\n";

  return str.erase(str.length() - 1); // Erase trailing \n
}

std::vector<std::reference_wrapper<Expression>>
FunctionExpression::getWithSubExpressions() const {
  std::vector<std::reference_wrapper<Expression>> vector =
      Expression::getWithSubExpressions();

  auto exprVec = body->getWithSubExpressions();
  std::move(exprVec.begin(), exprVec.end(),
            std::back_inserter(vector)); // Move rightVec into end of vector

  return vector;
}

void FunctionExpression::linkInternally() { addDependency(*body.get(), *this); }

int FunctionExpression::numberExpressions(int startWith) {
  id = startWith;
  startWith++;

  return body->numberExpressions(startWith);
}

int FunctionExpression::countInstructions() const {
  return 1 + body->countInstructions();
}

std::string CallExpression::toString() const {
  auto str = Expression::toString() + "(";

  // Use references
  for (auto &line : expressions) {
    str += line.get()->toString() + ", ";
  }

  str.erase(str.end() - 2, str.end());

  return str + ")";
}

std::string CallExpression::toByteCode() const {
  // Subtract 1 from count to exclude this instruction
  std::string str = "";

  // Use references
  for (auto &line : expressions) {
    str += line->toByteCode() + "\n";
  }

  str += Expression::toByteCode();

  return str.erase(str.length() - 1); // Erase trailing \n
}

std::vector<std::reference_wrapper<Expression>>
CallExpression::getWithSubExpressions() const {
  std::vector<std::reference_wrapper<Expression>> vector;

  for (auto expr : expressions) {
    auto exprVec = expr.get()->getWithSubExpressions();
    std::move(exprVec.begin(), exprVec.end(),
              std::back_inserter(vector)); // Move rightVec into end of vector
  }

  auto base = Expression::getWithSubExpressions();
  std::move(base.begin(), base.end(), std::back_inserter(vector));

  return vector;
}

int CallExpression::numberExpressions(int startWith) {

  for (auto &line : expressions) {
    startWith = line.get()->numberExpressions(startWith);
  }

  id = startWith;
  startWith++;

  return startWith;
}

std::string CallExpression::getFunctionName() const {
  auto nameExpr = std::static_pointer_cast<RootExpression>(expressions[0]);

  return nameExpr->token.raw;
}