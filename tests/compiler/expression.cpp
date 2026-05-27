#include "../../src/compiler/expression.hpp"
#include <gtest/gtest.h>

TEST(ExprDependent, CustomHasherIsConsistentWithoutArgIndices) {
  Expression expr(InstructionType::Block, 0);
  ExprDependent dep(expr);

  std::hash<ExprDependent> hasher;
  auto origHash = hasher(dep);

  for (int i = 0; i < 100; i++)
    ASSERT_EQ(hasher(dep), origHash);
}

TEST(ExprDependent, CustomHasherIsConsistentWithArgIndices) {
  Expression expr(InstructionType::Block, 0);
  ExprDependent dep(expr, 1);

  std::hash<ExprDependent> hasher;
  auto origHash = hasher(dep);

  for (int i = 0; i < 100; i++)
    ASSERT_EQ(hasher(dep), origHash);
}

TEST(ExprDependent, preventsDuplicatesInUnorderedSets) {
  Expression expr(InstructionType::Block, 0);
  ExprDependent dep(expr, 1);

  std::unordered_set<ExprDependent> set;

  set.insert(dep);
  set.insert(dep);

  EXPECT_EQ(set.size(), 1);
}

TEST(ExprDependent, allowsDifferentInUnorderedSets) {
  Expression expr1(InstructionType::Block, 0);
  Expression expr2(InstructionType::Block, 1);
  ExprDependent dep1(expr1, 1);
  ExprDependent dep2(expr1, 2);
  ExprDependent dep3(expr1);
  ExprDependent dep4(expr2);

  std::unordered_set<ExprDependent> set;

  set.insert(dep1);
  set.insert(dep2);
  set.insert(dep3);
  set.insert(dep4);

  EXPECT_EQ(set.size(), 4);
}