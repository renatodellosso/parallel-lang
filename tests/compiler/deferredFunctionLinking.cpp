#include "../../src/compiler/deferredFunctionLinking.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <unordered_set>
#include <vector>

std::shared_ptr<Function> makeFunction() {
  Instruction first(0);
  first.bytecodeArgs = {{ValueType::String, "returnType"},
                        {ValueType::String, "name"}};

  Instruction second(1);
  second.bytecodeArgs = {{ValueType::Integer, 0}};

  auto instructions = std::make_shared<std::vector<Instruction>>();
  instructions->push_back(first);
  instructions->push_back(second);

  Subprogram program(instructions);

  return std::make_shared<Function>(first, program);
}

TEST(DeferredFunctionLinking, producesConsistentHashes) {
  Instruction instr(0);
  Subprogram program;

  auto func = makeFunction();
  auto scope = std::make_shared<Scope<Resource>>();

  DeferredFunctionLinking defer = {func, scope};

  std::hash<DeferredFunctionLinking> hasher;
  auto origHash = hasher({func, scope});

  for (int i = 0; i < 100; i++)
    ASSERT_EQ(hasher({func, scope}), origHash);
}

TEST(DeferredFunctionLinking, preventsDuplicatesInUnorderedSets) {
  Instruction instr(0);
  Subprogram program;

  auto func = makeFunction();
  auto scope = std::make_shared<Scope<Resource>>();

  DeferredFunctionLinking defer1 = {func, scope};
  DeferredFunctionLinking defer2 = {func, scope};

  std::unordered_set<DeferredFunctionLinking> set;

  set.insert(defer1);
  set.insert(defer2);

  EXPECT_EQ(set.size(), 1);
}

TEST(DeferredFunctionLinking, allowsDifferentInUnorderedSets) {
  Instruction instr(0);
  Subprogram program;

  auto func1 = makeFunction();
  auto func2 = makeFunction();
  auto scope = std::make_shared<Scope<Resource>>();

  DeferredFunctionLinking defer1 = {func1, scope};
  DeferredFunctionLinking defer2 = {func2, scope};

  std::unordered_set<DeferredFunctionLinking> set;

  set.insert(defer1);
  set.insert(defer2);

  EXPECT_EQ(set.size(), 2);
}