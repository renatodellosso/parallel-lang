#include "../../src/cli.hpp"
#include "../testUtils.hpp"
#include "tests.hpp"
#include "gtest/gtest.h"
#include <filesystem>
#include <format>
#include <fstream>
#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <vector>

// Int is thread count
class E2EFixture : public testing::TestWithParam<std::tuple<E2eTest, int>> {};

const std::string folder = "temp";

std::string getTestName(E2eTest test, int threads) {
  return test.name + std::to_string(threads);
}

std::vector<std::string> split(std::string str, char delimiter) {
  std::vector<std::string> vec;

  std::string temp;
  std::stringstream stream(str);

  while (std::getline(stream, temp, delimiter))
    vec.push_back(temp);

  return vec;
}

std::string vectorToString(std::vector<std::string> vector) {
  std::string str = "";

  for (int i = 0; i < vector.size(); i++) {
    str += "'" + vector[i] + "'";
    if (i < vector.size() - 1)
      str += ", ";
  }

  return str;
}

testing::AssertionResult Contains(std::vector<std::string> vector,
                                  std::string line) {
  if (std::ranges::find(vector, line) == vector.end())
    return testing::AssertionFailure()
           << "Vector did not contain '" << line
           << "'. Vector: " << vectorToString(vector);
  return testing::AssertionSuccess() << "Vector contained '" << line
                                     << "'. Vector: " << vectorToString(vector);
}

testing::AssertionResult EqualSize(std::vector<std::string> actual,
                                   std::vector<std::string> expected) {
  std::string expectedStr = "";
  for (auto str : expected)
    expectedStr += "\"" + str + "\", ";
  if (!expected.empty())
    expectedStr.erase(expectedStr.end() - 2); // Remove trailing ', '

  std::string actualStr = "";
  for (auto str : actual)
    actualStr += "\"" + str + "\", ";
  if (!actual.empty())
    actualStr.erase(actualStr.end() - 2); // Remove trailing ', '

  std::string message = std::format(
      "Expected size: {}, Actual size: {}, Expected: {}, Actual: {}",
      expected.size(), actual.size(), expectedStr, actualStr);

  if (actual.size() == expected.size())
    return testing::AssertionSuccess() << message;
  return testing::AssertionFailure() << message;
}

TEST_P(E2EFixture, E2E) {
  auto test = std::get<E2eTest>(GetParam());
  auto threads = std::get<int>(GetParam());

  // Ensure folder exists
  std::filesystem::create_directory(folder);
  auto fileName = folder + "/" + getTestName(test, threads) + ".p";

  CliArgs args = {.target = fileName,
                  .mode = CliMode::CompileAndInterpret,
                  .threads = threads};

  // Create temp file with code
  std::ofstream fileStream(fileName);
  fileStream << test.code;
  fileStream.close();

  DISABLE_COUT
  EXPECT_EQ(executeCommand(args), ExitCode::Ok);
  auto output = REENABLE_COUT;

  std::filesystem::remove(fileName);

  auto splitOut = split(output, '\n');

  EXPECT_TRUE(EqualSize(splitOut, test.output));
  // EXPECT_EQ(splitOut.size(), test.output.size());
  for (auto line : test.output) {
    EXPECT_TRUE(Contains(splitOut, line));
  }
}

INSTANTIATE_TEST_SUITE_P(
    E2E, E2EFixture,
    testing::Combine(testing::ValuesIn(tests.begin(), tests.end()),
                     testing::Values(1, 2, 4, 8, 16)),
    [](const testing::TestParamInfo<E2EFixture::ParamType> &info) {
      return getTestName(std::get<E2eTest>(info.param),
                         std::get<int>(info.param));
    });