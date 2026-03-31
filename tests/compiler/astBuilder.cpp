#include <gtest/gtest.h>
#include "../../src/compiler/astBuilder.hpp"
#include "../testUtils.hpp"

TEST(AstBuilder, buildsLiteralExpressions)
{
  std::vector<Token> tokens = {
      {TokenType::Literal,
       TokenSubtype::Number,
       "1",
       1},
      {TokenType::Semicolon, TokenSubtype::None, ";", 1}};

  AstBuilder builder(std::make_unique<std::vector<Token>>(tokens));

  builder.build();

  EXPECT_EQ(builder.getErrors().get()->size(), 0);

  auto root = builder.getRoot();
  ASSERT_EQ(root.get()->expressions.size(), 1); // Assert to avoid segfault if no expressions

  auto expr = root.get()->expressions[0].get();
  EXPECT_EQ(expr->type, InstructionType::GetLiteral);

  // Can't use dynamic_cast here since Expression has no virtual methods
  RootExpression *rootExpr = static_cast<RootExpression *>(expr);

  EXPECT_TOKEN_EQ(rootExpr->token, tokens[0]);
}

TEST(AstBuilder, buildsBinaryExpressions)
{
  std::vector<Token> tokens = {
      {TokenType::Literal,
       TokenSubtype::Number,
       "1",
       1},
      {TokenType::Plus,
       TokenSubtype::None,
       "+",
       1},
      {TokenType::Literal,
       TokenSubtype::Number,
       "2",
       1},
      {TokenType::Semicolon, TokenSubtype::None, ";", 1}};

  AstBuilder builder(std::make_unique<std::vector<Token>>(tokens));

  builder.build();

  EXPECT_EQ(builder.getErrors().get()->size(), 0);

  auto root = builder.getRoot();
  ASSERT_EQ(root.get()->expressions.size(), 1); // Assert to avoid segfault if no expressions

  auto expr = root.get()->expressions[0].get();
  EXPECT_EQ(expr->type, InstructionType::Add);

  // Can't use dynamic_cast here since Expression has no virtual methods
  BinaryExpression *binary = static_cast<BinaryExpression *>(expr);
  RootExpression *left = static_cast<RootExpression *>(binary->left.get());
  RootExpression *right = static_cast<RootExpression *>(binary->right.get());

  EXPECT_EQ(binary->type, InstructionType::Add);
  EXPECT_TOKEN_EQ(left->token, tokens[0]);
  EXPECT_TOKEN_EQ(right->token, tokens[2]);
}

TEST(AstBuilder, buildsMultipleLines)
{
  std::vector<Token> tokens = {
      {TokenType::Literal,
       TokenSubtype::Number,
       "1",
       1},
      {TokenType::Plus,
       TokenSubtype::None,
       "+",
       1},
      {TokenType::Literal,
       TokenSubtype::Number,
       "2",
       1},
      {TokenType::Semicolon, TokenSubtype::None, ";", 1},
      {TokenType::Literal,
       TokenSubtype::Number,
       "2",
       2},
      {TokenType::Minus,
       TokenSubtype::None,
       "-",
       2},
      {TokenType::Literal,
       TokenSubtype::Number,
       "3",
       2},
      {TokenType::Semicolon, TokenSubtype::None, ";", 2}};

  AstBuilder builder(std::make_unique<std::vector<Token>>(tokens));

  builder.build();

  EXPECT_EQ(builder.getErrors().get()->size(), 0);

  auto root = builder.getRoot();
  ASSERT_EQ(root.get()->expressions.size(), 2); // Assert to avoid segfault if no expressions

  auto expr = root.get()->expressions[0].get();
  EXPECT_EQ(expr->type, InstructionType::Add);

  expr = root.get()->expressions[1].get();
  EXPECT_EQ(expr->type, InstructionType::Subtract);
}

TEST(AstBuilder, errorsOnInvalidToken)
{
  std::vector<Token> tokens = {
      {TokenType::Error,
       TokenSubtype::Number,
       "1",
       1},
  };

  AstBuilder builder(std::make_unique<std::vector<Token>>(tokens));

  builder.build();

  EXPECT_EQ(builder.getErrors().get()->size(), 1);
  EXPECT_EQ(builder.getRoot().get()->expressions.size(), 0);
}

TEST(AstBuilder, errorsOnPartialBinaryExpression)
{
  std::vector<Token> tokens = {
      {TokenType::Literal,
       TokenSubtype::Number,
       "1",
       1},
      {TokenType::Plus,
       TokenSubtype::None,
       "+",
       1},
      {TokenType::Semicolon,
       TokenSubtype::None,
       ";",
       1},
  };

  AstBuilder builder(std::make_unique<std::vector<Token>>(tokens));

  builder.build();

  EXPECT_EQ(builder.getErrors().get()->size(), 1);
  EXPECT_EQ(builder.getRoot().get()->expressions.size(), 0);
}

TEST(AstBuilder, errorsOnLeadingBinaryExpression)
{
  std::vector<Token> tokens = {
      {TokenType::Plus,
       TokenSubtype::None,
       "+",
       1},
      {TokenType::Semicolon,
       TokenSubtype::None,
       ";",
       1},
  };

  AstBuilder builder(std::make_unique<std::vector<Token>>(tokens));

  builder.build();

  EXPECT_EQ(builder.getErrors().get()->size(), 1);
  EXPECT_EQ(builder.getRoot().get()->expressions.size(), 0);
}