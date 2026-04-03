#include "../../src/interpreter/bytecodeParser.hpp"
#include "../testUtils.hpp"
#include <gtest/gtest.h>

TEST(buildInstructions, buildsSingleInstruction)
{
  DISABLE_COUT

  std::string text("1 1\n1 2\n4;");
  std::istringstream stream(text);

  auto instrs = std::vector<Instruction>();
  BytecodeParser parser(instrs, stream);
  parser.buildInstructions();

  EXPECT_EQ(instrs.size(), 3);
  auto instr = instrs[0];
  EXPECT_EQ(instr.type, InstructionType::GetLiteral);
  EXPECT_EQ(instr.args.size(), 1);
  EXPECT_FALSE(instr.endsLine);
  EXPECT_EQ(instr.instructionNumber, 1);

  instr = instrs[1];
  EXPECT_EQ(instr.type, InstructionType::GetLiteral);
  EXPECT_EQ(instr.args.size(), 1);
  EXPECT_FALSE(instr.endsLine);
  EXPECT_EQ(instr.instructionNumber, 2);

  instr = instrs[2];
  EXPECT_EQ(instr.type, InstructionType::Add);
  EXPECT_EQ(instr.args.size(), 0);
  EXPECT_TRUE(instr.endsLine);
  EXPECT_EQ(instr.instructionNumber, 3);

  REENABLE_COUT
}

TEST(buildInstructions, buildsMultipleInstructions)
{
  DISABLE_COUT

  std::string text("1 1\n1 2\n4;\n1 3\n1 4\n4;");
  std::istringstream stream(text);

  auto instrs = std::vector<Instruction>();
  BytecodeParser parser(instrs, stream);
  parser.buildInstructions();

  EXPECT_EQ(instrs.size(), 6);
  auto instr = instrs[0];
  EXPECT_EQ(instr.type, InstructionType::GetLiteral);
  EXPECT_EQ(instr.args.size(), 1);
  EXPECT_FALSE(instr.endsLine);
  EXPECT_EQ(instr.instructionNumber, 1);

  instr = instrs[2];
  EXPECT_EQ(instr.type, InstructionType::Add);
  EXPECT_EQ(instr.args.size(), 0);
  EXPECT_TRUE(instr.endsLine);
  EXPECT_EQ(instr.instructionNumber, 3);

  instr = instrs[5];
  EXPECT_EQ(instr.type, InstructionType::Add);
  EXPECT_EQ(instr.args.size(), 0);
  EXPECT_TRUE(instr.endsLine);
  EXPECT_EQ(instr.instructionNumber, 6);

  REENABLE_COUT
}

TEST(buildInstructions, buildsCompoundInstructions)
{
  DISABLE_COUT

  std::string text("1 1\n1 2\n4\n1 3\n5;");
  std::istringstream stream(text);

  auto instrs = std::vector<Instruction>();
  BytecodeParser parser(instrs, stream);
  parser.buildInstructions();

  EXPECT_EQ(instrs.size(), 5);

  auto instr = instrs[0];
  EXPECT_EQ(instr.type, InstructionType::GetLiteral);
  EXPECT_EQ(instr.args.size(), 1);
  EXPECT_FALSE(instr.endsLine);
  EXPECT_EQ(instr.instructionNumber, 1);

  instr = instrs[1];
  EXPECT_EQ(instr.type, InstructionType::GetLiteral);
  EXPECT_EQ(instr.args.size(), 1);
  EXPECT_FALSE(instr.endsLine);
  EXPECT_EQ(instr.instructionNumber, 2);

  instr = instrs[2];
  EXPECT_EQ(instr.type, InstructionType::Add);
  EXPECT_EQ(instr.args.size(), 0);
  EXPECT_FALSE(instr.endsLine);
  EXPECT_EQ(instr.instructionNumber, 3);

  instr = instrs[3];
  EXPECT_EQ(instr.type, InstructionType::GetLiteral);
  EXPECT_EQ(instr.args.size(), 1);
  EXPECT_FALSE(instr.endsLine);
  EXPECT_EQ(instr.instructionNumber, 4);

  instr = instrs[4];
  EXPECT_EQ(instr.type, InstructionType::Subtract);
  EXPECT_EQ(instr.args.size(), 0);
  EXPECT_TRUE(instr.endsLine);
  EXPECT_EQ(instr.instructionNumber, 5);

  REENABLE_COUT
}