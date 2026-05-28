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

std::vector<Instruction> getCompareInstrs(Value left, Value right,
                                          InstructionType type =
                                              InstructionType::CompareEquals,
                                          bool printResult = true) {
  std::vector<Instruction> instrs = printResult
                                        ? std::vector<Instruction>{
                                              Instruction(0), Instruction(1),
                                              Instruction(2), Instruction(3)}
                                        : std::vector<Instruction>{
                                              Instruction(0), Instruction(1),
                                              Instruction(2)};

  instrs[0].type = InstructionType::GetLiteral;
  instrs[0].bytecodeArgs.push_back(left);
  instrs[0].dependents.push_back(InstrDependent(&instrs[2], 0));

  instrs[1].type = InstructionType::GetLiteral;
  instrs[1].bytecodeArgs.push_back(right);
  instrs[1].dependents.push_back(InstrDependent(&instrs[2], 1));

  instrs[2].type = type;
  instrs[2].depCount = 2;

  if (printResult) {
    instrs[2].dependents.push_back(InstrDependent(&instrs[3], 0));
    instrs[3].type = InstructionType::Print;
    instrs[3].depCount = 1;
  }

  return instrs;
}

std::string runCompare(Value left, Value right,
                       InstructionType type = InstructionType::CompareEquals) {
  auto instrs = getCompareInstrs(left, right, type);
  Subprogram program(std::make_shared<std::vector<Instruction>>(instrs));

  Executor executor({.threads = 1}, program);
  DISABLE_COUT
  executor.startExecution();
  auto output = REENABLE_COUT

  return output;
}

void expectCompareThrows(Value left, Value right, InstructionType type) {
  auto instrs = getCompareInstrs(left, right, type, false);
  Subprogram program(std::make_shared<std::vector<Instruction>>(instrs));

  Executor executor({.threads = 1}, program);
  DISABLE_COUT
  EXPECT_THROW(executor.startExecution(), std::runtime_error);
  REENABLE_COUT
}

TEST(startExecution, doesntError) {
  DISABLE_COUT

  auto instrs = getInstrs();
  Subprogram program(std::make_shared<std::vector<Instruction>>(instrs));

  Executor executor({.threads = 1}, program);
  executor.startExecution();

  EXPECT_NO_THROW(executor.startExecution());

  REENABLE_COUT
}

TEST(startExecution, populatesDepArgs) {
  DISABLE_COUT

  auto instrs = getInstrs();
  Subprogram program(std::make_shared<std::vector<Instruction>>(instrs));

  Executor executor({.verbose = true, .threads = 1}, program);
  executor.startExecution();

  auto instr = program.begin() + 2;
  ASSERT_EQ(instr->depArgs.size(), 2);
  EXPECT_EQ(instr->depArgs[0]->type, ValueType::Integer);
  EXPECT_EQ(instr->depArgs[0]->val, instrs[0].bytecodeArgs[0].val);
  EXPECT_EQ(instr->depArgs[1]->type, ValueType::Integer);
  EXPECT_EQ(instr->depArgs[1]->val, instrs[1].bytecodeArgs[0].val);

  REENABLE_COUT
}

TEST(startExecution, compareEqualsComparesIntegers) {
  EXPECT_EQ(runCompare({.type = ValueType::Integer, .val = 1},
                       {.type = ValueType::Integer, .val = 1}),
            "true\n");
  EXPECT_EQ(runCompare({.type = ValueType::Integer, .val = 1},
                       {.type = ValueType::Integer, .val = 2}),
            "false\n");
}

TEST(startExecution, compareEqualsComparesStrings) {
  EXPECT_EQ(runCompare({.type = ValueType::String, .val = std::string("a")},
                       {.type = ValueType::String, .val = std::string("a")}),
            "true\n");
  EXPECT_EQ(runCompare({.type = ValueType::String, .val = std::string("a")},
                       {.type = ValueType::String, .val = std::string("b")}),
            "false\n");
}

TEST(startExecution, compareEqualsComparesBools) {
  EXPECT_EQ(runCompare({.type = ValueType::Bool, .val = true},
                       {.type = ValueType::Bool, .val = true}),
            "true\n");
  EXPECT_EQ(runCompare({.type = ValueType::Bool, .val = true},
                       {.type = ValueType::Bool, .val = false}),
            "false\n");
}

TEST(startExecution, compareEqualsReturnsFalseForDifferentTypes) {
  EXPECT_EQ(runCompare({.type = ValueType::Integer, .val = 1},
                       {.type = ValueType::Bool, .val = true}),
            "false\n");

  auto func = std::shared_ptr<Function>();
  EXPECT_EQ(runCompare({.type = ValueType::Function, .val = func},
                       {.type = ValueType::Integer, .val = 1}),
            "false\n");
}

TEST(startExecution, compareEqualsThrowsForSameTypeNonPrimitives) {
  auto func = std::shared_ptr<Function>();
  auto instrs = getCompareInstrs({.type = ValueType::Function, .val = func},
                                 {.type = ValueType::Function, .val = func},
                                 InstructionType::CompareEquals,
                                 false);
  Subprogram program(std::make_shared<std::vector<Instruction>>(instrs));

  Executor executor({.threads = 1}, program);
  DISABLE_COUT
  EXPECT_THROW(executor.startExecution(), std::runtime_error);
  REENABLE_COUT
}

TEST(startExecution, compareNotEqualsComparesPrimitives) {
  EXPECT_EQ(runCompare({.type = ValueType::Integer, .val = 1},
                       {.type = ValueType::Integer, .val = 2},
                       InstructionType::CompareNotEquals),
            "true\n");
  EXPECT_EQ(runCompare({.type = ValueType::String, .val = std::string("a")},
                       {.type = ValueType::String, .val = std::string("a")},
                       InstructionType::CompareNotEquals),
            "false\n");
  EXPECT_EQ(runCompare({.type = ValueType::Bool, .val = true},
                       {.type = ValueType::Bool, .val = false},
                       InstructionType::CompareNotEquals),
            "true\n");
}

TEST(startExecution, compareNotEqualsReturnsTrueForDifferentTypes) {
  auto func = std::shared_ptr<Function>();
  EXPECT_EQ(runCompare({.type = ValueType::Integer, .val = 1},
                       {.type = ValueType::Bool, .val = true},
                       InstructionType::CompareNotEquals),
            "true\n");
  EXPECT_EQ(runCompare({.type = ValueType::Function, .val = func},
                       {.type = ValueType::Integer, .val = 1},
                       InstructionType::CompareNotEquals),
            "true\n");
}

TEST(startExecution, compareNotEqualsThrowsForSameTypeNonPrimitives) {
  auto func = std::shared_ptr<Function>();
  expectCompareThrows({.type = ValueType::Function, .val = func},
                      {.type = ValueType::Function, .val = func},
                      InstructionType::CompareNotEquals);
}

TEST(startExecution, orderedComparisonsCompareIntegers) {
  EXPECT_EQ(runCompare({.type = ValueType::Integer, .val = 1},
                       {.type = ValueType::Integer, .val = 2},
                       InstructionType::CompareLessThan),
            "true\n");
  EXPECT_EQ(runCompare({.type = ValueType::Integer, .val = 2},
                       {.type = ValueType::Integer, .val = 2},
                       InstructionType::CompareLessThanEquals),
            "true\n");
  EXPECT_EQ(runCompare({.type = ValueType::Integer, .val = 3},
                       {.type = ValueType::Integer, .val = 2},
                       InstructionType::CompareGreaterThan),
            "true\n");
  EXPECT_EQ(runCompare({.type = ValueType::Integer, .val = 2},
                       {.type = ValueType::Integer, .val = 2},
                       InstructionType::CompareGreaterThanEquals),
            "true\n");
  EXPECT_EQ(runCompare({.type = ValueType::Integer, .val = 2},
                       {.type = ValueType::Integer, .val = 1},
                       InstructionType::CompareLessThan),
            "false\n");
}

TEST(startExecution, orderedComparisonsCompareStringsAlphabetically) {
  EXPECT_EQ(runCompare({.type = ValueType::String, .val = std::string("a")},
                       {.type = ValueType::String, .val = std::string("b")},
                       InstructionType::CompareLessThan),
            "true\n");
  EXPECT_EQ(runCompare({.type = ValueType::String, .val = std::string("b")},
                       {.type = ValueType::String, .val = std::string("a")},
                       InstructionType::CompareGreaterThan),
            "true\n");
  EXPECT_EQ(runCompare({.type = ValueType::String, .val = std::string("a")},
                       {.type = ValueType::String, .val = std::string("a")},
                       InstructionType::CompareLessThanEquals),
            "true\n");
  EXPECT_EQ(runCompare({.type = ValueType::String, .val = std::string("b")},
                       {.type = ValueType::String, .val = std::string("b")},
                       InstructionType::CompareGreaterThanEquals),
            "true\n");
}

TEST(startExecution, orderedComparisonsThrowForBools) {
  expectCompareThrows({.type = ValueType::Bool, .val = false},
                      {.type = ValueType::Bool, .val = true},
                      InstructionType::CompareLessThan);
  expectCompareThrows({.type = ValueType::Bool, .val = false},
                      {.type = ValueType::Integer, .val = 1},
                      InstructionType::CompareGreaterThan);
}

TEST(startExecution, orderedComparisonsThrowForDifferentTypes) {
  expectCompareThrows({.type = ValueType::Integer, .val = 1},
                      {.type = ValueType::String, .val = std::string("1")},
                      InstructionType::CompareLessThan);
}

Instruction getFuncInstr() {
  Instruction funcInstr(0);
  funcInstr.depCount = 0;
  funcInstr.type = InstructionType::Function;
  funcInstr.bytecodeArgs = {
      {ValueType::Identifier, "int"},
      {ValueType::Identifier, "func"},

      // Params
      {ValueType::Integer, 2},
      {ValueType::Identifier, "int"},
      {ValueType::Identifier, "a"},
      {ValueType::Identifier, "bool"},
      {ValueType::Identifier, "b"},

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

Instruction getBlockInstr() {
  Instruction block(1);

  block.depCount = 0;
  block.type = InstructionType::Block;

  block.bytecodeArgs = {
      {ValueType::Integer, 0},
  };

  return block;
}

TEST(startExecution, initsFunctions) {
  DISABLE_COUT

  auto instrs = std::make_shared<std::vector<Instruction>>(
      std::vector<Instruction>({getFuncInstr(), getBlockInstr()}));
  auto program = std::make_shared<Subprogram>(instrs);
  program->setSubprogramPointers(program);

  Instruction &funcInstr = instrs->at(0);

  Executor executor({.verbose = true, .threads = 1}, *program.get());
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
