#include <gtest/gtest.h>
#include "../../src/compiler/tokenizer.hpp"

std::vector<TokenType> singleCharTypes = {
    TokenType::Semicolon, TokenType::LeftBrace, TokenType::RightBrace,
    TokenType::LeftParen, TokenType::RightParen, TokenType::Plus,
    TokenType::Minus, TokenType::Star, TokenType::Slash, TokenType::Equals,
    TokenType::LessThan, TokenType::GreaterThan};

TEST(Tokenizer, identifiesSingleCharTokenTypes)
{
  std::string text(";{}()+-*/=<>");

  std::istringstream stream(text);
  Tokenizer *tokenizer = new Tokenizer(stream);

  tokenizer->parse();
  auto tokens = *(tokenizer->close().get());

  EXPECT_EQ(tokens.size(), singleCharTypes.size());
  for (int i = 0; i < tokens.size(); i++)
  {
    EXPECT_EQ(tokens[i].type, singleCharTypes[i]);
  }

  // Cleanup
  delete tokenizer;
}

TEST(Tokenizer, identifiesSingleCharTokenTypesWithWhitespace)
{
  std::string text(" ;\n{}\t()\r+  -*/=<>");

  std::istringstream stream(text);
  Tokenizer *tokenizer = new Tokenizer(stream);

  tokenizer->parse();
  auto tokens = *(tokenizer->close().get());

  EXPECT_EQ(tokens.size(), singleCharTypes.size());
  for (int i = 0; i < tokens.size(); i++)
  {
    EXPECT_EQ(tokens[i].type, singleCharTypes[i]);
  }

  // Cleanup
  delete tokenizer;
}