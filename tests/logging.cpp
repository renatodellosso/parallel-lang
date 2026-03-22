#include "../src/logging.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <iostream>
#include <sstream>
#include <string>
#include <memory>

struct CoutRedirect
{
  std::ostringstream oss;
  std::streambuf *coutBuf;
};

/*
 * Redirects cout into an ostringstream
 */
std::unique_ptr<CoutRedirect> redirectCout()
{
  auto res = std::make_unique<CoutRedirect>();

  // Get old cout
  res.get()->coutBuf = std::cout.rdbuf();
  // Redirect cout
  std::cout.rdbuf(res.get()->oss.rdbuf());

  return std::move(res);
}

/*
 * Returns the string from the redirected cout. Restores cout to its original
 * output.
 */
std::string restoreCout(std::unique_ptr<CoutRedirect> redirect)
{
  // Restore cout
  std::cout.rdbuf(redirect.get()->coutBuf);

  return redirect.get()->oss.str();
}

TEST(redirect, redirectsCout)
{
  auto redirect = redirectCout();

  std::cout << "Hello world";

  auto str = restoreCout(std::move(redirect));
  EXPECT_EQ(str, "Hello world");
}

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