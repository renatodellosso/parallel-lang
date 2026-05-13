#include "../../src/compiler/deferredFunctionLinking.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <unordered_set>

TEST(DeferredFunctionLinking, producesConsistentHashes) {
  Instruction instr(0);
  Subprogram program;

  auto func = std::shared_ptr<Function>();
  // Don't use make_shared to avoid the constructor (we'd need a fully-formed
  // set of instructions for that)
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

  auto func = std::shared_ptr<Function>();
  // Don't use make_shared to avoid the constructor (we'd need a fully-formed
  // set of instructions for that)
  auto scope = std::make_shared<Scope<Resource>>();

  DeferredFunctionLinking defer1 = {func, scope};
  DeferredFunctionLinking defer2 = {func, scope};

  std::unordered_set<DeferredFunctionLinking> set;

  set.insert(defer1);
  set.insert(defer2);

  EXPECT_EQ(set.size(), 1);
}