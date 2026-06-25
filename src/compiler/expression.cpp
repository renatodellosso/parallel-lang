#include "expression.hpp"
#include "../utils.hpp"
#include "token.hpp"
#include <algorithm>
#include <format>
#include <functional>
#include <iterator>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

static std::string getDebugString(const Expression &expr) {
  return "// " + replaceAll(expr.toString(), "\n", "\n// ");
}

void addDependency(Expression &expr, Expression &dependsOn, int argIndex = -1) {
  int origSize = dependsOn.dependents.size();
  dependsOn.dependents.insert(ExprDependent(
      expr, argIndex != -1 ? std::make_optional(argIndex) : std::nullopt));

  if (dependsOn.dependents.size() != origSize)
    expr.dependencies.push_back(dependsOn);
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

std::string Expression::toByteCode(CliArgs args) const {
  std::string bytecode = "";

  if (args.debugBytecode) {
    bytecode += getDebugString(*this) + "\n";
  }

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

std::string RootExpression::toByteCode(CliArgs args) const {
  return Expression::toByteCode(args) + " " + token.raw;
}

std::string UnaryExpression::toString() const {
  return Expression::toString() + "(" + root.get()->toString() + ")";
}

std::string UnaryExpression::toByteCode(CliArgs args) const {
  return root->toByteCode(args) + "\n" + Expression::toByteCode(args);
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

std::string BinaryExpression::toByteCode(CliArgs args) const {
  return left->toByteCode(args) + "\n" + right->toByteCode(args) + "\n" +
         Expression::toByteCode(args);
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

std::vector<int> BlockExpression::getUnaryCallOffsets() const {
  std::vector<int> offsets;

  for (auto expr : expressions) {
    if (expr->type == InstructionType::Call) {
      offsets.push_back(expr->id - id);
    }
  }

  return offsets;
}

std::string BlockExpression::toString() const {
  auto str = Expression::toString() + " {\n";

  // Use references
  for (auto &line : expressions) {
    str += "\t" + line.get()->toString() + "\n";
  }

  return str + "}";
}

// Outputs in format: "[# of calls] [call offset 1] [call offset 2] [call offset
// 3]""
std::string BlockExpression::toByteCode(CliArgs args) const {
  // Subtract 1 from count to exclude this instruction
  std::string str = Expression::toByteCode(args) + " " +
                    std::to_string(countInstructions() - 1);

  auto callOffsets = getUnaryCallOffsets();
  str += " " + std::to_string(callOffsets.size());
  for (auto offset : callOffsets)
    str += " " + std::to_string(offset);

  str += "\n";

  // Use references
  for (auto &line : expressions) {
    str += line->toByteCode(args) + "\n";
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

IfExpression::IfExpression(int lineNumber,
                           std::shared_ptr<Expression> condition,
                           std::shared_ptr<BlockExpression> thenBlock,
                           std::shared_ptr<BlockExpression> elseBlock)
    : UnaryExpression(InstructionType::If, lineNumber, condition),
      thenBlock(std::move(thenBlock)), elseInstruction(nullptr),
      elseBlock(std::move(elseBlock)),
      mergeInstruction(std::make_shared<Expression>(
          InstructionType::BranchMerge, lineNumber)) {
  if (this->elseBlock)
    elseInstruction =
        std::make_shared<Expression>(InstructionType::Else, lineNumber);
}

std::string IfExpression::toString() const {
  auto str = Expression::toString() + "(" + root->toString() + ") {\n";
  str += "\t" + thenBlock->toString() + "\n";

  if (elseBlock)
    str += "} else {\n\t" + elseBlock->toString() + "\n";

  return str + "}";
}

std::string IfExpression::toByteCode(CliArgs args) const {
  std::string str = root->toByteCode(args) + "\n" +
                    Expression::toByteCode(args) + "\n" +
                    thenBlock->toByteCode(args);

  if (elseBlock) {
    str += "\n" + elseInstruction->toByteCode(args) + "\n" +
           elseBlock->toByteCode(args);
  }

  str += "\n" + mergeInstruction->toByteCode(args);

  return str;
}

std::vector<std::reference_wrapper<Expression>>
IfExpression::getWithSubExpressions() const {
  std::vector<std::reference_wrapper<Expression>> vector =
      root->getWithSubExpressions();

  auto super = Expression::getWithSubExpressions();
  std::move(super.begin(), super.end(), std::back_inserter(vector));

  auto thenVec = thenBlock->getWithSubExpressions();
  std::move(thenVec.begin(), thenVec.end(), std::back_inserter(vector));

  if (elseBlock) {
    vector.push_back(*elseInstruction);

    auto elseVec = elseBlock->getWithSubExpressions();
    std::move(elseVec.begin(), elseVec.end(), std::back_inserter(vector));
  }

  vector.push_back(*mergeInstruction);

  return vector;
}

void IfExpression::linkInternally() {
  addDependency(*this, *root, 0);

  auto thenExprs = thenBlock->getWithSubExpressions();
  addDependency(*mergeInstruction, *this);
  for (auto expr : thenExprs)
    addDependency(*mergeInstruction, expr.get());

  if (!elseBlock)
    return;

  addDependency(*elseInstruction, *this, 0);

  for (auto expr : thenExprs)
    addDependency(*elseInstruction, expr.get());

  addDependency(*mergeInstruction, *elseInstruction);

  auto elseExprs = elseBlock->getWithSubExpressions();
  for (auto expr : elseExprs)
    addDependency(*mergeInstruction, expr.get());
}

int IfExpression::numberExpressions(int startWith) {
  startWith = root->numberExpressions(startWith);
  startWith = Expression::numberExpressions(startWith);
  startWith = thenBlock->numberExpressions(startWith);

  if (elseBlock) {
    startWith = elseInstruction->numberExpressions(startWith);
    startWith = elseBlock->numberExpressions(startWith);
  }

  return mergeInstruction->numberExpressions(startWith);
}

int IfExpression::countInstructions() const {
  int count = root->countInstructions() + 1 + thenBlock->countInstructions();

  if (elseBlock)
    count += 1 + elseBlock->countInstructions();

  return count + 1;
}

std::string FunctionExpression::toString() const {
  auto str = Expression::toString() + " " + returnType + " " + name + "(";

  str += "linking ";
  if (!finishedLinking)
    str += "NOT ";
  str += "finished, ";

  for (auto param : params)
    str += param.type + " " + param.name + ", ";

  str.erase(str.end() - 2, str.end()); // Remove trailing ', '

  str += ") {\n";

  str += "\t" + body.get()->toString() + "\n";

  return str + "}";
}

void FunctionExpression::findReturnStatements() {
  for (auto expr : body->getWithSubExpressions()) {
    if (expr.get().type == InstructionType::Return)
      returnStatements.push_back(expr);
  }
}

// Bytecode params are in format "[returnType] [name] [# of params] [param i
// type] [param i name] [first uses] [first writes] [last uses] [last
// writes]"
std::string FunctionExpression::toByteCode(CliArgs args) const {
  // Subtract 1 from count to exclude this instruction
  std::string str =
      Expression::toByteCode(args) + " " + returnType + " " + name + "\n";

  str += body->toByteCode(args);

  return str;
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

// Maps param index to first uses vector
static std::unordered_map<int, std::vector<std::reference_wrapper<Expression>>>
getCallArgMappings(const UnaryCallExpression &call) {
  auto func = call.function->get();

  std::unordered_map<int, std::vector<std::reference_wrapper<Expression>>>
      mappings;

  for (int i = 0; i < func.params.size(); i++) {
    mappings[i] = func.firstUses[func.params[i].name];
  }

  return mappings;
}

// Bytecode args are in format "[dependency remapping count] [[param index]
// [dependent count] [dependent subprogram ID]] [argument remapping count]
// [[argument ID relative to call] [first uses count] [first use subprogram ID]]
// [argument count] [[argument offset relative to call]] [return statement
// count] [[return statement subprogram ID]]"
std::string UnaryCallExpression::toByteCode(CliArgs args) const {
  std::string bytecode = UnaryExpression::toByteCode(args);

  // -1 so the block (1st in subprogram) is at offset 0
  int subprogramOffset = -function.value().get().id - 1;

  // Write dependent remappings
  bytecode += " " + std::to_string(depRemaps.size());
  for (auto remap : depRemaps) {
    bytecode += " " + std::to_string(remap.first) + " " +
                std::to_string(remap.second.size());
    for (auto dep : remap.second)
      bytecode += " " + std::to_string(dep.get().id + subprogramOffset);
  }

  // Write argument remappings
  auto argMappings = getCallArgMappings(*this);
  bytecode += " " + std::to_string(argMappings.size());
  for (auto remap : argMappings) {
    // Write offset relative to this, +1 to avoid this (the call
    // instruction)
    bytecode +=
        " " + std::to_string(block.expressions[remap.first + 1]->id - id);

    bytecode += " " + std::to_string(remap.second.size());
    for (auto dep : remap.second) {
      bytecode += " " + std::to_string(dep.get().id + subprogramOffset);
    }
  }

  // Write argument offsets
  bytecode += " " + std::to_string(block.expressions.size() - 1);
  for (int i = 1; i < block.expressions.size(); i++) {
    auto expr = block.expressions[i];
    auto declaration = std::static_pointer_cast<BinaryExpression>(expr)->left;
    bytecode += " " + std::to_string(declaration->id - id);
  }

  // Write return statement remappings
  bytecode += " " + std::to_string(function->get().returnStatements.size());
  for (auto returnStatement : function->get().returnStatements) {
    bytecode +=
        " " + std::to_string(returnStatement.get().id + subprogramOffset);
  }

  return bytecode;
}

std::string CallExpression::getFunctionName() const {
  auto call = std::static_pointer_cast<UnaryExpression>(expressions[0]);
  auto nameExpr = std::static_pointer_cast<RootExpression>(call->root);

  return nameExpr->token.raw;
}

static std::shared_ptr<Expression>
convertToDeclaration(std::shared_ptr<Expression> origExpr) {
  std::shared_ptr<Expression> type = std::make_shared<RootExpression>(
      InstructionType::GetIdentifier, origExpr->lineNumber,
      Token({TokenType::Identifier, TokenSubtype::None, "param type unset",
             origExpr->lineNumber}));

  std::shared_ptr<Expression> name = std::make_shared<RootExpression>(
      InstructionType::GetLiteral, origExpr->lineNumber,
      Token({TokenType::Literal, TokenSubtype::String, "param name unset",
             origExpr->lineNumber}));

  std::shared_ptr<Expression> declaration = std::make_shared<BinaryExpression>(
      InstructionType::Declare, origExpr->lineNumber, type, name);

  std::shared_ptr<Expression> set = std::make_shared<BinaryExpression>(
      InstructionType::Set, origExpr->lineNumber, declaration, origExpr);

  return set;
}

static void populateDeclarationWithParam(std::shared_ptr<Expression> origExpr,
                                         FunctionExprParameter param) {
  auto declaration = std::static_pointer_cast<BinaryExpression>(
      std::static_pointer_cast<BinaryExpression>(origExpr)->left);

  auto type = std::static_pointer_cast<RootExpression>(declaration->left);
  auto name = std::static_pointer_cast<RootExpression>(declaration->right);

  type->token.raw = param.type;
  name->token.raw = param.name;
}

void CallExpression::setFunction(
    std::reference_wrapper<FunctionExpression> function) {
  this->function = function;
  getActualCall().function = function;

  // Skip first expression since it's the GetIdentifier expression for the
  // function itself
  for (int i = 1; i < expressions.size(); i++) {
    populateDeclarationWithParam(expressions[i], function.get().params[i - 1]);
  }
}

UnaryCallExpression &CallExpression::getActualCall() {
  return *std::static_pointer_cast<UnaryCallExpression>(expressions[0]).get();
}

void CallExpression::linkInternally() {
  BlockExpression::linkInternally();

  auto &actualCall = getActualCall();

  // Add dependency from argument declarations to the call instruction
  for (int i = 1; i < expressions.size(); i++) {
    auto &declaration =
        std::static_pointer_cast<BinaryExpression>(expressions[i])->left;
    addDependency(*declaration.get(), actualCall);
  }
}

void CallExpression::addArgument(std::shared_ptr<Expression> arg) {
  auto declaration = convertToDeclaration(arg);
  expressions.push_back(declaration);
}
