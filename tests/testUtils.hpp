#pragma once

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
std::unique_ptr<CoutRedirect> redirectCout();

/*
 * Returns the string from the redirected cout. Restores cout to its original
 * output.
 */
std::string restoreCout(std::unique_ptr<CoutRedirect> redirect);