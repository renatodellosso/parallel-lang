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

  EXPECT_EQ(&condition->dependents[0].expr.get(), block.get());

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
  EXPECT_EQ(&condition->dependents[0].expr.get(), block.get());
  EXPECT_EQ(&condition->dependents[1].expr.get(), getIdentifierAfter.get());
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

  EXPECT_EQ(&set->dependents[0].expr.get(), get.get());
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

  EXPECT_EQ(&set->dependents[0].expr.get(), get.get());
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

  EXPECT_EQ(&declaration->dependents[0].expr.get(), get.get());
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

  EXPECT_EQ(&set->dependents[0].expr.get(), get.get());
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
  std::vector<std::shared_ptr<Expression>> callExprs = {getFunc};
  auto call = std::make_shared<CallExpression>(callExprs, 3);

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
  ASSERT_EQ(call->dependencies.size(),
            2); // The GetIdentifier already depends on the function

  EXPECT_EQ(&func->dependents[1].expr.get(), getFunc.get());

  EXPECT_EQ(&call->dependencies[0].get(), declaration.get());
  EXPECT_EQ(&call->dependencies[1].get(),
            getFunc.get()); // Internal links are after external links

  ASSERT_EQ(declaration->dependents.size(), 1);
  EXPECT_EQ(&declaration->dependents[0].expr.get(), call.get());

  ASSERT_EQ(getFunc->dependencies.size(), 1);
  EXPECT_EQ(&getFunc->dependencies[0].get(), func.get());

  ASSERT_EQ(getFunc->dependents.size(), 1);
  EXPECT_EQ(&getFunc->dependents[0].expr.get(), call.get());
  EXPECT_EQ(getFunc->dependents[0].argIndex.value(), 0);

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
  std::vector<std::shared_ptr<Expression>> callExprs = {getFunc};
  auto call = std::make_shared<CallExpression>(callExprs, 3);

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

  ASSERT_EQ(call->dependents.size(), 1);
  ASSERT_EQ(get->dependencies.size(), 1);

  EXPECT_EQ(&call->dependents[0].expr.get(), get.get());
  EXPECT_EQ(&get->dependencies[0].get(), call.get());

  delete linker;
}