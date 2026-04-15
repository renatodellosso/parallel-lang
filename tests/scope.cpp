#include "../src/scope.hpp"
#include "../src/value.hpp"
#include <gtest/gtest.h>
#include <memory>

TEST(get, returnsNullPtrIfNotFound) {
  std::unordered_map<std::string, std::shared_ptr<Value>> vars;

  Scope<Value> scope(vars);

  auto found = scope.get("not_found");
  EXPECT_EQ(found, nullptr);
}

TEST(get, returnsNullPtrIfNotFoundWithEnclosing) {
  std::unordered_map<std::string, std::shared_ptr<Value>> vars;

  auto enclosing = std::make_shared<Scope<Value>>();
  Scope<Value> scope(enclosing, vars);

  auto found = scope.get("not_found");
  EXPECT_EQ(found, nullptr);
}

TEST(get, returnsValPtrIfFound) {
  std::unordered_map<std::string, std::shared_ptr<Value>> vars;
  vars["found"] = std::make_shared<Value>();

  Scope<Value> scope(vars);

  auto found = scope.get("found");
  EXPECT_EQ(found, vars["found"]);
}

TEST(get, returnsValPtrIfFoundInEnclosing) {
  std::unordered_map<std::string, std::shared_ptr<Value>> vars;
  vars["found"] = std::make_shared<Value>();

  auto enclosing = std::make_shared<Scope<Value>>(vars);
  Scope<Value> scope(enclosing);

  auto found = scope.get("found");
  EXPECT_EQ(found, vars["found"]);
}

TEST(get, returnsValPtrIfFoundInBaseWithEnclosing) {
  std::unordered_map<std::string, std::shared_ptr<Value>> vars;
  vars["found"] = std::make_shared<Value>();

  auto enclosing = std::make_shared<Scope<Value>>();
  Scope<Value> scope(enclosing, vars);

  auto found = scope.get("found");
  EXPECT_EQ(found, vars["found"]);
}

TEST(get, returnsValPtrIfFoundInBaseAndEnclosing) {
  std::unordered_map<std::string, std::shared_ptr<Value>> vars;
  vars["found"] = std::make_shared<Value>();

  std::unordered_map<std::string, std::shared_ptr<Value>> outerVars;
  outerVars["found"] = std::make_shared<Value>();

  auto enclosing = std::make_shared<Scope<Value>>(outerVars);
  Scope<Value> scope(enclosing, vars);

  auto found = scope.get("found");
  EXPECT_EQ(found, vars["found"]);
}

TEST(alloc, returnsPtr) {
  Scope<Value> scope;

  auto ptr = scope.alloc("key");

  EXPECT_NE(ptr, nullptr);
}

TEST(alloc, returnsValue) {
  Scope<Value> scope;

  auto ptr = scope.alloc("key", {.type = ValueType::Integer, .val = 2});

  EXPECT_EQ(ptr->type, ValueType::Integer);
  EXPECT_EQ(std::get<int>(ptr->val), 2);
}

TEST(alloc, valueCanBeGotten) {
  Scope<Value> scope;

  auto ptr = scope.alloc("key", {.type = ValueType::Integer, .val = 2});

  EXPECT_EQ(scope.get("key"), ptr);
}

TEST(alloc, canShadowEnclosingVar) {
  std::unordered_map<std::string, std::shared_ptr<Value>> vars;
  vars["key"] = std::make_shared<Value>();

  auto enclosing = std::make_shared<Scope<Value>>(vars);
  Scope<Value> scope(enclosing);

  auto ptr = scope.alloc("key", {.type = ValueType::Integer, .val = 2});

  EXPECT_EQ(scope.get("key"), ptr);
}