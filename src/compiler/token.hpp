#pragma once

#include <string>

enum class TokenType {
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
  EqualsEquals,
  NotEquals,
  LessThan,
  LessThanEquals,
  GreaterThan,
  GreaterThanEquals,
  Comma,
  Literal,
  Identifier,
  If,
  While,
  Print,
  Else
};

enum class TokenSubtype { None, String, Integer, Bool };

struct Token {
  TokenType type;
  TokenSubtype subtype;
  std::string raw;
  int line;
};
