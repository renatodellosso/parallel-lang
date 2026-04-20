#pragma once

#include <string>
#include <vector>

struct E2eTest {
  std::string name;
  std::string code;
  std::vector<std::string> output;
};

extern std::vector<E2eTest> tests;