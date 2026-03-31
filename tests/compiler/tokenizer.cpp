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

  ASSERT_EQ(tokens.size(), singleCharTypes.size());
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

  ASSERT_EQ(tokens.size(), singleCharTypes.size());
  for (int i = 0; i < tokens.size(); i++)
  {
    EXPECT_EQ(tokens[i].type, singleCharTypes[i]);
  }

  // Cleanup
  delete tokenizer;
}

TEST(Tokenizer, tracksLineNumbers)
{
  std::string text(";\n;;\r;");

  std::istringstream stream(text);
  Tokenizer *tokenizer = new Tokenizer(stream);

  tokenizer->parse();
  auto tokens = *(tokenizer->close().get());

  ASSERT_EQ(tokens.size(), 4);
  EXPECT_EQ(tokens[0].line, 1);
  EXPECT_EQ(tokens[1].line, 2);
  EXPECT_EQ(tokens[2].line, 2);
  EXPECT_EQ(tokens[3].line, 3);

  // Cleanup
  delete tokenizer;
}

TEST(Tokenizer, identifiesStrings)
{
  std::string text("\"abc\"");

  std::istringstream stream(text);
  Tokenizer *tokenizer = new Tokenizer(stream);

  tokenizer->parse();
  auto tokens = *(tokenizer->close().get());

  ASSERT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens[0].type, TokenType::Literal);
  EXPECT_EQ(tokens[0].subtype, TokenSubtype::String);
  EXPECT_EQ(tokens[0].raw, text);

  // Cleanup
  delete tokenizer;
}

TEST(Tokenizer, identifiesStringsMixedWithOtherTypes)
{
  std::string text(";\"abc\",");

  std::istringstream stream(text);
  Tokenizer *tokenizer = new Tokenizer(stream);

  tokenizer->parse();
  auto tokens = *(tokenizer->close().get());

  ASSERT_EQ(tokens.size(), 3);
  EXPECT_EQ(tokens[1].type, TokenType::Literal);
  EXPECT_EQ(tokens[1].subtype, TokenSubtype::String);
  EXPECT_EQ(tokens[1].raw, "\"abc\"");

  // Cleanup
  delete tokenizer;
}

TEST(Tokenizer, handlesWhitespaceInsideStrings)
{
  std::string text("\"a b\nc\r\"");

  std::istringstream stream(text);
  Tokenizer *tokenizer = new Tokenizer(stream);

  tokenizer->parse();
  auto tokens = *(tokenizer->close().get());

  ASSERT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens[0].type, TokenType::Literal);
  EXPECT_EQ(tokens[0].subtype, TokenSubtype::String);
  EXPECT_EQ(tokens[0].raw, text);

  // Cleanup
  delete tokenizer;
}

TEST(Tokenizer, tracksLineNumbersInMultilineStrings)
{
  char *rawText = new char[]{
      '"', '\n', '"', ' ', '"', 'a', '"', '\0'};
  std::string text(rawText);

  std::istringstream stream(text);
  Tokenizer *tokenizer = new Tokenizer(stream);

  tokenizer->parse();
  auto tokens = *(tokenizer->close().get());

  ASSERT_EQ(tokens.size(), 2);
  EXPECT_EQ(tokens[0].line, 1);
  EXPECT_EQ(tokens[1].line, 2);

  // Cleanup
  delete tokenizer;
}

TEST(Tokenizer, identifiesNumbers)
{
  std::string text("123");

  std::istringstream stream(text);
  Tokenizer *tokenizer = new Tokenizer(stream);

  tokenizer->parse();
  auto tokens = *(tokenizer->close().get());

  ASSERT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens[0].type, TokenType::Literal);
  EXPECT_EQ(tokens[0].subtype, TokenSubtype::Number);
  EXPECT_EQ(tokens[0].raw, text);

  // Cleanup
  delete tokenizer;
}

TEST(Tokenizer, identifiesNumbersMixedWithOtherTypes)
{
  std::string text(";123)");

  std::istringstream stream(text);
  Tokenizer *tokenizer = new Tokenizer(stream);

  tokenizer->parse();
  auto tokens = *(tokenizer->close().get());

  ASSERT_EQ(tokens.size(), 3);
  EXPECT_EQ(tokens[1].type, TokenType::Literal);
  EXPECT_EQ(tokens[1].subtype, TokenSubtype::Number);
  EXPECT_EQ(tokens[1].raw, "123");

  // Cleanup
  delete tokenizer;
}

TEST(Tokenizer, identifiesBooleans)
{
  std::string text("false");

  std::istringstream stream(text);
  Tokenizer *tokenizer = new Tokenizer(stream);

  tokenizer->parse();
  auto tokens = *(tokenizer->close().get());

  ASSERT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens[0].type, TokenType::Literal);
  EXPECT_EQ(tokens[0].subtype, TokenSubtype::Bool);
  EXPECT_EQ(tokens[0].raw, text);

  // Cleanup
  delete tokenizer;
}

TEST(Tokenizer, identifiesBooleanssMixedWithOtherTypes)
{
  std::string text(";false)");

  std::istringstream stream(text);
  Tokenizer *tokenizer = new Tokenizer(stream);

  tokenizer->parse();
  auto tokens = *(tokenizer->close().get());

  ASSERT_EQ(tokens.size(), 3);
  EXPECT_EQ(tokens[1].type, TokenType::Literal);
  EXPECT_EQ(tokens[1].subtype, TokenSubtype::Bool);
  EXPECT_EQ(tokens[1].raw, "false");

  // Cleanup
  delete tokenizer;
}

TEST(Tokenizer, identifiesIdentifiers)
{
  std::string text("abc");

  std::istringstream stream(text);
  Tokenizer *tokenizer = new Tokenizer(stream);

  tokenizer->parse();
  auto tokens = *(tokenizer->close().get());

  ASSERT_EQ(tokens.size(), 1);
  EXPECT_EQ(tokens[0].type, TokenType::Identifier);
  EXPECT_EQ(tokens[0].raw, text);

  // Cleanup
  delete tokenizer;
}

TEST(Tokenizer, identifiesMixOfTypes)
{
  std::string text("int abc(1, \"test\");true");
  std::vector<TokenType> expected = {
      TokenType::Identifier, TokenType::Identifier,
      TokenType::LeftParen, TokenType::Literal, TokenType::Comma,
      TokenType::Literal, TokenType::RightParen, TokenType::Semicolon, TokenType::Literal};

  std::istringstream stream(text);
  Tokenizer *tokenizer = new Tokenizer(stream);

  tokenizer->parse();
  auto tokens = *(tokenizer->close().get());

  ASSERT_EQ(tokens.size(), expected.size());
  for (int i = 0; i < tokens.size(); i++)
  {
    EXPECT_EQ(tokens[i].type, expected[i]);
  }

  // Cleanup
  delete tokenizer;
}

TEST(Tokenizer, identifiesMixOfTypesWithWhitespace)
{
  std::string text("int abc\r(1\n, \"test\");true");
  std::vector<TokenType> expected = {
      TokenType::Identifier, TokenType::Identifier,
      TokenType::LeftParen, TokenType::Literal, TokenType::Comma,
      TokenType::Literal, TokenType::RightParen, TokenType::Semicolon, TokenType::Literal};

  std::istringstream stream(text);
  Tokenizer *tokenizer = new Tokenizer(stream);

  tokenizer->parse();
  auto tokens = *(tokenizer->close().get());

  ASSERT_EQ(tokens.size(), expected.size());
  for (int i = 0; i < tokens.size(); i++)
  {
    EXPECT_EQ(tokens[i].type, expected[i]);
  }

  // Cleanup
  delete tokenizer;
}