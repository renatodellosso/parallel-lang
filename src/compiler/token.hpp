#pragma once

#include <string>

enum class TokenType
{
  Error,
  Semicolon,
  LeftParen,
  RightParen,
  LeftBrace,
  RightBrace,
  Plus,
  Minus,
  Star,
  Slash,
  Exclamation,
  Equals,
  LessThan,
  GreaterThan,
  Comma,
  Literal,
  Identifier
};

struct Token
{
  TokenType type;
  std::string raw;
  int line;
};
