#include "../../src/compiler/astBuilder.hpp"
#include "../testUtils.hpp"
#include <gtest/gtest.h>

TEST(AstBuilder, buildsLiteralExpressions) {
  std::vector<Token> tokens = {
      {TokenType::Literal, TokenSubtype::Integer, "1", 1},
      {TokenType::Semicolon, TokenSubtype::None, ";", 1}};

  AstBuilder builder(std::make_unique<std::vector<Token>>(tokens));

  builder.build();

  EXPECT_EQ(builder.getErrors().get()->size(), 0);

  auto expressions = *builder.getExpressions().get();
  ASSERT_EQ(expressions.size(),
            1); // Assert to avoid segfault if no expressions

  auto expr = expressions[0].get();
  EXPECT_EQ(expr->type, InstructionType::GetLiteral);

  // Can't use dynamic_cast here since Expression has no virtual methods
  RootExpression *rootExpr = static_cast<RootExpression *>(expr);

  EXPECT_TOKEN_EQ(rootExpr->token, tokens[0]);
}

TEST(AstBuilder, buildsBinaryExpressions) {
  std::vector<Token> tokens = {
      {TokenType::Literal, TokenSubtype::Integer, "1", 1},
      {TokenType::Plus, TokenSubtype::None, "+", 1},
      {TokenType::Literal, TokenSubtype::Integer, "2", 1},
      {TokenType::Semicolon, TokenSubtype::None, ";", 1}};

  AstBuilder builder(std::make_unique<std::vector<Token>>(tokens));

  builder.build();

  EXPECT_EQ(builder.getErrors().get()->size(), 0);

  auto expressions = *builder.getExpressions().get();
  ASSERT_EQ(expressions.size(),
            1); // Assert to avoid segfault if no expressions

  auto expr = expressions[0].get();
  EXPECT_EQ(expr->type, InstructionType::Add);

  // Can't use dynamic_cast here since Expression has no virtual methods
  BinaryExpression *binary = static_cast<BinaryExpression *>(expr);
  RootExpression *left = static_cast<RootExpression *>(binary->left.get());
  RootExpression *right = static_cast<RootExpression *>(binary->right.get());

  EXPECT_EQ(binary->type, InstructionType::Add);
  EXPECT_TOKEN_EQ(left->token, tokens[0]);
  EXPECT_TOKEN_EQ(right->token, tokens[2]);
}

TEST(AstBuilder, buildsDeclarations) {
  std::vector<Token> tokens = {
      {TokenType::Identifier, TokenSubtype::None, "int", 1},
      {TokenType::Identifier, TokenSubtype::None, "var", 1},
      {TokenType::Semicolon, TokenSubtype::None, ";", 1}};

  AstBuilder builder(std::make_unique<std::vector<Token>>(tokens));

  builder.build();

  EXPECT_EQ(builder.getErrors().get()->size(), 0);

  auto expressions = *builder.getExpressions().get();
  ASSERT_EQ(expressions.size(),
            1); // Assert to avoid segfault if no expressions

  auto expr = expressions[0].get();
  EXPECT_EQ(expr->type, InstructionType::Declare);

  // Can't use dynamic_cast here since Expression has no virtual methods
  BinaryExpression *binary = static_cast<BinaryExpression *>(expr);
  RootExpression *left = static_cast<RootExpression *>(binary->left.get());
  RootExpression *right = static_cast<RootExpression *>(binary->right.get());

  EXPECT_EQ(binary->type, InstructionType::Declare);
  EXPECT_TOKEN_EQ(left->token, tokens[0]);
  EXPECT_TOKEN_EQ(right->token, tokens[1]);
}

TEST(AstBuilder, buildsSets) {
  std::vector<Token> tokens = {
      {TokenType::Identifier, TokenSubtype::None, "int", 1},
      {TokenType::Equals, TokenSubtype::None, "=", 1},
      {TokenType::Identifier, TokenSubtype::None, "int", 1},
      {TokenType::Semicolon, TokenSubtype::None, ";", 1}};

  AstBuilder builder(std::make_unique<std::vector<Token>>(tokens));

  builder.build();

  EXPECT_EQ(builder.getErrors().get()->size(), 0);

  auto expressions = *builder.getExpressions().get();
  ASSERT_EQ(expressions.size(),
            1); // Assert to avoid segfault if no expressions

  auto expr = expressions[0].get();
  EXPECT_EQ(expr->type, InstructionType::Set);

  // Can't use dynamic_cast here since Expression has no virtual methods
  BinaryExpression *binary = static_cast<BinaryExpression *>(expr);
  RootExpression *left = static_cast<RootExpression *>(binary->left.get());
  RootExpression *right = static_cast<RootExpression *>(binary->right.get());

  EXPECT_EQ(binary->type, InstructionType::Set);
  EXPECT_TOKEN_EQ(left->token, tokens[0]);
  EXPECT_TOKEN_EQ(right->token, tokens[2]);
}

TEST(AstBuilder, buildsMultipleLines) {
  std::vector<Token> tokens = {
      {TokenType::Literal, TokenSubtype::Integer, "1", 1},
      {TokenType::Plus, TokenSubtype::None, "+", 1},
      {TokenType::Literal, TokenSubtype::Integer, "2", 1},
      {TokenType::Semicolon, TokenSubtype::None, ";", 1},
      {TokenType::Literal, TokenSubtype::Integer, "2", 2},
      {TokenType::Minus, TokenSubtype::None, "-", 2},
      {TokenType::Literal, TokenSubtype::Integer, "3", 2},
      {TokenType::Semicolon, TokenSubtype::None, ";", 2}};

  AstBuilder builder(std::make_unique<std::vector<Token>>(tokens));

  builder.build();

  EXPECT_EQ(builder.getErrors().get()->size(), 0);

  auto expressions = *builder.getExpressions().get();
  ASSERT_EQ(expressions.size(),
            2); // Assert to avoid segfault if no expressions

  auto expr = expressions[0].get();
  EXPECT_EQ(expr->type, InstructionType::Add);

  expr = expressions[1].get();
  EXPECT_EQ(expr->type, InstructionType::Subtract);
}

TEST(AstBuilder, errorsOnInvalidToken) {
  std::vector<Token> tokens = {
      {TokenType::Error, TokenSubtype::Integer, "1", 1},
  };

  AstBuilder builder(std::make_unique<std::vector<Token>>(tokens));

  builder.build();

  EXPECT_EQ(builder.getErrors().get()->size(), 1);
  EXPECT_EQ(builder.getExpressions().get()->size(), 0);
}

TEST(AstBuilder, errorsOnPartialBinaryExpression) {
  std::vector<Token> tokens = {
      {TokenType::Literal, TokenSubtype::Integer, "1", 1},
      {TokenType::Plus, TokenSubtype::None, "+", 1},
      {TokenType::Semicolon, TokenSubtype::None, ";", 1},
  };

  AstBuilder builder(std::make_unique<std::vector<Token>>(tokens));

  builder.build();

  EXPECT_EQ(builder.getErrors().get()->size(), 1);
  EXPECT_EQ(builder.getExpressions().get()->size(), 0);
}

TEST(AstBuilder, errorsOnLeadingBinaryExpression) {
  std::vector<Token> tokens = {
      {TokenType::Plus, TokenSubtype::None, "+", 1},
      {TokenType::Semicolon, TokenSubtype::None, ";", 1},
  };

  AstBuilder builder(std::make_unique<std::vector<Token>>(tokens));

  builder.build();

  EXPECT_EQ(builder.getErrors().get()->size(), 1);
  EXPECT_EQ(builder.getExpressions().get()->size(), 0);
}

TEST(AstBuilder, buildsIfStatements) {
  std::vector<Token> tokens = {
      {TokenType::If, TokenSubtype::None, "+", 1},
      {TokenType::LeftParen, TokenSubtype::None, "+", 1},
      {TokenType::Literal, TokenSubtype::Bool, "true", 1},
      {TokenType::RightParen, TokenSubtype::None, ";", 1},
      {TokenType::Literal, TokenSubtype::Integer, "1", 2},
      {TokenType::Plus, TokenSubtype::None, "+", 2},
      {TokenType::Literal, TokenSubtype::Integer, "1", 2},
  };

  AstBuilder builder(std::make_unique<std::vector<Token>>(tokens));

  builder.build();

  EXPECT_EQ(builder.getErrors().get()->size(), 0);
  ASSERT_EQ(builder.getExpressions().get()->size(), 2);
  EXPECT_EQ(builder.getExpressions().get()->at(0)->type, InstructionType::If);
}