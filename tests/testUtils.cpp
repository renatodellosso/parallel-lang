#include "testUtils.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

std::unique_ptr<CoutRedirect> redirectCout() {
  auto res = std::make_unique<CoutRedirect>();

  // Get old cout
  res.get()->coutBuf = std::cout.rdbuf();
  // Redirect cout
  std::cout.rdbuf(res.get()->oss.rdbuf());

  return std::move(res);
}

std::string restoreCout(std::unique_ptr<CoutRedirect> redirect) {
  // Restore cout
  std::cout.rdbuf(redirect.get()->coutBuf);

  return redirect.get()->oss.str();
}

TEST(redirect, redirectsCout) {
  auto redirect = redirectCout();

  std::cout << "Hello world";

  auto str = restoreCout(std::move(redirect));
  EXPECT_EQ(str, "Hello world");
}