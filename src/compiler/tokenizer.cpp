#include "tokenizer.hpp"
#include <iostream>

static bool isWhitespace(char c)
{
  return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

static bool isAlpha(char c)
{
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

static bool isNumber(char c)
{
  return c >= '0' && c <= '9';
}

static bool isAlphanumeric(char c)
{
  return isAlpha(c) || isAlphanumeric(c);
}

void Tokenizer::skipWhitespace()
{
  for (char c = stream.peek(); !stream.eof() && isWhitespace(c); c = stream.peek())
  {
    stream.get(); // We know it's whitespace, so we read the character
    if (c == '\n' || c == '\r')
      line++;
  }
}

void Tokenizer::parseToken()
{
  skipWhitespace();

  char c = stream.get();
  if (stream.eof())
    return;

  std::string raw;
  raw += c;

  Token token{.type = TokenType::Error, .line = line};

  // Single-char tokens
  switch (c)
  {
  case ';':
    token.type = TokenType::Semicolon;
    break;
  case '{':
    token.type = TokenType::LeftBrace;
    break;
  case '}':
    token.type = TokenType::RightBrace;
    break;
  case '(':
    token.type = TokenType::LeftParen;
    break;
  case ')':
    token.type = TokenType::RightParen;
    break;
  case '+':
    token.type = TokenType::Plus;
    break;
  case '-':
    token.type = TokenType::Minus;
    break;
  case '*':
    token.type = TokenType::Star;
    break;
  case '/':
    token.type = TokenType::Slash;
    break;
  case '=':
    token.type = TokenType::Equals;
    break;
  case '<':
    token.type = TokenType::LessThan;
    break;
  case '>':
    token.type = TokenType::GreaterThan;
    break;

  default:
    break;
  }

  if (token.type != TokenType::Error)
  {
    tokens.get()->push_back(token);
    return;
  }

  std::cout << "(multi-char)";

  // Multi-char tokens

  if (c == '"')
  {
    // String
    for (c = stream.get(); raw.at(raw.size() - 1) != '"' && raw.at(raw.size() - 2) != '\\'; c = stream.get())
    {
      raw += c;
    }

    token.type = TokenType::Literal;
  }
  else if (isNumber(c))
  {
    // Number
    for (c = stream.get(); !stream.eof() && isNumber(c); c = stream.get())
    {
      raw += c;
    }

    token.type = TokenType::Literal;
  }
  else
  {
    for (c = stream.get(); !stream.eof() && isAlphanumeric(c); c = stream.get())
    {
      raw += c;
    }
    token.type = TokenType::Identifier;
  }

  tokens.get()->push_back(token);
}

void Tokenizer::parse()
{
  while (!stream.eof())
  {
    parseToken();
  }
}

std::unique_ptr<std::vector<Token>> Tokenizer::close()
{
  return std::move(tokens);
}
