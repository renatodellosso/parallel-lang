#include "../src/cli.hpp"
#include "testUtils.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

TEST(parseArgs, parsesCliMode)
{
  int argc = 2;

  char *argv[] = {(char *)"exec", (char *)"--compile"};

  auto res = parseArgs(argc, argv);
  EXPECT_EQ(CliMode::Compile, res.mode);

  argv[1] = (char *)"--interpret";
  res = parseArgs(argc, argv);
  EXPECT_EQ(res.mode, CliMode::Interpret);
}

TEST(parseArgs, parsesTarget)
{
  int argc = 3;

  char *argv[] = {(char *)"exec", (char *)"--target", (char *)"test"};

  auto res = parseArgs(argc, argv);
  EXPECT_EQ(res.target, "test");

  argv[1] = (char *)"-t";
  res = parseArgs(argc, argv);
  EXPECT_EQ(res.target, "test");
}

TEST(parseArgs, handlesUnknownArgs)
{
  int argc = 2;

  char *argv[] = {(char *)"exec", (char *)"--ofiuhiudoaiwdoais"};

  auto redirect = redirectCout();
  auto res = parseArgs(argc, argv);
  auto out = restoreCout(std::move(redirect));

  EXPECT_THAT(out, testing::HasSubstr("Unknown CLI argument"));
}

TEST(parseArgs, handlesKnownAndUnknownArgs)
{
  int argc = 3;

  char *argv[] = {(char *)"exec", (char *)"--ofiuhiudoaiwdoais", (char *)"--compile"};

  auto redirect = redirectCout();
  auto res = parseArgs(argc, argv);
  auto out = restoreCout(std::move(redirect));

  EXPECT_EQ(res.mode, CliMode::Compile);
  EXPECT_THAT(out, testing::HasSubstr("Unknown CLI argument"));
}

TEST(parseArgs, parsesShortcuts)
{
  int argc = 2;

  char *argv[] = {(char *)"exec", (char *)"-c"};

  auto res = parseArgs(argc, argv);
  EXPECT_EQ(res.mode, CliMode::Compile);
}