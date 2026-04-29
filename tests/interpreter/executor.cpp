#include "../../src/interpreter/executor.hpp"
#include "../../src/interpreter/function.hpp"
#include "../testUtils.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <vector>

std::vector<Instruction> getInstrs() {
  DISABLE_COUT

  std::vector<Instruction> instrs = {Instruction(0), Instruction(1),
                                     Instruction(2)};

  instrs[0].type = InstructionType::GetLiteral;
  instrs[0].bytecodeArgs.push_back({.type = ValueType::Integer, .val = 1});
  instrs[0].dependents.push_back(InstrDependent(&instrs[2], 0));

  instrs[1].type = InstructionType::GetLiteral;
  instrs[1].bytecodeArgs.push_back({.type = ValueType::Integer, .val = 2});
  instrs[1].dependents.push_back(InstrDependent(&instrs[2], 1));

  instrs[2].type = InstructionType::Add;
  instrs[2].depCount = 2;

  REENABLE_COUT
  return instrs;
}

TEST(startExecution, doesntError) {
  DISABLE_COUT

  auto instrs = getInstrs();

  Executor executor({.threads = 1}, instrs);
  executor.startExecution();

  EXPECT_NO_THROW(executor.startExecution());

  REENABLE_COUT
}

TEST(startExecution, populatesDepArgs) {
  DISABLE_COUT

  auto instrs = getInstrs();

  Executor executor({.verbose = true, .threads = 1}, instrs);
  executor.startExecution();

  ASSERT_EQ(instrs[2].depArgs.size(), 2);
  EXPECT_EQ(instrs[2].depArgs[0]->type, ValueType::Integer);
  EXPECT_EQ(instrs[2].depArgs[0]->val, instrs[0].bytecodeArgs[0].val);
  EXPECT_EQ(instrs[2].depArgs[1]->type, ValueType::Integer);
  EXPECT_EQ(instrs[2].depArgs[1]->val, instrs[1].bytecodeArgs[0].val);

  REENABLE_COUT
}

Instruction getFuncInstr() {
  Instruction funcInstr(0);
  funcInstr.depCount = 0;
  funcInstr.type = InstructionType::Function;
  funcInstr.bytecodeArgs = {
      {ValueType::Identifier, "int"},
      {ValueType::Identifier, "func"},

      // First uses
      {ValueType::Integer, 2},
      {ValueType::Identifier, "var1"},
      {ValueType::Integer, 1},
      {ValueType::Integer, 0},
      {ValueType::Identifier, "var2"},
      {ValueType::Integer, 1},
      {ValueType::Integer, 0},

      // First writes
      {ValueType::Integer, 2},
      {ValueType::Identifier, "a"},
      {ValueType::Integer, 0},
      {ValueType::Identifier, "b"},
      {ValueType::Integer, 0},

      // Last uses
      {ValueType::Integer, 2},
      {ValueType::Identifier, "var3"},
      {ValueType::Integer, 1},
      {ValueType::Integer, 0},
      {ValueType::Identifier, "var4"},
      {ValueType::Integer, 1},
      {ValueType::Integer, 0},

      // Last writes
      {ValueType::Integer, 2},
      {ValueType::Identifier, "c"},
      {ValueType::Integer, 0},
      {ValueType::Identifier, "d"},
      {ValueType::Integer, 0},
  };

  return funcInstr;
}

TEST(startExecution, initsFunctions) {
  DISABLE_COUT

  std::vector<Instruction> instrs = {getFuncInstr()};

  Instruction &funcInstr = instrs[0];

  Executor executor({.verbose = true, .threads = 1}, instrs);
  executor.startExecution();

  auto funcVal = funcInstr.scope->get("func");
  EXPECT_EQ(funcVal->type, ValueType::Function);
  ASSERT_NE(std::get<std::shared_ptr<Function>>(funcVal->val), nullptr);

  auto func = std::get<std::shared_ptr<Function>>(funcVal->val);
  ASSERT_NE(func, nullptr);

  EXPECT_EQ(func->getReturnType(),
            std::get<std::string>(funcInstr.bytecodeArgs[0].val));
  EXPECT_EQ(func->getName(),
            std::get<std::string>(funcInstr.bytecodeArgs[1].val));

  REENABLE_COUT
}

TEST(startExecution, initsFunctionsWithFirstUses) {
  DISABLE_COUT

  std::vector<Instruction> instrs = {getFuncInstr()};

  Instruction &funcInstr = instrs[0];

  Executor executor({.verbose = true, .threads = 1}, instrs);
  executor.startExecution();

  auto funcVal = funcInstr.scope->get("func");
  EXPECT_EQ(funcVal->type, ValueType::Function);
  ASSERT_NE(std::get<std::shared_ptr<Function>>(funcVal->val), nullptr);

  auto func = std::get<std::shared_ptr<Function>>(funcVal->val);
  ASSERT_NE(func, nullptr);

  auto uses = func->getFirstUses();
  ASSERT_EQ(uses.size(), 2);

  ASSERT_TRUE(uses.contains("var1"));
  ASSERT_EQ(uses["var1"].size(), 1);
  EXPECT_EQ(&uses["var1"][0].get(), &funcInstr);

  ASSERT_TRUE(uses.contains("var2"));
  ASSERT_EQ(uses["var2"].size(), 1);
  EXPECT_EQ(&uses["var2"][0].get(), &funcInstr);

  REENABLE_COUT
}

TEST(startExecution, initsFunctionsWithLastUses) {
  DISABLE_COUT

  std::vector<Instruction> instrs = {getFuncInstr()};

  Instruction &funcInstr = instrs[0];

  Executor executor({.verbose = true, .threads = 1}, instrs);
  executor.startExecution();

  auto funcVal = funcInstr.scope->get("func");
  EXPECT_EQ(funcVal->type, ValueType::Function);
  ASSERT_NE(std::get<std::shared_ptr<Function>>(funcVal->val), nullptr);

  auto func = std::get<std::shared_ptr<Function>>(funcVal->val);
  ASSERT_NE(func, nullptr);

  auto uses = func->getLastUses();
  ASSERT_EQ(uses.size(), 2);

  ASSERT_TRUE(uses.contains("var3"));
  ASSERT_EQ(uses["var3"].size(), 1);
  EXPECT_EQ(&uses["var3"][0].get(), &funcInstr);

  ASSERT_TRUE(uses.contains("var4"));
  ASSERT_EQ(uses["var4"].size(), 1);
  EXPECT_EQ(&uses["var4"][0].get(), &funcInstr);

  REENABLE_COUT
}

TEST(startExecution, initsFunctionsWithFirstWrites) {
  DISABLE_COUT

  std::vector<Instruction> instrs = {getFuncInstr()};

  Instruction &funcInstr = instrs[0];

  Executor executor({.verbose = true, .threads = 1}, instrs);
  executor.startExecution();

  auto funcVal = funcInstr.scope->get("func");
  EXPECT_EQ(funcVal->type, ValueType::Function);
  ASSERT_NE(std::get<std::shared_ptr<Function>>(funcVal->val), nullptr);

  auto func = std::get<std::shared_ptr<Function>>(funcVal->val);
  ASSERT_NE(func, nullptr);

  auto writes = func->getFirstWrites();
  ASSERT_EQ(writes.size(), 2);

  ASSERT_TRUE(writes.contains("a"));
  EXPECT_EQ(&writes.find("a")->second.get(), &funcInstr);

  ASSERT_TRUE(writes.contains("b"));
  EXPECT_EQ(&writes.find("b")->second.get(), &funcInstr);

  REENABLE_COUT
}

TEST(startExecution, initsFunctionsWithLastWrites) {
  DISABLE_COUT

  std::vector<Instruction> instrs = {getFuncInstr()};

  Instruction &funcInstr = instrs[0];

  Executor executor({.verbose = true, .threads = 1}, instrs);
  executor.startExecution();

  auto funcVal = funcInstr.scope->get("func");
  EXPECT_EQ(funcVal->type, ValueType::Function);
  ASSERT_NE(std::get<std::shared_ptr<Function>>(funcVal->val), nullptr);

  auto func = std::get<std::shared_ptr<Function>>(funcVal->val);
  ASSERT_NE(func, nullptr);

  auto writes = func->getLastWrites();
  ASSERT_EQ(writes.size(), 2);

  ASSERT_TRUE(writes.contains("c"));
  EXPECT_EQ(&writes.find("c")->second.get(), &funcInstr);

  ASSERT_TRUE(writes.contains("d"));
  EXPECT_EQ(&writes.find("d")->second.get(), &funcInstr);

  REENABLE_COUT
}