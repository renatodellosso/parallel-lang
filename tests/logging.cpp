#include "../src/logging.hpp"
#include "testUtils.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

TEST(logBase, printsWithNoArgs)
{
  auto redirect = redirectCout();

  logBase("type", "where", "msg");

  auto str = restoreCout(std::move(redirect));

  EXPECT_THAT(str, testing::HasSubstr("type"));
  EXPECT_THAT(str, testing::HasSubstr("where"));
  EXPECT_THAT(str, testing::HasSubstr("msg"));
}

TEST(logBase, printsArgs)
{
  auto redirect = redirectCout();

  logBase("type", "where", "msg {:d}", 123);

  auto str = restoreCout(std::move(redirect));

  EXPECT_THAT(str, testing::HasSubstr("123"));
}