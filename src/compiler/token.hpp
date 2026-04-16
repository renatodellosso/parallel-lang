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
  LessThan,
  GreaterThan,
  Comma,
  Literal,
  Identifier,
  If,
  While
};

enum class TokenSubtype { None, String, Integer, Bool };

struct Token {
  TokenType type;
  TokenSubtype subtype;
  std::string raw;
  int line;
};
