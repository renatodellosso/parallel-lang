#include "../../src/cli.hpp"
#include "../testUtils.hpp"
#include "tests.hpp"
#include "gtest/gtest.h"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <vector>

// Int is thread count
class E2eFixture : public testing::TestWithParam<std::tuple<E2eTest, int>> {};

constexpr std::string folder = "temp";

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

TEST_P(E2eFixture, E2E) {
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

  EXPECT_EQ(test.output.size(), splitOut.size());
  for (auto line : test.output) {
    EXPECT_NE(std::ranges::find(splitOut, line), splitOut.end());
  }
}

INSTANTIATE_TEST_SUITE_P(
    E2E, E2eFixture,
    testing::Combine(testing::ValuesIn(tests.begin(), tests.end()),
                     testing::Values(1, 2, 4, 8, 16)),
    [](const testing::TestParamInfo<E2eFixture::ParamType> &info) {
      return getTestName(std::get<E2eTest>(info.param),
                         std::get<int>(info.param));
    });