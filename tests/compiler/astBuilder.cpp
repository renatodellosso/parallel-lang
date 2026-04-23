#include "../../src/compiler/astBuilder.hpp"
#include "../testUtils.hpp"
#include <gtest/gtest.h>
#include <memory>

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
      {TokenType::If, TokenSubtype::None, "if", 1},
      {TokenType::LeftParen, TokenSubtype::None, "(", 1},
      {TokenType::Literal, TokenSubtype::Bool, "true", 1},
      {TokenType::RightParen, TokenSubtype::None, ")", 1},
      {TokenType::Literal, TokenSubtype::Integer, "1", 2},
      {TokenType::Plus, TokenSubtype::None, "+", 2},
      {TokenType::Literal, TokenSubtype::Integer, "1", 2},
      {TokenType::Semicolon, TokenSubtype::None, ";", 2},
  };

  AstBuilder builder(std::make_unique<std::vector<Token>>(tokens));

  builder.build();

  EXPECT_EQ(builder.getErrors().get()->size(), 0);
  ASSERT_EQ(builder.getExpressions().get()->size(), 2);
  EXPECT_EQ(builder.getExpressions().get()->at(0)->type, InstructionType::If);
}

TEST(AstBuilder, buildsWhileStatements) {
  std::vector<Token> tokens = {
      {TokenType::While, TokenSubtype::None, "while", 1},
      {TokenType::LeftParen, TokenSubtype::None, "(", 1},
      {TokenType::Literal, TokenSubtype::Bool, "true", 1},
      {TokenType::RightParen, TokenSubtype::None, ")", 1},
      {TokenType::Literal, TokenSubtype::Integer, "1", 2},
      {TokenType::Plus, TokenSubtype::None, "+", 2},
      {TokenType::Literal, TokenSubtype::Integer, "1", 2},
      {TokenType::Semicolon, TokenSubtype::None, ";", 2},
  };

  AstBuilder builder(std::make_unique<std::vector<Token>>(tokens));

  builder.build();

  EXPECT_EQ(builder.getErrors().get()->size(), 0);
  ASSERT_EQ(builder.getExpressions().get()->size(), 2);
  EXPECT_EQ(builder.getExpressions().get()->at(0)->type,
            InstructionType::While);

  auto expressions = builder.getExpressions();
  int startWith = 0;
  for (auto expr : *expressions) {
    startWith = expr->numberExpressions(startWith);
  }

  auto block = std::dynamic_pointer_cast<BlockExpression>(expressions->at(1));
  ASSERT_NE(block, nullptr);
  ASSERT_EQ(block->expressions.size(), 2);

  auto goTo =
      std::dynamic_pointer_cast<RootExpression>(block->expressions.at(1));
  ASSERT_NE(goTo, nullptr);

  auto whileStatement = expressions->at(0);
  EXPECT_EQ(whileStatement->type, InstructionType::While);

  EXPECT_EQ(goTo->id + std::atoi(goTo->token.raw.c_str()), 0);
}

TEST(AstBuilder, buildsFunctionsWithoutParameters) {
  std::vector<Token> tokens = {
      {TokenType::Identifier, TokenSubtype::None, "void", 1},
      {TokenType::Identifier, TokenSubtype::None, "func", 1},
      {TokenType::LeftParen, TokenSubtype::None, "(", 1},
      {TokenType::RightParen, TokenSubtype::None, ")", 1},
      {TokenType::Literal, TokenSubtype::Integer, "1", 2},
      {TokenType::Plus, TokenSubtype::None, "+", 2},
      {TokenType::Literal, TokenSubtype::Integer, "1", 2},
      {TokenType::Semicolon, TokenSubtype::None, ";", 2},
  };

  AstBuilder builder(std::make_unique<std::vector<Token>>(tokens));

  builder.build();

  EXPECT_EQ(builder.getErrors().get()->size(), 0);
  ASSERT_EQ(builder.getExpressions().get()->size(), 1);
  EXPECT_EQ(builder.getExpressions().get()->at(0)->type,
            InstructionType::Function);

  auto expressions = builder.getExpressions();
  int startWith = 0;
  for (auto expr : *expressions) {
    startWith = expr->numberExpressions(startWith);
  }

  auto func = std::dynamic_pointer_cast<FunctionExpression>(expressions->at(0));
  ASSERT_NE(func, nullptr);

  EXPECT_EQ(func->type, InstructionType::Function);
  EXPECT_EQ(func->name, tokens[1].raw);
  EXPECT_EQ(func->returnType, tokens[0].raw);

  auto body = func->body;
  ASSERT_NE(body, nullptr);
  EXPECT_EQ(body->type, InstructionType::Add);
}

TEST(AstBuilder, buildsFunctionsWithSingleParameter) {
  std::vector<Token> tokens = {
      {TokenType::Identifier, TokenSubtype::None, "void", 1},
      {TokenType::Identifier, TokenSubtype::None, "func", 1},
      {TokenType::LeftParen, TokenSubtype::None, "(", 1},
      {TokenType::Identifier, TokenSubtype::None, "int", 1},
      {TokenType::Identifier, TokenSubtype::None, "param", 1},
      {TokenType::RightParen, TokenSubtype::None, ")", 1},
      {TokenType::Literal, TokenSubtype::Integer, "1", 2},
      {TokenType::Plus, TokenSubtype::None, "+", 2},
      {TokenType::Literal, TokenSubtype::Integer, "1", 2},
      {TokenType::Semicolon, TokenSubtype::None, ";", 2},
  };

  AstBuilder builder(std::make_unique<std::vector<Token>>(tokens));

  builder.build();

  EXPECT_EQ(builder.getErrors().get()->size(), 0);
  ASSERT_EQ(builder.getExpressions().get()->size(), 1);
  EXPECT_EQ(builder.getExpressions().get()->at(0)->type,
            InstructionType::Function);

  auto expressions = builder.getExpressions();
  int startWith = 0;
  for (auto expr : *expressions) {
    startWith = expr->numberExpressions(startWith);
  }

  auto func = std::dynamic_pointer_cast<FunctionExpression>(expressions->at(0));
  ASSERT_NE(func, nullptr);

  EXPECT_EQ(func->type, InstructionType::Function);
  EXPECT_EQ(func->name, tokens[1].raw);
  EXPECT_EQ(func->returnType, tokens[0].raw);

  ASSERT_EQ(func->params.size(), 1);
  auto param = func->params[0];
  EXPECT_EQ(param.type, tokens[3].raw);
  EXPECT_EQ(param.name, tokens[4].raw);

  auto body = func->body;
  ASSERT_NE(body, nullptr);
  EXPECT_EQ(body->type, InstructionType::Add);
}

TEST(AstBuilder, buildsFunctionsWithMultipleParameters) {
  std::vector<Token> tokens = {
      {TokenType::Identifier, TokenSubtype::None, "void", 1},
      {TokenType::Identifier, TokenSubtype::None, "func", 1},
      {TokenType::LeftParen, TokenSubtype::None, "(", 1},
      {TokenType::Identifier, TokenSubtype::None, "int", 1},
      {TokenType::Identifier, TokenSubtype::None, "a", 1},
      {TokenType::Comma, TokenSubtype::None, ",", 1},
      {TokenType::Identifier, TokenSubtype::None, "int", 1},
      {TokenType::Identifier, TokenSubtype::None, "b", 1},
      {TokenType::RightParen, TokenSubtype::None, ")", 1},
      {TokenType::Literal, TokenSubtype::Integer, "1", 2},
      {TokenType::Plus, TokenSubtype::None, "+", 2},
      {TokenType::Literal, TokenSubtype::Integer, "1", 2},
      {TokenType::Semicolon, TokenSubtype::None, ";", 2},
  };

  AstBuilder builder(std::make_unique<std::vector<Token>>(tokens));

  builder.build();

  EXPECT_EQ(builder.getErrors().get()->size(), 0);
  ASSERT_EQ(builder.getExpressions().get()->size(), 1);
  EXPECT_EQ(builder.getExpressions().get()->at(0)->type,
            InstructionType::Function);

  auto expressions = builder.getExpressions();
  int startWith = 0;
  for (auto expr : *expressions) {
    startWith = expr->numberExpressions(startWith);
  }

  auto func = std::dynamic_pointer_cast<FunctionExpression>(expressions->at(0));
  ASSERT_NE(func, nullptr);

  EXPECT_EQ(func->type, InstructionType::Function);
  EXPECT_EQ(func->name, tokens[1].raw);
  EXPECT_EQ(func->returnType, tokens[0].raw);

  ASSERT_EQ(func->params.size(), 2);

  auto param = func->params[0];
  EXPECT_EQ(param.type, tokens[3].raw);
  EXPECT_EQ(param.name, tokens[4].raw);

  param = func->params[1];
  EXPECT_EQ(param.type, tokens[6].raw);
  EXPECT_EQ(param.name, tokens[7].raw);

  auto body = func->body;
  ASSERT_NE(body, nullptr);
  EXPECT_EQ(body->type, InstructionType::Add);
}

TEST(AstBuilder, buildsFunctionsWitBlockBodies) {
  std::vector<Token> tokens = {
      {TokenType::Identifier, TokenSubtype::None, "void", 1},
      {TokenType::Identifier, TokenSubtype::None, "func", 1},
      {TokenType::LeftParen, TokenSubtype::None, "(", 1},
      {TokenType::RightParen, TokenSubtype::None, ")", 1},
      {TokenType::LeftBrace, TokenSubtype::None, "{", 1},
      {TokenType::Literal, TokenSubtype::Integer, "1", 2},
      {TokenType::Plus, TokenSubtype::None, "+", 2},
      {TokenType::Literal, TokenSubtype::Integer, "1", 2},
      {TokenType::Semicolon, TokenSubtype::None, ";", 2},
      {TokenType::RightBrace, TokenSubtype::None, "}", 1},
  };

  AstBuilder builder(std::make_unique<std::vector<Token>>(tokens));

  builder.build();

  EXPECT_EQ(builder.getErrors().get()->size(), 0);
  ASSERT_EQ(builder.getExpressions().get()->size(), 1);
  EXPECT_EQ(builder.getExpressions().get()->at(0)->type,
            InstructionType::Function);

  auto expressions = builder.getExpressions();
  int startWith = 0;
  for (auto expr : *expressions) {
    startWith = expr->numberExpressions(startWith);
  }

  auto func = std::dynamic_pointer_cast<FunctionExpression>(expressions->at(0));
  ASSERT_NE(func, nullptr);

  EXPECT_EQ(func->type, InstructionType::Function);
  EXPECT_EQ(func->name, tokens[1].raw);
  EXPECT_EQ(func->returnType, tokens[0].raw);

  EXPECT_EQ(func->params.size(), 0);

  auto body = std::dynamic_pointer_cast<BlockExpression>(func->body);
  ASSERT_NE(body, nullptr);
  EXPECT_EQ(body->type, InstructionType::Block);
  EXPECT_EQ(body->expressions.size(), 1);
}