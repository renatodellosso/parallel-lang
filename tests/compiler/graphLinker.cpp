#include "../../src/compiler/graphLinker.hpp"
#include <gtest/gtest.h>

TEST(constructor, createsDefaultResources)
{
  GraphLinker *linker = new GraphLinker(std::make_shared<BlockExpression>(BlockExpression()));

  auto resources = linker->getResources();
  EXPECT_EQ(resources.size(), 3);
  EXPECT_NE(resources.find("int"), resources.end());
  EXPECT_NE(resources.find("bool"), resources.end());
  EXPECT_NE(resources.find("string"), resources.end());
}

TEST(linkGraph, createsResources)
{
  auto type = std::make_shared<RootExpression>(RootExpression(InstructionType::GetIdentifier, 0, {TokenType::Identifier, TokenSubtype::None, "int", 1}));
  auto name = std::make_shared<RootExpression>(RootExpression(InstructionType::ReferenceIdentifier, 0, {TokenType::Identifier, TokenSubtype::None, "var", 1}));
  auto declaration = std::make_shared<BinaryExpression>(BinaryExpression(InstructionType::Declare, 0, type, name));
  auto root = std::make_shared<BlockExpression>(BlockExpression());
  root.get()->expressions.push_back(declaration);

  GraphLinker *linker = new GraphLinker(root);
  auto &resources = linker->getResources();
  int originalResourceCount = resources.size();

  linker->linkGraph();

  EXPECT_EQ(linker->getErrors().get()->size(), 0);
  EXPECT_EQ(resources.size(), originalResourceCount + 1);

  auto var = resources.find("var");
  ASSERT_NE(var, resources.end());
  EXPECT_EQ(var->second.name, name.get()->token.raw);
  EXPECT_EQ(var->second.lastWrittenBy, declaration.get());
  ASSERT_EQ(var->second.currAccesses.size(), 1);
  EXPECT_EQ(&var->second.currAccesses[0].get(), declaration.get());
}

TEST(linkGraph, readsResources)
{
  auto type = std::make_shared<RootExpression>(RootExpression(InstructionType::GetIdentifier, 0, {TokenType::Identifier, TokenSubtype::None, "int", 1}));
  auto declareName = std::make_shared<RootExpression>(RootExpression(InstructionType::ReferenceIdentifier, 0, {TokenType::Identifier, TokenSubtype::None, "var", 1}));
  auto declaration = std::make_shared<BinaryExpression>(BinaryExpression(InstructionType::Declare, 0, type, declareName));

  auto refName = std::make_shared<RootExpression>(RootExpression(InstructionType::GetIdentifier, 0, {TokenType::Identifier, TokenSubtype::None, "var", 1}));

  auto root = std::make_shared<BlockExpression>(BlockExpression());
  root.get()->expressions.push_back(declaration);
  root.get()->expressions.push_back(refName);

  GraphLinker *linker = new GraphLinker(root);
  auto &resources = linker->getResources();
  int originalResourceCount = resources.size();

  linker->linkGraph();

  EXPECT_EQ(linker->getErrors().get()->size(), 0);
  EXPECT_EQ(resources.size(), originalResourceCount + 1);

  auto var = resources.find("var");
  ASSERT_NE(var, resources.end());

  auto lastWrite = var->second.lastWrittenBy;
  ASSERT_EQ(refName.get()->dependencies.size(), 1);
  ASSERT_EQ(lastWrite->dependents.size(), 1);
  EXPECT_EQ(&refName.get()->dependencies[0].get(), lastWrite);
  EXPECT_EQ(&lastWrite->dependents[0].get(), refName.get());

  ASSERT_EQ(var->second.currAccesses.size(), 2);
  EXPECT_EQ(&var->second.currAccesses[1].get(), refName.get());
}

TEST(linkGraph, writesResources)
{
  auto type = std::make_shared<RootExpression>(RootExpression(InstructionType::GetIdentifier, 0, {TokenType::Identifier, TokenSubtype::None, "int", 1}));
  auto declareName = std::make_shared<RootExpression>(RootExpression(InstructionType::ReferenceIdentifier, 0, {TokenType::Identifier, TokenSubtype::None, "var", 1}));
  auto declaration = std::make_shared<BinaryExpression>(BinaryExpression(InstructionType::Declare, 0, type, declareName));

  auto refName = std::make_shared<RootExpression>(RootExpression(InstructionType::ReferenceIdentifier, 0, {TokenType::Identifier, TokenSubtype::None, "var", 1}));
  auto literal = std::make_shared<RootExpression>(RootExpression(InstructionType::GetLiteral, 0, {TokenType::Literal, TokenSubtype::Integer, "1", 1}));
  auto set = std::make_shared<BinaryExpression>(BinaryExpression(InstructionType::Set, 0, refName, literal));

  auto root = std::make_shared<BlockExpression>(BlockExpression());
  root.get()->expressions.push_back(declaration);
  root.get()->expressions.push_back(set);

  GraphLinker *linker = new GraphLinker(root);
  auto &resources = linker->getResources();
  int originalResourceCount = resources.size();

  linker->linkGraph();

  EXPECT_EQ(linker->getErrors().get()->size(), 0);
  EXPECT_EQ(resources.size(), originalResourceCount + 1);

  auto var = resources.find("var");
  ASSERT_NE(var, resources.end());

  auto lastWrite = var->second.lastWrittenBy;
  EXPECT_EQ(lastWrite, set.get());

  ASSERT_EQ(var->second.currAccesses.size(), 1);
  EXPECT_EQ(&var->second.currAccesses[0].get(), set.get());

  // Check that set depends on declaration
  ASSERT_EQ(set.get()->dependencies.size(), 3);
  ASSERT_EQ(declaration.get()->dependents.size(), 1);
  EXPECT_EQ(&set.get()->dependencies[0].get(), declaration.get());
  EXPECT_EQ(&declaration.get()->dependents[0].get(), set.get());
}