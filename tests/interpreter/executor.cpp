#include "../../src/interpreter/executor.hpp"
#include <gtest/gtest.h>

std::vector<Instruction> getInstrs() {
  std::vector<Instruction> instrs = {Instruction(0), Instruction(1),
                                     Instruction(2)};

  instrs[0].type = InstructionType::GetLiteral;
  instrs[0].bytecodeArgs.push_back({.type = ValueType::Integer, .val = 1});
  instrs[0].dependents.push_back(InstrDependent(2, 0));

  instrs[1].type = InstructionType::GetLiteral;
  instrs[1].bytecodeArgs.push_back({.type = ValueType::Integer, .val = 2});
  instrs[1].dependents.push_back(InstrDependent(2, 1));

  instrs[2].type = InstructionType::Add;
  instrs[2].depCount = 2;

  return instrs;
}

TEST(startExecution, doesntError) {
  auto instrs = getInstrs();

  Executor executor({.threads = 1}, instrs);
  executor.startExecution();

  EXPECT_NO_THROW(executor.startExecution());
}

TEST(startExecution, updatesDepsFulfilled) {
  auto instrs = getInstrs();

  Executor executor({.threads = 1}, instrs);
  executor.startExecution();

  EXPECT_EQ(instrs[2].depsFulfilled, 2);
}

TEST(startExecution, populatesDepArgs) {
  auto instrs = getInstrs();

  Executor executor({.threads = 1}, instrs);
  executor.startExecution();

  ASSERT_EQ(instrs[2].depArgs.size(), 2);
  EXPECT_EQ(instrs[2].depArgs[0].type, ValueType::Integer);
  EXPECT_EQ(instrs[2].depArgs[0].val, instrs[0].bytecodeArgs[0].val);
  EXPECT_EQ(instrs[2].depArgs[1].type, ValueType::Integer);
  EXPECT_EQ(instrs[2].depArgs[1].val, instrs[1].bytecodeArgs[0].val);
}