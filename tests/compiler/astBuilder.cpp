#include "../../src/compiler/astBuilder.hpp"
#include "../testUtils.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <vector>

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

TEST(AstBuilder, buildsEqualsExpressions) {
  std::vector<Token> tokens = {
      {TokenType::Literal, TokenSubtype::Integer, "1", 1},
      {TokenType::EqualsEquals, TokenSubtype::None, "==", 1},
      {TokenType::Literal, TokenSubtype::Integer, "1", 1},
      {TokenType::Semicolon, TokenSubtype::None, ";", 1}};

  AstBuilder builder(std::make_unique<std::vector<Token>>(tokens));

  builder.build();

  EXPECT_EQ(builder.getErrors().get()->size(), 0);

  auto expressions = *builder.getExpressions().get();
  ASSERT_EQ(expressions.size(), 1);

  auto expr = expressions[0].get();
  EXPECT_EQ(expr->type, InstructionType::CompareEquals);

  BinaryExpression *binary = static_cast<BinaryExpression *>(expr);
  RootExpression *left = static_cast<RootExpression *>(binary->left.get());
  RootExpression *right = static_cast<RootExpression *>(binary->right.get());

  EXPECT_EQ(binary->type, InstructionType::CompareEquals);
  EXPECT_TOKEN_EQ(left->token, tokens[0]);
  EXPECT_TOKEN_EQ(right->token, tokens[2]);
}

TEST(AstBuilder, buildsComparisonExpressions) {
  std::vector<std::pair<Token, InstructionType>> cases = {
      {{TokenType::NotEquals, TokenSubtype::None, "!=", 1},
       InstructionType::CompareNotEquals},
      {{TokenType::LessThan, TokenSubtype::None, "<", 1},
       InstructionType::CompareLessThan},
      {{TokenType::LessThanEquals, TokenSubtype::None, "<=", 1},
       InstructionType::CompareLessThanEquals},
      {{TokenType::GreaterThan, TokenSubtype::None, ">", 1},
       InstructionType::CompareGreaterThan},
      {{TokenType::GreaterThanEquals, TokenSubtype::None, ">=", 1},
       InstructionType::CompareGreaterThanEquals},
  };

  for (auto testCase : cases) {
    std::vector<Token> tokens = {
        {TokenType::Literal, TokenSubtype::Integer, "1", 1}, testCase.first,
        {TokenType::Literal, TokenSubtype::Integer, "2", 1},
        {TokenType::Semicolon, TokenSubtype::None, ";", 1}};

    AstBuilder builder(std::make_unique<std::vector<Token>>(tokens));

    builder.build();

    EXPECT_EQ(builder.getErrors().get()->size(), 0);

    auto expressions = *builder.getExpressions().get();
    ASSERT_EQ(expressions.size(), 1);

    auto expr = expressions[0].get();
    EXPECT_EQ(expr->type, testCase.second);

    BinaryExpression *binary = static_cast<BinaryExpression *>(expr);
    RootExpression *left = static_cast<RootExpression *>(binary->left.get());
    RootExpression *right = static_cast<RootExpression *>(binary->right.get());

    EXPECT_EQ(binary->type, testCase.second);
    EXPECT_TOKEN_EQ(left->token, tokens[0]);
    EXPECT_TOKEN_EQ(right->token, tokens[2]);
  }
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
  ASSERT_EQ(builder.getExpressions().get()->size(), 1);

  auto ifExpr =
      std::dynamic_pointer_cast<IfExpression>(builder.getExpressions()->at(0));
  ASSERT_NE(ifExpr, nullptr);
  EXPECT_EQ(ifExpr->type, InstructionType::If);
  ASSERT_EQ(ifExpr->thenBlock->expressions.size(), 1);
  EXPECT_EQ(ifExpr->thenBlock->expressions.at(0)->type, InstructionType::Add);
  EXPECT_EQ(ifExpr->elseBlock, nullptr);
}

TEST(AstBuilder, buildsElseStatements) {
  std::vector<Token> tokens = {
      {TokenType::If, TokenSubtype::None, "if", 1},
      {TokenType::LeftParen, TokenSubtype::None, "(", 1},
      {TokenType::Literal, TokenSubtype::Bool, "true", 1},
      {TokenType::RightParen, TokenSubtype::None, ")", 1},
      {TokenType::Literal, TokenSubtype::Integer, "1", 1},
      {TokenType::Semicolon, TokenSubtype::None, ";", 1},
      {TokenType::Else, TokenSubtype::None, "else", 1},
      {TokenType::Literal, TokenSubtype::Integer, "2", 1},
      {TokenType::Semicolon, TokenSubtype::None, ";", 1},
  };

  AstBuilder builder(std::make_unique<std::vector<Token>>(tokens));

  builder.build();

  EXPECT_EQ(builder.getErrors().get()->size(), 0);
  ASSERT_EQ(builder.getExpressions().get()->size(), 1);

  auto ifExpr =
      std::dynamic_pointer_cast<IfExpression>(builder.getExpressions()->at(0));
  ASSERT_NE(ifExpr, nullptr);
  ASSERT_NE(ifExpr->elseBlock, nullptr);
  ASSERT_NE(ifExpr->elseInstruction, nullptr);
  EXPECT_EQ(ifExpr->elseInstruction->type, InstructionType::Else);
  ASSERT_EQ(ifExpr->thenBlock->expressions.size(), 1);
  ASSERT_EQ(ifExpr->elseBlock->expressions.size(), 1);
}

TEST(AstBuilder, buildsElseIfStatements) {
  std::vector<Token> tokens = {
      {TokenType::If, TokenSubtype::None, "if", 1},
      {TokenType::LeftParen, TokenSubtype::None, "(", 1},
      {TokenType::Literal, TokenSubtype::Bool, "false", 1},
      {TokenType::RightParen, TokenSubtype::None, ")", 1},
      {TokenType::Literal, TokenSubtype::Integer, "1", 1},
      {TokenType::Semicolon, TokenSubtype::None, ";", 1},
      {TokenType::Else, TokenSubtype::None, "else", 1},
      {TokenType::If, TokenSubtype::None, "if", 1},
      {TokenType::LeftParen, TokenSubtype::None, "(", 1},
      {TokenType::Literal, TokenSubtype::Bool, "true", 1},
      {TokenType::RightParen, TokenSubtype::None, ")", 1},
      {TokenType::Literal, TokenSubtype::Integer, "2", 1},
      {TokenType::Semicolon, TokenSubtype::None, ";", 1},
  };

  AstBuilder builder(std::make_unique<std::vector<Token>>(tokens));

  builder.build();

  EXPECT_EQ(builder.getErrors().get()->size(), 0);
  ASSERT_EQ(builder.getExpressions().get()->size(), 1);

  auto ifExpr =
      std::dynamic_pointer_cast<IfExpression>(builder.getExpressions()->at(0));
  ASSERT_NE(ifExpr, nullptr);
  ASSERT_NE(ifExpr->elseBlock, nullptr);
  ASSERT_EQ(ifExpr->elseBlock->expressions.size(), 1);

  auto nestedIf =
      std::dynamic_pointer_cast<IfExpression>(ifExpr->elseBlock->expressions[0]);
  ASSERT_NE(nestedIf, nullptr);
  EXPECT_EQ(nestedIf->type, InstructionType::If);
}

TEST(AstBuilder, errorsOnElseWithoutIf) {
  std::vector<Token> tokens = {
      {TokenType::Else, TokenSubtype::None, "else", 1},
      {TokenType::Literal, TokenSubtype::Integer, "1", 1},
      {TokenType::Semicolon, TokenSubtype::None, ";", 1},
  };

  AstBuilder builder(std::make_unique<std::vector<Token>>(tokens));

  builder.build();

  EXPECT_EQ(builder.getErrors().get()->size(), 1);
  EXPECT_EQ(builder.getExpressions().get()->size(), 0);
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
}

TEST(AstBuilder, buildsFunctionsWithBlockBodies) {
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

TEST(AstBuilder, buildsFunctionsWithImplicitBlockBodies) {
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

  auto body = func->body;
  ASSERT_NE(body, nullptr);
  EXPECT_EQ(body->type, InstructionType::Block);

  auto block = std::dynamic_pointer_cast<BlockExpression>(body);
  ASSERT_NE(func, nullptr);
  ASSERT_EQ(block->expressions.size(), 1);
  EXPECT_EQ(block->expressions[0]->type, InstructionType::Add);
}

TEST(AstBuilder, buildsCallsWithoutArguments) {
  std::vector<Token> tokens = {
      {TokenType::Identifier, TokenSubtype::None, "func", 1},
      {TokenType::LeftParen, TokenSubtype::None, "(", 1},
      {TokenType::RightParen, TokenSubtype::None, ")", 1},
      {TokenType::Semicolon, TokenSubtype::None, ";", 1},
  };

  AstBuilder builder(std::make_unique<std::vector<Token>>(tokens));
  builder.build();

  EXPECT_EQ(builder.getErrors().get()->size(), 0);

  auto expressions = builder.getExpressions();

  ASSERT_EQ(expressions->size(), 1);

  auto expr = expressions->at(0);
  ASSERT_EQ(expr->type, InstructionType::Block);

  auto func = std::static_pointer_cast<CallExpression>(expr);

  ASSERT_EQ(func->expressions.size(), 1);
  ASSERT_EQ(func->expressions[0]->type, InstructionType::Call);

  auto call =
      std::static_pointer_cast<UnaryCallExpression>(func->expressions[0]);

  auto root = std::static_pointer_cast<RootExpression>(call->root);
  EXPECT_EQ(root->token.raw, tokens[0].raw);
}

TEST(AstBuilder, buildsCallsWithArguments) {
  std::vector<Token> tokens = {
      {TokenType::Identifier, TokenSubtype::None, "func", 1},
      {TokenType::LeftParen, TokenSubtype::None, "(", 1},
      {TokenType::Identifier, TokenSubtype::None, "1", 1},
      {TokenType::Comma, TokenSubtype::None, ",", 1},
      {TokenType::Identifier, TokenSubtype::None, "true", 1},
      {TokenType::Comma, TokenSubtype::None, ",", 1},
      {TokenType::Identifier, TokenSubtype::None, "\"a\"", 1},
      {TokenType::RightParen, TokenSubtype::None, ")", 1},
      {TokenType::Semicolon, TokenSubtype::None, ";", 1},
  };

  AstBuilder builder(std::make_unique<std::vector<Token>>(tokens));
  builder.build();

  EXPECT_EQ(builder.getErrors().get()->size(), 0);

  auto expressions = builder.getExpressions();

  ASSERT_EQ(expressions->size(), 1);

  auto expr = expressions->at(0);
  ASSERT_EQ(expr->type, InstructionType::Block);

  auto func = std::static_pointer_cast<CallExpression>(expr);

  ASSERT_EQ(func->expressions.size(), 4);

  for (int i = 1; i < func->expressions.size(); i++) {
    auto expr = func->expressions[i];
    ASSERT_EQ(expr->type, InstructionType::Set);

    auto set = std::static_pointer_cast<BinaryExpression>(expr);

    ASSERT_EQ(set->left->type, InstructionType::Declare);
    auto declaration = std::static_pointer_cast<BinaryExpression>(set->left);

    auto arg = std::static_pointer_cast<RootExpression>(set->right);
    EXPECT_EQ(arg->token.raw, tokens[i * 2].raw);

    auto type = std::static_pointer_cast<RootExpression>(declaration->left);
    auto name = std::static_pointer_cast<RootExpression>(declaration->right);

    EXPECT_EQ(type->type, InstructionType::GetIdentifier);
    EXPECT_EQ(name->type, InstructionType::GetLiteral);
  }
}

TEST(AstBuilder, buildsCallsWithComplexArguments) {
  std::vector<Token> tokens = {
      {TokenType::Identifier, TokenSubtype::None, "func", 1},
      {TokenType::LeftParen, TokenSubtype::None, "(", 1},
      {TokenType::Literal, TokenSubtype::Integer, "1", 1},
      {TokenType::Plus, TokenSubtype::None, "+", 1},
      {TokenType::Literal, TokenSubtype::Integer, "2", 1},
      {TokenType::Comma, TokenSubtype::None, ",", 1},
      {TokenType::Literal, TokenSubtype::Integer, "true", 1},
      {TokenType::Minus, TokenSubtype::None, "-", 1},
      {TokenType::Literal, TokenSubtype::String, "\"abc\"", 1},
      {TokenType::RightParen, TokenSubtype::None, ")", 1},
      {TokenType::Semicolon, TokenSubtype::None, ";", 1},
  };

  AstBuilder builder(std::make_unique<std::vector<Token>>(tokens));
  builder.build();

  EXPECT_EQ(builder.getErrors().get()->size(), 0);

  auto expressions = builder.getExpressions();

  ASSERT_EQ(expressions->size(), 1);

  auto expr = expressions->at(0);
  ASSERT_EQ(expr->type, InstructionType::Block);

  auto func = std::static_pointer_cast<CallExpression>(expr);

  ASSERT_EQ(func->expressions.size(), 3);

  ASSERT_EQ(func->expressions[0]->type, InstructionType::Call);

  auto binary =
      std::static_pointer_cast<BinaryExpression>(func->expressions[1]);

  binary = std::static_pointer_cast<BinaryExpression>(binary->right);
  ASSERT_EQ(binary->type, InstructionType::Add);

  auto root = std::static_pointer_cast<RootExpression>(binary->left);
  EXPECT_EQ(root->type, InstructionType::GetLiteral);
  EXPECT_EQ(root->token.raw, "1");

  root = std::static_pointer_cast<RootExpression>(binary->right);
  EXPECT_EQ(root->type, InstructionType::GetLiteral);
  EXPECT_EQ(root->token.raw, "2");

  binary = std::static_pointer_cast<BinaryExpression>(func->expressions[2]);
  binary = std::static_pointer_cast<BinaryExpression>(binary->right);
  ASSERT_EQ(binary->type, InstructionType::Subtract);

  root = std::static_pointer_cast<RootExpression>(binary->left);
  EXPECT_EQ(root->type, InstructionType::GetLiteral);
  EXPECT_EQ(root->token.raw, "true");

  root = std::static_pointer_cast<RootExpression>(binary->right);
  EXPECT_EQ(root->type, InstructionType::GetLiteral);
  EXPECT_EQ(root->token.raw, "\"abc\"");
}

TEST(AstBuilder, buildsCallsInLargerExpressions) {
  std::vector<Token> tokens = {
      {TokenType::Literal, TokenSubtype::Integer, "1", 1},
      {TokenType::Plus, TokenSubtype::None, "+", 1},
      {TokenType::Identifier, TokenSubtype::None, "func", 2},
      {TokenType::LeftParen, TokenSubtype::None, "(", 2},
      {TokenType::RightParen, TokenSubtype::None, ")", 2},
      {TokenType::Plus, TokenSubtype::None, "+", 2},
      {TokenType::Literal, TokenSubtype::Integer, "2", 3},
      {TokenType::Semicolon, TokenSubtype::None, ";", 3},
  };

  AstBuilder builder(std::make_unique<std::vector<Token>>(tokens));
  builder.build();

  EXPECT_EQ(builder.getErrors().get()->size(), 0);
  for (auto err : *builder.getErrors().get())
    std::cout << err.toString() << "\n";

  auto expressions = builder.getExpressions();

  ASSERT_EQ(expressions->size(), 1);

  auto expr = expressions->at(0);
  ASSERT_EQ(expr->type, InstructionType::Add);

  auto binary = std::static_pointer_cast<BinaryExpression>(expr);
  binary = std::static_pointer_cast<BinaryExpression>(binary->right);

  ASSERT_EQ(binary->left->type, InstructionType::Block);
  ASSERT_NE(std::dynamic_pointer_cast<CallExpression>(binary->left), nullptr);
  auto func = std::static_pointer_cast<CallExpression>(binary->left);

  ASSERT_EQ(func->expressions.size(), 1);
}
