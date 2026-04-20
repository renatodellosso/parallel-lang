#include "tests.hpp"

std::vector<E2eTest> tests = {
    // Print statements
    {"PrintWorksWithNumbers", "print 1;", {"1"}},
    {"PrintWorksWithStrings", "print \"abc\";", {"abc"}},
    {"PrintWorksWithBools", "print true;", {"true"}},
    {"PrintWorksWithMultiplePrints", "print 2;\nprint false;", {"false", "2"}},

    // Operators
    {"AdditionWorksWithNumbers", "print 1 + 2;", {"3"}},
    {"AdditionWorksWithThreeNumbers", "print 1 + 1 + 1;", {"3"}},
    {"AdditionWorksWithStrings", "print \"a\" + \"b\";", {"ab"}},
    {"AdditionWorksWithBools", "print false + true;", {"true"}},
    {"AdditionWorksWithMixedTypes", "print 1 + \"b\" + true;", {"1btrue"}}};