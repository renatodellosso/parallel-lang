#include "../../src/interpreter/bytecodeParser.hpp"
#include "../testUtils.hpp"
#include <format>
#include <gtest/gtest.h>

TEST(buildInstructions, buildsSingleInstruction) {
  DISABLE_COUT

  std::string text(std::format(
      "0 2.0 {} 1\n0 2.1 {} 2\n2  {};", (int)InstructionType::GetLiteral,
      (int)InstructionType::GetLiteral, (int)InstructionType::Add));
  std::istringstream stream(text);

  auto instrs = std::vector<Instruction>();
  BytecodeParser parser({}, instrs, stream);
  parser.buildInstructions();

  ASSERT_EQ(instrs.size(), 3);

  auto instr = instrs[0];
  EXPECT_EQ(instr.type, InstructionType::GetLiteral);
  EXPECT_EQ(instr.bytecodeArgs.size(), 1);
  EXPECT_FALSE(instr.endsLine);
  EXPECT_EQ(instr.id, 0);
  EXPECT_EQ(instr.depCount, 0);
  ASSERT_EQ(instr.dependents.size(), 1);
  EXPECT_EQ(instr.dependents[0].instr, &instrs[2]);
  ASSERT_TRUE(instr.dependents[0].argIndex.has_value());
  EXPECT_EQ(instr.dependents[0].argIndex.value(), 0);

  instr = instrs[1];
  EXPECT_EQ(instr.type, InstructionType::GetLiteral);
  EXPECT_EQ(instr.bytecodeArgs.size(), 1);
  EXPECT_FALSE(instr.endsLine);
  EXPECT_EQ(instr.id, 1);
  EXPECT_EQ(instr.depCount, 0);
  ASSERT_EQ(instr.dependents.size(), 1);
  EXPECT_EQ(instr.dependents[0].instr, &instrs[2]);
  ASSERT_TRUE(instr.dependents[0].argIndex.has_value());
  EXPECT_EQ(instr.dependents[0].argIndex.value(), 1);

  instr = instrs[2];
  EXPECT_EQ(instr.type, InstructionType::Add);
  EXPECT_EQ(instr.bytecodeArgs.size(), 0);
  EXPECT_TRUE(instr.endsLine);
  EXPECT_EQ(instr.id, 2);
  EXPECT_EQ(instr.depCount, 2);
  EXPECT_EQ(instr.dependents.size(), 0);

  REENABLE_COUT
}

TEST(buildInstructions, buildsMultipleInstructions) {
  DISABLE_COUT

  std::string text(std::format(
      "0 2.0 {} 1\n0 2.1 {} 2\n2  {};\n0 5.0 {} 3\n0 5.1 {} 4\n2  {};",
      (int)InstructionType::GetLiteral, (int)InstructionType::GetLiteral,
      (int)InstructionType::Add, (int)InstructionType::GetLiteral,
      (int)InstructionType::GetLiteral, (int)InstructionType::Add));
  std::istringstream stream(text);

  auto instrs = std::vector<Instruction>();
  BytecodeParser parser({}, instrs, stream);
  parser.buildInstructions();

  ASSERT_EQ(instrs.size(), 6);

  auto instr = instrs[0];
  EXPECT_EQ(instr.type, InstructionType::GetLiteral);
  EXPECT_EQ(instr.bytecodeArgs.size(), 1);
  EXPECT_FALSE(instr.endsLine);
  EXPECT_EQ(instr.id, 0);
  EXPECT_EQ(instr.depCount, 0);
  ASSERT_EQ(instr.dependents.size(), 1);
  EXPECT_EQ(instr.dependents[0].instr, &instrs[2]);
  ASSERT_TRUE(instr.dependents[0].argIndex.has_value());
  EXPECT_EQ(instr.dependents[0].argIndex.value(), 0);

  instr = instrs[2];
  EXPECT_EQ(instr.type, InstructionType::Add);
  EXPECT_EQ(instr.bytecodeArgs.size(), 0);
  EXPECT_TRUE(instr.endsLine);
  EXPECT_EQ(instr.id, 2);
  EXPECT_EQ(instr.depCount, 2);
  EXPECT_EQ(instr.dependents.size(), 0);

  instr = instrs[5];
  EXPECT_EQ(instr.type, InstructionType::Add);
  EXPECT_EQ(instr.bytecodeArgs.size(), 0);
  EXPECT_TRUE(instr.endsLine);
  EXPECT_EQ(instr.id, 5);
  EXPECT_EQ(instr.depCount, 2);
  EXPECT_EQ(instr.dependents.size(), 0);

  REENABLE_COUT
}

TEST(buildInstructions, buildsCompoundInstructions) {
  DISABLE_COUT

  std::string text(std::format(
      "0 2.0 {} 1\n0 2.1 {} 2\n2 4.0 {}\n0 4.1 {} 3\n2  {};",
      (int)InstructionType::GetLiteral, (int)InstructionType::GetLiteral,
      (int)InstructionType::Add, (int)InstructionType::GetLiteral,
      (int)InstructionType::Subtract));
  std::istringstream stream(text);

  auto instrs = std::vector<Instruction>();
  BytecodeParser parser({}, instrs, stream);
  parser.buildInstructions();

  ASSERT_EQ(instrs.size(), 5);

  auto instr = instrs[0];
  EXPECT_EQ(instr.type, InstructionType::GetLiteral);
  EXPECT_EQ(instr.bytecodeArgs.size(), 1);
  EXPECT_FALSE(instr.endsLine);
  EXPECT_EQ(instr.id, 0);

  instr = instrs[1];
  EXPECT_EQ(instr.type, InstructionType::GetLiteral);
  EXPECT_EQ(instr.bytecodeArgs.size(), 1);
  EXPECT_FALSE(instr.endsLine);
  EXPECT_EQ(instr.id, 1);

  instr = instrs[2];
  EXPECT_EQ(instr.type, InstructionType::Add);
  EXPECT_EQ(instr.bytecodeArgs.size(), 0);
  EXPECT_FALSE(instr.endsLine);
  EXPECT_EQ(instr.id, 2);
  EXPECT_EQ(instr.depCount, 2);
  ASSERT_EQ(instr.dependents.size(), 1);
  EXPECT_EQ(instr.dependents[0].instr, &instrs[4]);

  instr = instrs[3];
  EXPECT_EQ(instr.type, InstructionType::GetLiteral);
  EXPECT_EQ(instr.bytecodeArgs.size(), 1);
  EXPECT_FALSE(instr.endsLine);
  EXPECT_EQ(instr.id, 3);

  instr = instrs[4];
  EXPECT_EQ(instr.type, InstructionType::Subtract);
  EXPECT_EQ(instr.bytecodeArgs.size(), 0);
  EXPECT_TRUE(instr.endsLine);
  EXPECT_EQ(instr.id, 4);

  REENABLE_COUT
}