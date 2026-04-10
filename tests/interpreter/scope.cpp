#include "../../src/interpreter/scope.hpp"
#include <gtest/gtest.h>
#include <memory>

TEST(get, returnsNullPtrIfNotFound) {
  std::unordered_map<std::string, std::shared_ptr<Value>> vars;

  Scope scope(vars);

  auto found = scope.get("not_found");
  EXPECT_EQ(found, nullptr);
}

TEST(get, returnsNullPtrIfNotFoundWithEnclosing) {
  std::unordered_map<std::string, std::shared_ptr<Value>> vars;

  auto enclosing = std::make_shared<Scope>();
  Scope scope(enclosing, vars);

  auto found = scope.get("not_found");
  EXPECT_EQ(found, nullptr);
}

TEST(get, returnsValPtrIfFound) {
  std::unordered_map<std::string, std::shared_ptr<Value>> vars;
  vars["found"] = std::make_shared<Value>();

  Scope scope(vars);

  auto found = scope.get("found");
  EXPECT_EQ(found, vars["found"]);
}

TEST(get, returnsValPtrIfFoundInEnclosing) {
  std::unordered_map<std::string, std::shared_ptr<Value>> vars;
  vars["found"] = std::make_shared<Value>();

  auto enclosing = std::make_shared<Scope>(vars);
  Scope scope(enclosing);

  auto found = scope.get("found");
  EXPECT_EQ(found, vars["found"]);
}

TEST(get, returnsValPtrIfFoundInBaseWithEnclosing) {
  std::unordered_map<std::string, std::shared_ptr<Value>> vars;
  vars["found"] = std::make_shared<Value>();

  auto enclosing = std::make_shared<Scope>();
  Scope scope(enclosing, vars);

  auto found = scope.get("found");
  EXPECT_EQ(found, vars["found"]);
}

TEST(get, returnsValPtrIfFoundInBaseAndEnclosing) {
  std::unordered_map<std::string, std::shared_ptr<Value>> vars;
  vars["found"] = std::make_shared<Value>();

  std::unordered_map<std::string, std::shared_ptr<Value>> outerVars;
  outerVars["found"] = std::make_shared<Value>();

  auto enclosing = std::make_shared<Scope>(outerVars);
  Scope scope(enclosing, vars);

  auto found = scope.get("found");
  EXPECT_EQ(found, vars["found"]);
}