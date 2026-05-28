#include "../../src/compiler/compiler.hpp"
#include "../../src/instruction.hpp"
#include "../testUtils.hpp"
#include <format>
#include <gtest/gtest.h>

const CliArgs args = {};

std::string testCompile(std::string program) {
  DISABLE_COUT

  std::istringstream stream(program);

  std::string out;
  std::function<std::optional<std::string>(std::string)> writeOut =
      [&out](std::string text) {
        out = text;
        return std::nullopt;
      };

  compile(args, stream, writeOut);

  REENABLE_COUT
  return out;
}

void expectCompileBinary(std::string program, InstructionType type) {
  auto out = testCompile(program);
  EXPECT_EQ(out, std::format("0 2.0 {} 1\n0 2.1 {} 1\n2  {}",
                             (int)InstructionType::GetLiteral,
                             (int)InstructionType::GetLiteral, (int)type));
}

TEST(compile, compilesBasicProgram) {
  expectCompileBinary("1 + 1", InstructionType::Add);
}

TEST(compile, compilesEqualsExpressions) {
  expectCompileBinary("1 == 1", InstructionType::CompareEquals);
}

TEST(compile, compilesComparisonExpressions) {
  expectCompileBinary("1 != 1", InstructionType::CompareNotEquals);
  expectCompileBinary("1 < 1", InstructionType::CompareLessThan);
  expectCompileBinary("1 <= 1", InstructionType::CompareLessThanEquals);
  expectCompileBinary("1 > 1", InstructionType::CompareGreaterThan);
  expectCompileBinary("1 >= 1", InstructionType::CompareGreaterThanEquals);
}

TEST(compile, compilesMultilineProgram) {
  auto out = testCompile("1 + 1;\n1 - 2");
  EXPECT_EQ(
      out,
      std::format(
          "0 2.0 {} 1\n0 2.1 {} 1\n2  {}\n0 5.0 {} 1\n0 5.1 {} 2\n2  {}",
          (int)InstructionType::GetLiteral, (int)InstructionType::GetLiteral,
          (int)InstructionType::Add, (int)InstructionType::GetLiteral,
          (int)InstructionType::GetLiteral, (int)InstructionType::Subtract));
}

TEST(compile, compilesCompoundExpressions) {
  auto out = testCompile("1 + 1 + 1");
  EXPECT_EQ(out,
            std::format("0 4.0 {} 1\n0 3.0 {} 1\n0 3.1 {} 1\n2 4.1 {}\n2  {}",
                        (int)InstructionType::GetLiteral,
                        (int)InstructionType::GetLiteral,
                        (int)InstructionType::GetLiteral,
                        (int)InstructionType::Add, (int)InstructionType::Add));
}

TEST(compile, compilesBlockExpressions) {
  auto out = testCompile("{1 + 1;}");
  EXPECT_EQ(out, std::format("0 2,1 {} 3\n1 3.0 {} 1\n1 3.1 {} 1\n2  {}",
                             (int)InstructionType::Block,
                             (int)InstructionType::GetLiteral,
                             (int)InstructionType::GetLiteral,
                             (int)InstructionType::Add));
}

TEST(compile, compilesElseStatements) {
  auto out = testCompile("if (true) print \"then\"; else print \"else\";");

  EXPECT_NE(out.find(std::format(" {}", (int)InstructionType::If)),
            std::string::npos);
  EXPECT_NE(out.find(std::format(" {}", (int)InstructionType::Else)),
            std::string::npos);
  EXPECT_NE(out.find("then"), std::string::npos);
  EXPECT_NE(out.find("else"), std::string::npos);
}
