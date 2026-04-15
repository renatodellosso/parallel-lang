#include "../../src/compiler/graphLinker.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <vector>

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
      RootExpression(InstructionType::GetLiteral, 0,
                     {TokenType::Identifier, TokenSubtype::None, "var", 1}));
  auto declaration = std::make_shared<BinaryExpression>(
      BinaryExpression(InstructionType::Declare, 0, type, name));
  auto root = std::make_shared<BlockExpression>(BlockExpression());
  auto expressions =
      std::make_shared<std::vector<std::shared_ptr<Expression>>>();
  expressions->push_back(declaration);

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
  EXPECT_EQ(&lastWrite->dependents[0].expr.get(), refName.get());

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
  EXPECT_EQ(&declaration.get()->dependents[0].expr.get(), refName.get());
  EXPECT_EQ(&declaration.get()->dependents[1].expr.get(), set.get());
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
  EXPECT_EQ(&left.get()->dependents[0].expr.get(), binary.get());
  EXPECT_EQ(&right.get()->dependents[0].expr.get(), binary.get());

  // Check arg indices
  ASSERT_TRUE(left.get()->dependents[0].argIndex.has_value());
  ASSERT_TRUE(right.get()->dependents[0].argIndex.has_value());
  EXPECT_EQ(left.get()->dependents[0].argIndex.value(), 0);
  EXPECT_EQ(right.get()->dependents[0].argIndex.value(), 1);
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

  EXPECT_EQ(&outer->dependents[0].expr.get(), inner.get());
  EXPECT_EQ(&inner->dependencies[0].get(), outer.get());

  EXPECT_EQ(&inner->dependents[0].expr.get(), base.get());
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

  // Must number expressions to properly index during linking!
  block->numberExpressions(0);
  GraphLinker *linker = new GraphLinker(expressions);

  EXPECT_NO_THROW(linker->linkGraph());
  EXPECT_EQ(linker->getErrors()->size(), 0);

  delete linker;

  ASSERT_EQ(declaration1->dependents.size(), 1);
  ASSERT_EQ(refName1->dependencies.size(), 1);

  EXPECT_EQ(&refName1->dependencies[0].get(), declaration1.get());
  EXPECT_EQ(&declaration1->dependents[0].expr.get(), refName1.get());

  ASSERT_EQ(declaration2->dependents.size(), 1);
  ASSERT_EQ(refName2->dependencies.size(), 1);

  EXPECT_EQ(&refName2->dependencies[0].get(), declaration2.get());
  EXPECT_EQ(&declaration2->dependents[0].expr.get(), refName2.get());
}