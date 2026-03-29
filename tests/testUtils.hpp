#pragma once

#include "../src/compiler/token.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <memory>

#define DISABLE_COUT auto macro_coutRedirect = redirectCout();
#define REENABLE_COUT restoreCout(std::move(macro_coutRedirect));

struct CoutRedirect
{
  std::ostringstream oss;
  std::streambuf *coutBuf;
};

/*
 * Redirects cout into an ostringstream
 */
std::unique_ptr<CoutRedirect> redirectCout();

/*
 * Returns the string from the redirected cout. Restores cout to its original
 * output.
 */
std::string restoreCout(std::unique_ptr<CoutRedirect> redirect);

#define EXPECT_TOKEN_EQ(lhs, rhs)      \
  EXPECT_EQ(lhs.type, rhs.type);       \
  EXPECT_EQ(lhs.subtype, rhs.subtype); \
  EXPECT_EQ(lhs.line, rhs.line);       \
  EXPECT_EQ(lhs.raw, rhs.raw);