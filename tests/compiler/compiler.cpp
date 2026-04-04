#include "../../src/compiler/compiler.hpp"
#include "../../src/instruction.hpp"
#include "../testUtils.hpp"
#include <gtest/gtest.h>
#include <format>

const CliArgs args = {};

std::string testCompile(std::string program)
{
  DISABLE_COUT

  std::istringstream stream(program);

  std::string out;
  std::function<std::optional<std::string>(std::string)> writeOut = [&out](std::string text)
  {
    out = text;
    return std::nullopt;
  };

  compile(args, stream, writeOut);

  REENABLE_COUT
  return out;
}

TEST(compile, compilesBasicProgram)
{
  auto out = testCompile("1+1;");
  EXPECT_EQ(out, std::format("{} 1\n{} 1\n{};", (int)InstructionType::GetLiteral, (int)InstructionType::GetLiteral, (int)InstructionType::Add));
}

TEST(compile, compilesMultilineProgram)
{
  auto out = testCompile("1+1;\n1-2;");
  EXPECT_EQ(out, std::format("{} 1\n{} 1\n{};\n{} 1\n{} 2\n{};", (int)InstructionType::GetLiteral, (int)InstructionType::GetLiteral, (int)InstructionType::Add,
                             (int)InstructionType::GetLiteral, (int)InstructionType::GetLiteral, (int)InstructionType::Subtract));
}

TEST(compile, compilesCompoundExpressions)
{
  auto out = testCompile("1+1+1;");
  EXPECT_EQ(out, std::format("{} 1\n{} 1\n{}\n{} 1\n{};", (int)InstructionType::GetLiteral, (int)InstructionType::GetLiteral, (int)InstructionType::Add,
                             (int)InstructionType::GetLiteral, (int)InstructionType::Add));
}