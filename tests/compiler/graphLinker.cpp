#include "../../src/compiler/graphLinker.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <vector>

void numberExpressions(
    std::shared_ptr<std::vector<std::shared_ptr<Expression>>> expressions) {
  // Must number expressions to properly index during linking!
  int startWith = 0;
  for (auto expr : *expressions) {
    startWith = expr->numberExpressions(startWith);
  }
}

static GraphLinker *makeGraphLinker(
    std::shared_ptr<std::vector<std::shared_ptr<Expression>>> exprs) {
  CliArgs args = {};
  return new GraphLinker(args, exprs);
}

static bool dependsOn(std::shared_ptr<Expression> expr,
                      std::shared_ptr<Expression> dependency) {
  for (auto dep : expr->dependencies) {
    if (&dep.get() == dependency.get())
      return true;
  }

  return false;
}

TEST(constructor, createsDefaultResources) {
  GraphLinker *linker = new GraphLinker(
      std::make_shared<std::vector<std::shared_ptr<Expression>>>());

  auto resources = linker->getResources();
  EXPECT_EQ(resources.size(), 3);
  EXPECT_NE(resources.find("int"), resources.end());
  EXPECT_NE(resources.find("bool"), resources.end());
  EXPECT_NE(resources.find("string"), resources.end());
}

TEST(linkGraph, createsResources) {
  auto type = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetIdentifier, 0,
                     {TokenType::Identifier, TokenSubtype::None, "int", 1}));
  auto name = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 1,
                     {TokenType::Identifier, TokenSubtype::None, "var", 1}));
  auto declaration = std::make_shared<BinaryExpression>(
      BinaryExpression(InstructionType::Declare, 2, type, name));
  auto root = std::make_shared<BlockExpression>(BlockExpression());
  auto expressions =
      std::make_shared<std::vector<std::shared_ptr<Expression>>>();
  expressions->push_back(declaration);

  // Don't forget to number expressions!
  numberExpressions(expressions);

  GraphLinker *linker = new GraphLinker(expressions);
  auto &resources = linker->getResources();
  int originalResourceCount = resources.size();

  linker->linkGraph();

  EXPECT_EQ(linker->getErrors().get()->size(), 0);
  EXPECT_EQ(resources.size(), originalResourceCount + 1);

  auto var = resources.find("var");
  ASSERT_NE(var, resources.end());
  EXPECT_EQ(var->second->name, name.get()->token.raw);
  EXPECT_EQ(var->second->lastWrittenBy, declaration.get());
  ASSERT_EQ(var->second->currAccesses.size(), 1);
  EXPECT_EQ(&var->second->currAccesses[0].get(), declaration.get());

  delete linker;
}

TEST(linkGraph, readsResources) {
  auto type = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetIdentifier, 0,
                     {TokenType::Identifier, TokenSubtype::None, "int", 1}));
  auto declareName = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 0,
                     {TokenType::Identifier, TokenSubtype::None, "var", 1}));
  auto declaration = std::make_shared<BinaryExpression>(
      BinaryExpression(InstructionType::Declare, 0, type, declareName));

  auto refName = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetIdentifier, 0,
                     {TokenType::Identifier, TokenSubtype::None, "var", 1}));

  auto expressions =
      std::make_shared<std::vector<std::shared_ptr<Expression>>>();
  expressions->push_back(declaration);
  expressions->push_back(refName);

  numberExpressions(expressions);

  GraphLinker *linker = new GraphLinker(expressions);
  auto &resources = linker->getResources();
  int originalResourceCount = resources.size();

  linker->linkGraph();

  EXPECT_EQ(linker->getErrors().get()->size(), 0);
  EXPECT_EQ(resources.size(), originalResourceCount + 1);

  auto var = resources.find("var");
  ASSERT_NE(var, resources.end());

  auto lastWrite = var->second->lastWrittenBy;
  ASSERT_EQ(refName.get()->dependencies.size(), 1);
  ASSERT_EQ(lastWrite->dependents.size(), 1);
  EXPECT_EQ(&refName.get()->dependencies[0].get(), lastWrite);
  EXPECT_EQ(&lastWrite->dependents.begin()->expr.get(), refName.get());

  ASSERT_EQ(var->second->currAccesses.size(), 2);
  EXPECT_EQ(&var->second->currAccesses[1].get(), refName.get());
}

TEST(linkGraph, writesResources) {
  auto type = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetIdentifier, 0,
                     {TokenType::Identifier, TokenSubtype::None, "int", 1}));
  auto declareName = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 0,
                     {TokenType::Identifier, TokenSubtype::None, "var", 1}));
  auto declaration = std::make_shared<BinaryExpression>(
      BinaryExpression(InstructionType::Declare, 0, type, declareName));

  auto refName = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetIdentifier, 0,
                     {TokenType::Identifier, TokenSubtype::None, "var", 1}));
  auto literal = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 0,
                     {TokenType::Literal, TokenSubtype::Integer, "1", 1}));
  auto set = std::make_shared<BinaryExpression>(
      BinaryExpression(InstructionType::Set, 0, refName, literal));

  auto expressions =
      std::make_shared<std::vector<std::shared_ptr<Expression>>>();
  expressions->push_back(declaration);
  expressions->push_back(set);

  numberExpressions(expressions);

  GraphLinker *linker = new GraphLinker(expressions);
  auto &resources = linker->getResources();
  int originalResourceCount = resources.size();

  linker->linkGraph();

  EXPECT_EQ(linker->getErrors().get()->size(), 0);
  EXPECT_EQ(resources.size(), originalResourceCount + 1);

  auto var = resources.find("var");
  ASSERT_NE(var, resources.end());

  auto lastWrite = var->second->lastWrittenBy;
  EXPECT_EQ(lastWrite, set.get());

  ASSERT_EQ(var->second->currAccesses.size(), 1);
  EXPECT_EQ(&var->second->currAccesses[0].get(), set.get());

  // Check that set depends on declaration
  ASSERT_EQ(set.get()->dependencies.size(), 3);
  ASSERT_EQ(declaration.get()->dependents.size(), 2);
  EXPECT_EQ(&set.get()->dependencies[0].get(), declaration.get());

  bool refFound, setFound;
  for (auto entry : declaration.get()->dependents) {
    if (&entry.expr.get() == refName.get())
      refFound = true;
    else if (&entry.expr.get() == set.get())
      setFound = true;
  }

  EXPECT_TRUE(refFound);
  EXPECT_TRUE(setFound);
}

TEST(linkGraph, linksInternally) {
  auto left = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 0,
                     {TokenType::Literal, TokenSubtype::Integer, "1", 1}));
  auto right = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 0,
                     {TokenType::Literal, TokenSubtype::Integer, "2", 1}));
  auto binary = std::make_shared<BinaryExpression>(
      BinaryExpression(InstructionType::Add, 0, left, right));

  auto expressions =
      std::make_shared<std::vector<std::shared_ptr<Expression>>>();
  expressions->push_back(binary);

  numberExpressions(expressions);

  GraphLinker *linker = new GraphLinker(expressions);
  auto &resources = linker->getResources();
  int originalResourceCount = resources.size();

  linker->linkGraph();

  EXPECT_EQ(linker->getErrors().get()->size(), 0);

  ASSERT_EQ(binary.get()->dependencies.size(), 2);
  ASSERT_EQ(left.get()->dependents.size(), 1);
  ASSERT_EQ(right.get()->dependents.size(), 1);

  // Check binary -> left, right
  EXPECT_EQ(&binary.get()->dependencies[0].get(), left.get());
  EXPECT_EQ(&binary.get()->dependencies[1].get(), right.get());

  // Check left, right -> binary
  EXPECT_EQ(&left.get()->dependents.begin()->expr.get(), binary.get());
  EXPECT_EQ(&right.get()->dependents.begin()->expr.get(), binary.get());

  // Check arg indices
  ASSERT_TRUE(left.get()->dependents.begin()->argIndex.has_value());
  ASSERT_TRUE(right.get()->dependents.begin()->argIndex.has_value());
  EXPECT_EQ(left.get()->dependents.begin()->argIndex.value(), 0);
  EXPECT_EQ(right.get()->dependents.begin()->argIndex.value(), 1);
}

TEST(linkGraph, linksNestedBlocks) {
  auto base = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 1,
                     {TokenType::Literal, TokenSubtype::Integer, "1", 1}));
  auto inner = std::make_shared<BlockExpression>(BlockExpression({base}, 1));
  auto outer = std::make_shared<BlockExpression>(BlockExpression({inner}, 1));
  auto expressions =
      std::make_shared<std::vector<std::shared_ptr<Expression>>>();
  expressions->push_back(outer);

  // Must number expressions to properly index during linking!
  outer->numberExpressions(0);
  GraphLinker *linker = new GraphLinker(expressions);
  linker->linkGraph();
  delete linker;

  ASSERT_EQ(outer->dependents.size(), 1);
  ASSERT_EQ(inner->dependencies.size(), 1);
  ASSERT_EQ(inner->dependents.size(), 1);
  ASSERT_EQ(base->dependencies.size(), 1);

  EXPECT_EQ(&outer->dependents.begin()->expr.get(), inner.get());
  EXPECT_EQ(&inner->dependencies[0].get(), outer.get());

  EXPECT_EQ(&inner->dependents.begin()->expr.get(), base.get());
  EXPECT_EQ(&base->dependencies[0].get(), inner.get());
}

TEST(linkGraph, allowsVariableShadowing) {
  auto type1 = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetIdentifier, 0,
                     {TokenType::Identifier, TokenSubtype::None, "int", 1}));
  auto name1 = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 0,
                     {TokenType::Identifier, TokenSubtype::None, "var", 1}));
  auto declaration1 = std::make_shared<BinaryExpression>(
      BinaryExpression(InstructionType::Declare, 0, type1, name1));

  auto refName1 = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetIdentifier, 0,
                     {TokenType::Identifier, TokenSubtype::None, "var", 1}));

  auto type2 = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetIdentifier, 0,
                     {TokenType::Identifier, TokenSubtype::None, "int", 1}));
  auto name2 = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 0,
                     {TokenType::Identifier, TokenSubtype::None, "var", 1}));
  auto declaration2 = std::make_shared<BinaryExpression>(
      BinaryExpression(InstructionType::Declare, 0, type2, name2));

  auto refName2 = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetIdentifier, 0,
                     {TokenType::Identifier, TokenSubtype::None, "var", 1}));

  auto block = std::make_shared<BlockExpression>(
      BlockExpression({declaration2, refName2}, 1));
  auto expressions =
      std::make_shared<std::vector<std::shared_ptr<Expression>>>();
  expressions->push_back(declaration1);
  expressions->push_back(block);
  expressions->push_back(refName1);

  numberExpressions(expressions);

  GraphLinker *linker = new GraphLinker(expressions);

  EXPECT_NO_THROW(linker->linkGraph());
  EXPECT_EQ(linker->getErrors()->size(), 0);

  delete linker;

  ASSERT_EQ(declaration1->dependents.size(), 1);
  ASSERT_EQ(refName1->dependencies.size(), 1);

  EXPECT_EQ(&refName1->dependencies[0].get(), declaration1.get());
  EXPECT_EQ(&declaration1->dependents.begin()->expr.get(), refName1.get());

  ASSERT_EQ(declaration2->dependents.size(), 1);
  ASSERT_EQ(refName2->dependencies.size(), 2);

  // Ideally, the block dependency wouldn't exist but that's a later problem
  EXPECT_EQ(&refName2->dependencies[0].get(), block.get());
  EXPECT_EQ(&refName2->dependencies[1].get(), declaration2.get());
  EXPECT_EQ(&declaration2->dependents.begin()->expr.get(), refName2.get());
}

TEST(linkGraph, linksWhileLoops) {
  auto trueLiteral = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 1,
                     {TokenType::Literal, TokenSubtype::Bool, "true", 1}));
  auto condition = std::make_shared<UnaryExpression>(
      UnaryExpression(InstructionType::While, 1, trueLiteral));
  auto getLiteral = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 1,
                     {TokenType::Literal, TokenSubtype::Integer, "99", 1}));
  auto goTo = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GoTo, 1,
                     {TokenType::Literal, TokenSubtype::Integer, "-4", 1}));
  auto block =
      std::make_shared<BlockExpression>(BlockExpression({getLiteral, goTo}, 1));

  auto expressions =
      std::make_shared<std::vector<std::shared_ptr<Expression>>>();
  expressions->push_back(condition);
  expressions->push_back(block);

  // Must number expressions to properly index during linking!
  int startWith = 0;
  for (auto expr : *expressions) {
    startWith = expr->numberExpressions(startWith);
  }

  GraphLinker *linker = new GraphLinker(expressions);
  EXPECT_NO_THROW(linker->linkGraph());
  EXPECT_EQ(linker->getErrors()->size(), 0);

  delete linker;

  ASSERT_EQ(condition->dependents.size(), 1);

  EXPECT_EQ(&condition->dependents.begin()->expr.get(), block.get());

  ASSERT_EQ(block->dependencies.size(), 1);
  EXPECT_EQ(&block->dependencies[0].get(), condition.get());

  ASSERT_EQ(getLiteral->dependencies.size(), 1);
  EXPECT_EQ(&getLiteral->dependencies[0].get(), block.get());

  ASSERT_EQ(goTo->dependencies.size(), 2);
  EXPECT_EQ(&goTo->dependencies[0].get(), block.get());
  EXPECT_EQ(&goTo->dependencies[1].get(), getLiteral.get());
}

TEST(linkGraph, linksWhileLoopsWithFollowingStatements) {
  auto type = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetIdentifier, 0,
                     {TokenType::Identifier, TokenSubtype::None, "int", 1}));
  auto declareName = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 0,
                     {TokenType::Identifier, TokenSubtype::None, "var", 1}));
  auto declaration = std::make_shared<BinaryExpression>(
      BinaryExpression(InstructionType::Declare, 0, type, declareName));

  auto trueLiteral = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 1,
                     {TokenType::Literal, TokenSubtype::Bool, "true", 1}));
  auto condition = std::make_shared<UnaryExpression>(
      UnaryExpression(InstructionType::While, 1, trueLiteral));
  auto getLiteral = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 1,
                     {TokenType::Literal, TokenSubtype::Integer, "99", 1}));
  auto getIdentifier = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetIdentifier, 1,
                     {TokenType::Identifier, TokenSubtype::None, "var", 1}));
  auto set = std::make_shared<BinaryExpression>(
      BinaryExpression(InstructionType::Set, 1, getIdentifier, getLiteral));

  auto goTo = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GoTo, 1,
                     {TokenType::Literal, TokenSubtype::Integer, "-7", 1}));
  auto block =
      std::make_shared<BlockExpression>(BlockExpression({set, goTo}, 1));

  auto getIdentifierAfter = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetIdentifier, 1,
                     {TokenType::Identifier, TokenSubtype::None, "var", 1}));

  auto expressions =
      std::make_shared<std::vector<std::shared_ptr<Expression>>>();
  expressions->push_back(declaration);
  expressions->push_back(condition);
  expressions->push_back(block);
  expressions->push_back(getIdentifierAfter);

  // Must number expressions to properly index during linking!
  int startWith = 0;
  for (auto expr : *expressions) {
    startWith = expr->numberExpressions(startWith);
  }

  GraphLinker *linker = new GraphLinker(expressions);
  EXPECT_NO_THROW(linker->linkGraph());
  EXPECT_EQ(linker->getErrors()->size(), 0);

  delete linker;

  ASSERT_EQ(getIdentifierAfter->dependencies.size(), 1);
  EXPECT_EQ(&getIdentifierAfter->dependencies[0].get(), condition.get());

  ASSERT_EQ(condition->dependents.size(), 2);

  bool blockFound, getFound;
  for (auto entry : condition.get()->dependents) {
    if (&entry.expr.get() == block.get())
      blockFound = true;
    else if (&entry.expr.get() == getIdentifierAfter.get())
      getFound = true;
  }

  EXPECT_TRUE(block);
  EXPECT_TRUE(getFound);
}

TEST(linkGraph, linksElseBranchesFromPreBranchResourceState) {
  auto type = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetIdentifier, 0,
                     {TokenType::Identifier, TokenSubtype::None, "int", 1}));
  auto declareName = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 0,
                     {TokenType::Identifier, TokenSubtype::None, "a", 1}));
  auto declaration = std::make_shared<BinaryExpression>(
      BinaryExpression(InstructionType::Declare, 0, type, declareName));
  auto initialValue = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 0,
                     {TokenType::Literal, TokenSubtype::Integer, "0", 1}));
  auto initialSet = std::make_shared<BinaryExpression>(
      BinaryExpression(InstructionType::Set, 0, declaration, initialValue));

  auto condition = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 1,
                     {TokenType::Literal, TokenSubtype::Bool, "false", 1}));

  auto thenRef = std::make_shared<RootExpression>(
      RootExpression(InstructionType::ReferenceIdentifier, 1,
                     {TokenType::Identifier, TokenSubtype::None, "a", 1}));
  auto thenValue = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 1,
                     {TokenType::Literal, TokenSubtype::Integer, "1", 1}));
  auto thenSet = std::make_shared<BinaryExpression>(
      BinaryExpression(InstructionType::Set, 1, thenRef, thenValue));
  auto thenBlock =
      std::make_shared<BlockExpression>(BlockExpression({thenSet}, 1));

  auto elseRef = std::make_shared<RootExpression>(
      RootExpression(InstructionType::ReferenceIdentifier, 1,
                     {TokenType::Identifier, TokenSubtype::None, "a", 1}));
  auto elseValue = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 1,
                     {TokenType::Literal, TokenSubtype::Integer, "2", 1}));
  auto elseSet = std::make_shared<BinaryExpression>(
      BinaryExpression(InstructionType::Set, 1, elseRef, elseValue));
  auto elseBlock =
      std::make_shared<BlockExpression>(BlockExpression({elseSet}, 1));

  auto ifExpr =
      std::make_shared<IfExpression>(1, condition, thenBlock, elseBlock);

  auto afterRef = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetIdentifier, 2,
                     {TokenType::Identifier, TokenSubtype::None, "a", 2}));
  auto print = std::make_shared<UnaryExpression>(
      UnaryExpression(InstructionType::Print, 2, afterRef));

  auto expressions =
      std::make_shared<std::vector<std::shared_ptr<Expression>>>();
  expressions->push_back(initialSet);
  expressions->push_back(ifExpr);
  expressions->push_back(print);

  numberExpressions(expressions);

  GraphLinker *linker = new GraphLinker(expressions);
  EXPECT_NO_THROW(linker->linkGraph());
  EXPECT_EQ(linker->getErrors()->size(), 0);
  delete linker;

  EXPECT_TRUE(dependsOn(elseRef, initialSet));
  EXPECT_FALSE(dependsOn(elseRef, thenSet));
  EXPECT_TRUE(dependsOn(ifExpr->mergeInstruction, initialSet));
  EXPECT_TRUE(dependsOn(afterRef, ifExpr->mergeInstruction));
}

TEST(linkGraph, linksFollowingStatementsToIfBranchMerge) {
  auto condition = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 1,
                     {TokenType::Literal, TokenSubtype::Bool, "true", 1}));

  auto branchValue = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 1,
                     {TokenType::Literal, TokenSubtype::Integer, "1", 1}));
  auto branchPrint = std::make_shared<UnaryExpression>(
      UnaryExpression(InstructionType::Print, 1, branchValue));
  auto thenBlock =
      std::make_shared<BlockExpression>(BlockExpression({branchPrint}, 1));
  auto ifExpr = std::make_shared<IfExpression>(1, condition, thenBlock);

  auto afterValue = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 2,
                     {TokenType::Literal, TokenSubtype::Integer, "2", 2}));
  auto afterPrint = std::make_shared<UnaryExpression>(
      UnaryExpression(InstructionType::Print, 2, afterValue));

  auto block = std::make_shared<BlockExpression>(
      BlockExpression({ifExpr, afterPrint}, 1));
  auto expressions =
      std::make_shared<std::vector<std::shared_ptr<Expression>>>();
  expressions->push_back(block);

  numberExpressions(expressions);

  GraphLinker *linker = new GraphLinker(expressions);
  EXPECT_NO_THROW(linker->linkGraph());
  EXPECT_EQ(linker->getErrors()->size(), 0);
  delete linker;

  EXPECT_TRUE(dependsOn(afterPrint, ifExpr->mergeInstruction));
}

TEST(linkGraph, linksPrintStatementsWithVariables) {
  auto type = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetIdentifier, 0,
                     {TokenType::Identifier, TokenSubtype::None, "int", 1}));
  auto declareName = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 0,
                     {TokenType::Identifier, TokenSubtype::None, "var", 1}));
  auto declaration = std::make_shared<BinaryExpression>(
      BinaryExpression(InstructionType::Declare, 0, type, declareName));

  auto value = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 1,
                     {TokenType::Literal, TokenSubtype::Integer, "1", 1}));
  auto set = std::make_shared<BinaryExpression>(
      BinaryExpression(InstructionType::Set, 1, declaration, value));

  auto get = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetIdentifier, 2,
                     {TokenType::Identifier, TokenSubtype::None, "var", 2}));
  auto print = std::make_shared<UnaryExpression>(
      UnaryExpression(InstructionType::Print, 2, get));

  auto expressions =
      std::make_shared<std::vector<std::shared_ptr<Expression>>>();
  expressions->push_back(set);
  expressions->push_back(print);

  // Must number expressions to properly index during linking!
  int startWith = 0;
  for (auto expr : *expressions) {
    startWith = expr->numberExpressions(startWith);
  }

  GraphLinker *linker = new GraphLinker(expressions);
  EXPECT_NO_THROW(linker->linkGraph());
  EXPECT_EQ(linker->getErrors()->size(), 0);

  auto resource = linker->getResources().find("var");
  ASSERT_NE(resource, linker->getResources().end());
  EXPECT_EQ(resource->second->lastWrittenBy, set.get());

  ASSERT_EQ(set->dependents.size(), 1);
  ASSERT_EQ(get->dependencies.size(), 1);

  EXPECT_EQ(&set->dependents.begin()->expr.get(), get.get());
  EXPECT_EQ(&get->dependencies[0].get(), set.get());

  delete linker;
}

TEST(linkGraph, linksFunctionsInternally) {
  auto type = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetIdentifier, 0,
                     {TokenType::Identifier, TokenSubtype::None, "int", 1}));
  auto declareName = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 0,
                     {TokenType::Identifier, TokenSubtype::None, "var", 1}));
  auto declaration = std::make_shared<BinaryExpression>(
      BinaryExpression(InstructionType::Declare, 0, type, declareName));

  auto value = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 1,
                     {TokenType::Literal, TokenSubtype::Integer, "1", 1}));
  auto set = std::make_shared<BinaryExpression>(
      BinaryExpression(InstructionType::Set, 1, declaration, value));

  auto get = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetIdentifier, 2,
                     {TokenType::Identifier, TokenSubtype::None, "var", 2}));
  auto print = std::make_shared<UnaryExpression>(
      UnaryExpression(InstructionType::Print, 2, get));

  auto block =
      std::make_shared<BlockExpression>(BlockExpression({set, print}, 1));

  auto func = std::make_shared<FunctionExpression>(
      FunctionExpression("func", "void", 1));
  func->body = block; // Don't forget to set body!

  auto expressions =
      std::make_shared<std::vector<std::shared_ptr<Expression>>>();
  expressions->push_back(func);

  // Must number expressions to properly index during linking!
  int startWith = 0;
  for (auto expr : *expressions) {
    startWith = expr->numberExpressions(startWith);
  }

  GraphLinker *linker = new GraphLinker(expressions);
  EXPECT_NO_THROW(linker->linkGraph());
  EXPECT_EQ(linker->getErrors()->size(), 0);

  ASSERT_EQ(set->dependents.size(), 1);
  ASSERT_EQ(get->dependencies.size(), 2);

  EXPECT_EQ(&set->dependents.begin()->expr.get(), get.get());
  EXPECT_EQ(&get->dependencies[0].get(), block.get());
  EXPECT_EQ(&get->dependencies[1].get(), set.get());

  delete linker;
}

TEST(linkGraph, linksFunctionsNonExternally) {
  auto type = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetIdentifier, 0,
                     {TokenType::Identifier, TokenSubtype::None, "int", 1}));
  auto declareName = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 0,
                     {TokenType::Identifier, TokenSubtype::None, "var", 1}));
  auto declaration = std::make_shared<BinaryExpression>(
      BinaryExpression(InstructionType::Declare, 0, type, declareName));

  auto ref = std::make_shared<RootExpression>(
      RootExpression(InstructionType::ReferenceIdentifier, 1,
                     {TokenType::Identifier, TokenSubtype::None, "var", 1}));
  auto value = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 1,
                     {TokenType::Literal, TokenSubtype::Integer, "1", 1}));
  auto set = std::make_shared<BinaryExpression>(
      BinaryExpression(InstructionType::Set, 1, ref, value));

  auto get = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetIdentifier, 2,
                     {TokenType::Identifier, TokenSubtype::None, "var", 2}));
  auto print = std::make_shared<UnaryExpression>(
      UnaryExpression(InstructionType::Print, 2, get));

  auto block = std::make_shared<BlockExpression>(BlockExpression({set}, 1));

  auto func = std::make_shared<FunctionExpression>(
      FunctionExpression("func", "void", 1));
  func->body = block; // Don't forget to set body!

  auto expressions =
      std::make_shared<std::vector<std::shared_ptr<Expression>>>();
  expressions->push_back(declaration);
  expressions->push_back(func);
  expressions->push_back(print);

  // Must number expressions to properly index during linking!
  int startWith = 0;
  for (auto expr : *expressions) {
    startWith = expr->numberExpressions(startWith);
  }

  GraphLinker *linker = new GraphLinker(expressions);
  EXPECT_NO_THROW(linker->linkGraph());
  EXPECT_EQ(linker->getErrors()->size(), 0);

  EXPECT_EQ(set->dependents.size(), 0);
  ASSERT_EQ(declaration->dependents.size(), 1);
  ASSERT_EQ(get->dependencies.size(), 1);

  EXPECT_EQ(&declaration->dependents.begin()->expr.get(), get.get());
  EXPECT_EQ(&get->dependencies[0].get(), declaration.get());

  delete linker;
}

TEST(linkGraph, linksFunctionsInternallyWithParams) {
  auto ref = std::make_shared<RootExpression>(
      RootExpression(InstructionType::ReferenceIdentifier, 1,
                     {TokenType::Identifier, TokenSubtype::None, "var", 1}));

  auto value = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 1,
                     {TokenType::Literal, TokenSubtype::Integer, "1", 1}));
  auto set = std::make_shared<BinaryExpression>(
      BinaryExpression(InstructionType::Set, 1, ref, value));

  auto get = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetIdentifier, 2,
                     {TokenType::Identifier, TokenSubtype::None, "var", 2}));
  auto print = std::make_shared<UnaryExpression>(
      UnaryExpression(InstructionType::Print, 2, get));

  auto block =
      std::make_shared<BlockExpression>(BlockExpression({set, print}, 1));

  auto func = std::make_shared<FunctionExpression>(
      FunctionExpression("func", "void", 1));
  func->body = block; // Don't forget to set body!
  func->params.emplace_back("int", "var");

  auto expressions =
      std::make_shared<std::vector<std::shared_ptr<Expression>>>();
  expressions->push_back(func);

  // Must number expressions to properly index during linking!
  int startWith = 0;
  for (auto expr : *expressions) {
    startWith = expr->numberExpressions(startWith);
  }

  GraphLinker *linker = new GraphLinker(expressions);
  EXPECT_NO_THROW(linker->linkGraph());
  EXPECT_EQ(linker->getErrors()->size(), 0);

  ASSERT_EQ(set->dependents.size(), 1);
  ASSERT_EQ(get->dependencies.size(), 2);

  EXPECT_EQ(&set->dependents.begin()->expr.get(), get.get());
  EXPECT_EQ(&get->dependencies[0].get(), block.get());
  EXPECT_EQ(&get->dependencies[1].get(), set.get());

  delete linker;
}

TEST(linkGraph, linksFunctionsWithExternalVariables) {
  auto type = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetIdentifier, 0,
                     {TokenType::Identifier, TokenSubtype::None, "int", 1}));
  auto declareName = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 0,
                     {TokenType::Identifier, TokenSubtype::None, "var", 1}));
  auto declaration = std::make_shared<BinaryExpression>(
      BinaryExpression(InstructionType::Declare, 0, type, declareName));

  auto ref = std::make_shared<RootExpression>(
      RootExpression(InstructionType::ReferenceIdentifier, 2,
                     {TokenType::Identifier, TokenSubtype::None, "var", 2}));
  auto value = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 1,
                     {TokenType::Literal, TokenSubtype::Integer, "1", 1}));
  auto set = std::make_shared<BinaryExpression>(
      BinaryExpression(InstructionType::Set, 1, ref, value));

  auto get = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetIdentifier, 2,
                     {TokenType::Identifier, TokenSubtype::None, "var", 2}));
  auto print = std::make_shared<UnaryExpression>(
      UnaryExpression(InstructionType::Print, 2, get));

  auto block =
      std::make_shared<BlockExpression>(BlockExpression({set, print}, 1));

  auto func = std::make_shared<FunctionExpression>(
      FunctionExpression("func", "void", 1));
  func->body = block; // Don't forget to set body!

  auto expressions =
      std::make_shared<std::vector<std::shared_ptr<Expression>>>();
  expressions->push_back(declaration);
  expressions->push_back(func);

  // Must number expressions to properly index during linking!
  int startWith = 0;
  for (auto expr : *expressions) {
    startWith = expr->numberExpressions(startWith);
  }

  GraphLinker *linker = new GraphLinker(expressions);
  EXPECT_NO_THROW(linker->linkGraph());
  EXPECT_EQ(linker->getErrors()->size(), 0);

  EXPECT_EQ(declaration->dependents.size(), 0);
  ASSERT_EQ(ref->dependencies.size(), 1);
  EXPECT_EQ(&ref->dependencies[0].get(), block.get());

  delete linker;
}

TEST(linkGraph, setsFunctionFirstAndLastUsesAndWrites) {
  auto type = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetIdentifier, 0,
                     {TokenType::Identifier, TokenSubtype::None, "int", 1}));
  auto declareName = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 0,
                     {TokenType::Identifier, TokenSubtype::None, "var", 1}));
  auto declaration = std::make_shared<BinaryExpression>(
      BinaryExpression(InstructionType::Declare, 0, type, declareName));

  auto ref = std::make_shared<RootExpression>(
      RootExpression(InstructionType::ReferenceIdentifier, 2,
                     {TokenType::Identifier, TokenSubtype::None, "var", 2}));
  auto value = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 1,
                     {TokenType::Literal, TokenSubtype::Integer, "1", 1}));
  auto set = std::make_shared<BinaryExpression>(
      BinaryExpression(InstructionType::Set, 1, ref, value));

  auto get = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetIdentifier, 2,
                     {TokenType::Identifier, TokenSubtype::None, "var", 2}));
  auto print = std::make_shared<UnaryExpression>(
      UnaryExpression(InstructionType::Print, 2, get));

  auto block =
      std::make_shared<BlockExpression>(BlockExpression({set, print}, 1));

  auto func = std::make_shared<FunctionExpression>(
      FunctionExpression("func", "void", 1));
  func->body = block; // Don't forget to set body!

  auto expressions =
      std::make_shared<std::vector<std::shared_ptr<Expression>>>();
  expressions->push_back(declaration);
  expressions->push_back(func);

  // Must number expressions to properly index during linking!
  int startWith = 0;
  for (auto expr : *expressions) {
    startWith = expr->numberExpressions(startWith);
  }

  GraphLinker *linker = new GraphLinker(expressions);
  EXPECT_NO_THROW(linker->linkGraph());
  EXPECT_EQ(linker->getErrors()->size(), 0);

  EXPECT_TRUE(func->firstUses.contains("var"));
  EXPECT_TRUE(func->firstWrites.contains("var"));
  EXPECT_TRUE(func->lastUses.contains("var"));
  EXPECT_TRUE(func->lastWrites.contains("var"));

  ASSERT_EQ(func->firstUses["var"].size(), 1);
  EXPECT_EQ(&func->firstUses["var"][0].get(), ref.get());

  EXPECT_EQ(&func->firstWrites.at("var").get(), set.get());

  ASSERT_EQ(func->lastUses["var"].size(), 2);
  EXPECT_EQ(&func->lastUses["var"][0].get(), set.get());
  EXPECT_EQ(&func->lastUses["var"][1].get(), get.get());

  EXPECT_EQ(&func->lastWrites.at("var").get(), set.get());

  delete linker;
}

TEST(linkGraph, setsFunctionFinishedLinking) {
  auto type = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetIdentifier, 0,
                     {TokenType::Identifier, TokenSubtype::None, "int", 1}));
  auto declareName = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 0,
                     {TokenType::Identifier, TokenSubtype::None, "var", 1}));
  auto declaration = std::make_shared<BinaryExpression>(
      BinaryExpression(InstructionType::Declare, 0, type, declareName));

  auto ref = std::make_shared<RootExpression>(
      RootExpression(InstructionType::ReferenceIdentifier, 2,
                     {TokenType::Identifier, TokenSubtype::None, "var", 2}));
  auto value = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 1,
                     {TokenType::Literal, TokenSubtype::Integer, "1", 1}));
  auto set = std::make_shared<BinaryExpression>(
      BinaryExpression(InstructionType::Set, 1, ref, value));

  auto get = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetIdentifier, 2,
                     {TokenType::Identifier, TokenSubtype::None, "var", 2}));
  auto print = std::make_shared<UnaryExpression>(
      UnaryExpression(InstructionType::Print, 2, get));

  auto block =
      std::make_shared<BlockExpression>(BlockExpression({set, print}, 1));

  auto func = std::make_shared<FunctionExpression>(
      FunctionExpression("func", "void", 1));
  func->body = block; // Don't forget to set body!

  auto expressions =
      std::make_shared<std::vector<std::shared_ptr<Expression>>>();
  expressions->push_back(declaration);
  expressions->push_back(func);

  // Must number expressions to properly index during linking!
  int startWith = 0;
  for (auto expr : *expressions) {
    startWith = expr->numberExpressions(startWith);
  }

  GraphLinker *linker = new GraphLinker(expressions);
  EXPECT_NO_THROW(linker->linkGraph());
  EXPECT_EQ(linker->getErrors()->size(), 0);

  EXPECT_TRUE(func->finishedLinking);

  delete linker;
}

TEST(linkGraph, createsFunctionResource) {
  auto type = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetIdentifier, 0,
                     {TokenType::Identifier, TokenSubtype::None, "int", 1}));
  auto declareName = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 0,
                     {TokenType::Identifier, TokenSubtype::None, "var", 1}));
  auto declaration = std::make_shared<BinaryExpression>(
      BinaryExpression(InstructionType::Declare, 0, type, declareName));

  auto block = std::make_shared<BlockExpression>(BlockExpression({}, 1));

  auto func = std::make_shared<FunctionExpression>(
      FunctionExpression("func", "void", 1));
  func->body = block; // Don't forget to set body!

  auto expressions =
      std::make_shared<std::vector<std::shared_ptr<Expression>>>();
  expressions->push_back(func);

  // Must number expressions to properly index during linking!
  int startWith = 0;
  for (auto expr : *expressions) {
    startWith = expr->numberExpressions(startWith);
  }

  GraphLinker *linker = new GraphLinker(expressions);
  EXPECT_NO_THROW(linker->linkGraph());
  EXPECT_EQ(linker->getErrors()->size(), 0);

  auto &resources = linker->getResources();

  ASSERT_TRUE(resources.contains("func"));
  auto resource = resources["func"];
  ASSERT_TRUE(resource->function.has_value());
  EXPECT_EQ(&resource->function.value().get(), func.get());

  delete linker;
}

TEST(linkGraph, linksCallDependenciesWithoutParameters) {
  auto type = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetIdentifier, 0,
                     {TokenType::Identifier, TokenSubtype::None, "int", 1}));
  auto declareName = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 0,
                     {TokenType::Identifier, TokenSubtype::None, "var", 1}));
  auto declaration = std::make_shared<BinaryExpression>(
      BinaryExpression(InstructionType::Declare, 0, type, declareName));

  auto ref = std::make_shared<RootExpression>(
      RootExpression(InstructionType::ReferenceIdentifier, 2,
                     {TokenType::Identifier, TokenSubtype::None, "var", 2}));
  auto value = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 1,
                     {TokenType::Literal, TokenSubtype::Integer, "1", 1}));
  auto set = std::make_shared<BinaryExpression>(
      BinaryExpression(InstructionType::Set, 1, ref, value));

  auto get = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetIdentifier, 2,
                     {TokenType::Identifier, TokenSubtype::None, "var", 2}));
  auto print = std::make_shared<UnaryExpression>(
      UnaryExpression(InstructionType::Print, 2, get));

  auto block =
      std::make_shared<BlockExpression>(BlockExpression({set, print}, 1));

  auto func = std::make_shared<FunctionExpression>(
      FunctionExpression("func", "void", 1));
  func->body = block; // Don't forget to set body!

  Token token = {TokenType::Identifier, TokenSubtype::None, "func", 3};
  auto getFunc = std::make_shared<RootExpression>(
      InstructionType::GetIdentifier, 3, token);
  auto call = std::make_shared<CallExpression>(getFunc, 3);

  auto expressions =
      std::make_shared<std::vector<std::shared_ptr<Expression>>>();
  expressions->push_back(declaration);
  expressions->push_back(func);
  expressions->push_back(call);

  // Must number expressions to properly index during linking!
  int startWith = 0;
  for (auto expr : *expressions) {
    startWith = expr->numberExpressions(startWith);
  }

  GraphLinker *linker = new GraphLinker(expressions);
  EXPECT_NO_THROW(linker->linkGraph());
  EXPECT_EQ(linker->getErrors()->size(), 0);

  ASSERT_EQ(func->dependents.size(), 2); // Block and call
  ASSERT_EQ(call->getActualCall().dependencies.size(),
            2); // The GetIdentifier already depends on the function

  bool getFound;
  for (auto entry : func.get()->dependents) {
    if (&entry.expr.get() == getFunc.get())
      getFound = true;
  }

  EXPECT_TRUE(getFound);

  EXPECT_EQ(&call->getActualCall().dependencies[0].get(), declaration.get());
  EXPECT_EQ(&call->getActualCall().dependencies[1].get(),
            getFunc.get()); // Internal links are after external links

  ASSERT_EQ(declaration->dependents.size(), 1);
  EXPECT_EQ(&declaration->dependents.begin()->expr.get(),
            &call->getActualCall());

  ASSERT_EQ(getFunc->dependencies.size(),
            2); // 1st dep is the enclosing call block
  EXPECT_EQ(&getFunc->dependencies[1].get(), func.get());

  ASSERT_EQ(getFunc->dependents.size(), 1);
  EXPECT_EQ(&getFunc->dependents.begin()->expr.get(), &call->getActualCall());
  EXPECT_EQ(getFunc->dependents.begin()->argIndex.value(), 0);

  delete linker;
}

TEST(linkGraph, linksCallDependentsWithoutParameters) {
  auto type = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetIdentifier, 0,
                     {TokenType::Identifier, TokenSubtype::None, "int", 1}));
  auto declareName = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 0,
                     {TokenType::Identifier, TokenSubtype::None, "var", 1}));
  auto declaration = std::make_shared<BinaryExpression>(
      BinaryExpression(InstructionType::Declare, 0, type, declareName));

  auto ref = std::make_shared<RootExpression>(
      RootExpression(InstructionType::ReferenceIdentifier, 2,
                     {TokenType::Identifier, TokenSubtype::None, "var", 2}));
  auto value = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 1,
                     {TokenType::Literal, TokenSubtype::Integer, "1", 1}));
  auto set = std::make_shared<BinaryExpression>(
      BinaryExpression(InstructionType::Set, 1, ref, value));

  auto block = std::make_shared<BlockExpression>(BlockExpression({set}, 1));

  auto func = std::make_shared<FunctionExpression>(
      FunctionExpression("func", "void", 1));
  func->body = block; // Don't forget to set body!

  Token token = {TokenType::Identifier, TokenSubtype::None, "func", 3};
  auto getFunc = std::make_shared<RootExpression>(
      InstructionType::GetIdentifier, 3, token);
  auto call = std::make_shared<CallExpression>(getFunc, 3);

  auto get = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetIdentifier, 2,
                     {TokenType::Identifier, TokenSubtype::None, "var", 2}));
  auto print = std::make_shared<UnaryExpression>(
      UnaryExpression(InstructionType::Print, 2, get));

  auto expressions =
      std::make_shared<std::vector<std::shared_ptr<Expression>>>();
  expressions->push_back(declaration);
  expressions->push_back(func);
  expressions->push_back(call);
  expressions->push_back(print);

  // Must number expressions to properly index during linking!
  int startWith = 0;
  for (auto expr : *expressions) {
    startWith = expr->numberExpressions(startWith);
  }

  GraphLinker *linker = new GraphLinker(expressions);
  EXPECT_NO_THROW(linker->linkGraph());
  EXPECT_EQ(linker->getErrors()->size(), 0);

  ASSERT_EQ(call->getActualCall().dependents.size(), 1);
  ASSERT_EQ(get->dependencies.size(), 1);

  EXPECT_EQ(&call->getActualCall().dependents.begin()->expr.get(), get.get());
  EXPECT_EQ(&get->dependencies[0].get(), &call->getActualCall());

  delete linker;
}

TEST(linkGraph, setsCallFunction) {
  auto type = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetIdentifier, 0,
                     {TokenType::Identifier, TokenSubtype::None, "int", 1}));
  auto declareName = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 0,
                     {TokenType::Identifier, TokenSubtype::None, "var", 1}));
  auto declaration = std::make_shared<BinaryExpression>(
      BinaryExpression(InstructionType::Declare, 0, type, declareName));

  auto ref = std::make_shared<RootExpression>(
      RootExpression(InstructionType::ReferenceIdentifier, 2,
                     {TokenType::Identifier, TokenSubtype::None, "var", 2}));
  auto value = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 1,
                     {TokenType::Literal, TokenSubtype::Integer, "1", 1}));
  auto set = std::make_shared<BinaryExpression>(
      BinaryExpression(InstructionType::Set, 1, ref, value));

  auto block = std::make_shared<BlockExpression>(BlockExpression({set}, 1));

  auto func = std::make_shared<FunctionExpression>(
      FunctionExpression("func", "void", 1));
  func->body = block; // Don't forget to set body!

  Token token = {TokenType::Identifier, TokenSubtype::None, "func", 3};
  auto getFunc = std::make_shared<RootExpression>(
      InstructionType::GetIdentifier, 3, token);
  auto call = std::make_shared<CallExpression>(getFunc, 3);

  auto get = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetIdentifier, 2,
                     {TokenType::Identifier, TokenSubtype::None, "var", 2}));
  auto print = std::make_shared<UnaryExpression>(
      UnaryExpression(InstructionType::Print, 2, get));

  auto expressions =
      std::make_shared<std::vector<std::shared_ptr<Expression>>>();
  expressions->push_back(declaration);
  expressions->push_back(func);
  expressions->push_back(call);
  expressions->push_back(print);

  // Must number expressions to properly index during linking!
  int startWith = 0;
  for (auto expr : *expressions) {
    startWith = expr->numberExpressions(startWith);
  }

  GraphLinker *linker = new GraphLinker(expressions);
  EXPECT_NO_THROW(linker->linkGraph());
  EXPECT_EQ(linker->getErrors()->size(), 0);

  EXPECT_EQ(&call->function.value().get(), func.get());
  EXPECT_EQ(&call->getActualCall().function.value().get(), func.get());

  delete linker;
}

TEST(linkGraph, setsCallDepRemaps) {
  auto type = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetIdentifier, 0,
                     {TokenType::Identifier, TokenSubtype::None, "int", 1}));
  auto declareName = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 0,
                     {TokenType::Identifier, TokenSubtype::None, "var", 1}));
  auto declaration = std::make_shared<BinaryExpression>(
      BinaryExpression(InstructionType::Declare, 0, type, declareName));

  auto ref = std::make_shared<RootExpression>(
      RootExpression(InstructionType::ReferenceIdentifier, 2,
                     {TokenType::Identifier, TokenSubtype::None, "var", 2}));
  auto value = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetLiteral, 1,
                     {TokenType::Literal, TokenSubtype::Integer, "1", 1}));
  auto set = std::make_shared<BinaryExpression>(
      BinaryExpression(InstructionType::Set, 1, ref, value));

  auto block = std::make_shared<BlockExpression>(BlockExpression({set}, 1));

  auto func = std::make_shared<FunctionExpression>(
      FunctionExpression("func", "void", 1));
  func->body = block; // Don't forget to set body!

  Token token = {TokenType::Identifier, TokenSubtype::None, "func", 3};
  auto getFunc = std::make_shared<RootExpression>(
      InstructionType::GetIdentifier, 3, token);
  auto call = std::make_shared<CallExpression>(getFunc, 3);

  auto get = std::make_shared<RootExpression>(
      RootExpression(InstructionType::GetIdentifier, 2,
                     {TokenType::Identifier, TokenSubtype::None, "var", 2}));
  auto print = std::make_shared<UnaryExpression>(
      UnaryExpression(InstructionType::Print, 2, get));

  auto expressions =
      std::make_shared<std::vector<std::shared_ptr<Expression>>>();
  expressions->push_back(declaration);
  expressions->push_back(func);
  expressions->push_back(call);
  expressions->push_back(print);

  // Must number expressions to properly index during linking!
  int startWith = 0;
  for (auto expr : *expressions) {
    startWith = expr->numberExpressions(startWith);
  }

  GraphLinker *linker = new GraphLinker(expressions);
  EXPECT_NO_THROW(linker->linkGraph());
  EXPECT_EQ(linker->getErrors()->size(), 0);

  ASSERT_EQ(call->getActualCall().depRemaps.size(), 1);

  auto remap = call->getActualCall().depRemaps.find(0);
  ASSERT_NE(remap, call->getActualCall().depRemaps.end());

  ASSERT_EQ(remap->second.size(), 1);
  EXPECT_EQ(&remap->second[0].get(), set.get());

  delete linker;
}
