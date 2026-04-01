#include "../../src/compiler/compiler.hpp"
#include <gtest/gtest.h>

const CliArgs args = {};

std::string testCompile(std::string program)
{
  std::istringstream stream(program);

  std::string out;
  std::function<std::optional<std::string>(std::string)> writeOut = [&out](std::string text)
  {
    out = text;
    return std::nullopt;
  };

  compile(args, stream, writeOut);

  return out;
}

TEST(compile, compilesBasicProgram)
{
  auto out = testCompile("1+1;");
  EXPECT_EQ(out, "1 1\n1 1\n4;");
}

TEST(compile, compilesMultilineProgram)
{
  auto out = testCompile("1+1;\n1-2;");
  EXPECT_EQ(out, "1 1\n1 1\n4;\n1 1\n1 2\n5;");
}

TEST(compile, compilesCompoundExpressions)
{
  auto out = testCompile("1+1+1;");
  EXPECT_EQ(out, "1 1\n1 1\n4\n1 1\n4;");
}