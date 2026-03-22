#pragma once

#include <string>

enum class TokenType
{
  Semicolon,
  String,
  Other
};

struct Token
{
  TokenType type;
  std::string raw;
};
