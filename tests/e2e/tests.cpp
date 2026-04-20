#include "tests.hpp"

std::vector<E2eTest> tests = {
    {"PrintWorksWithNumbers", "print 1;", {"1"}},
    {"PrintWorksWithStrings", "print \"abc\";", {"abc"}},
    {"PrintWorksWithBools", "print true;", {"true"}},
    {"PrintWorksWithMultiplePrints", "print 2;\nprint false;", {"false", "2"}}};