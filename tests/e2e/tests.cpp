#include "tests.hpp"

// NOTE: Expecting the same line multiple times will not work!

std::vector<E2eTest> tests = {
    // Print statements
    {"PrintWorksWithNumbers", "print 1;", {"1"}},
    {"PrintWorksWithStrings", "print \"abc\";", {"abc"}},
    {"PrintWorksWithBools", "print true;", {"true"}},
    {"PrintWorksWithMultiplePrints", "print 2;\nprint false;", {"false", "2"}},

    // Addition
    {"AdditionWorksWithNumbers", "print 1 + 2;", {"3"}},
    {"AdditionWorksWithThreeNumbers", "print 1 + 1 + 1;", {"3"}},
    {"AdditionWorksWithStrings", "print \"a\" + \"b\";", {"ab"}},
    {"AdditionWorksWithBools", "print false + true;", {"true"}},
    {"AdditionWorksWithMixedTypes", "print 1 + \"b\" + true;", {"1btrue"}},

    // Subtraction
    {"SubtractionWorks", "print 1 - 1;", {"0"}},
    {"SubtractionWorksWithThreeNumbers", "print 1 - 1 - 2;", {"2"}},
    {"SubtractionWorksWithNegativeNumbers", "print 1 - -1;", {"2"}},

    // Multiplication
    {"MultiplicationWorks", "print 2 * 2;", {"4"}},
    {"MultiplicationWithThreeNumbers", "print 2 * 3 * 2;", {"12"}},
    {"MultiplicationWorksWithNegativeNumbers", "print 1 * -1;", {"-1"}},

    // Division
    {"DivisionWorksWhenCleanlyDivisible", "print 4 / 2;", {"2"}},
    {"DivisionWorksWhenNotCleanlyDivisible", "print 3 / 2;", {"1"}},
    {"DivisionWorksWithNegativeNumbers", "print 1 / -1;", {"-1"}},

    // Variables
    {"VariablesCanBeDeclared", "int a;", {}},
    {"VariablesCanBeDeclaredAndInitialized", "int a = 1;\nprint a;", {"1"}},
    {"VariablesCanBeSet", "int a;\na = 1;\nprint a;", {"1"}},
    {"VariablesCanBeSetMultipleTimes",
     "int a;\na = 1;\na = 2;\nprint a;",
     {"2"}},
    {"VariablesCanBeUsedInOperations", "int a = 1;\nprint a + 1;", {"2"}},
    {"VariablesCanBeDeclaredMultipleTimes",
     "int a = 1;\nprint a;\nint b = 2;\nprint b;",
     {"1", "2"}},
    {"VariablesCanHaveBoolType", "bool a = true;\nprint a;", {"true"}},
    {"VariablesCanHaveStringType", "string a = \"abc\";\nprint a;", {"abc"}},
    {"VariablesCanBeUsedToUpdateThemselves",
     "int a = 1;\na = a + 1;\nprint a;",
     {"2"}},

    // If statements
    {"IfsDontRunIfConditionIsFalse", "if (false) { print \"ran\"; }", {}},
    {"IfsRunIfConditionIsTrue", "if (true) { print \"ran\"; }", {"ran"}},
    {"IfsAllowImplicitBlocks", "if (true) print \"ran\";", {"ran"}},
    {"IfsAllowComplexConditions", "if (1 + 1 - 1) print \"ran\";", {"ran"}},
    {"IfsAllowVariablesInCondition",
     "bool a = true;\nif (a) print \"ran\";",
     {"ran"}},

    // While loops
    {"WhileLoopsRunWhileConditionIsTrue",
     "int count = 10;\n"
     "while (count) {\n"
     "print count;\n"
     "count = count - 1;\n"
     "}",
     {"10", "9", "8", "7", "6", "5", "4", "3", "2", "1"}},
    {"WhileLoopsAllowImplicitBlocks",
     "int count = 10;\n"
     "while (count)\n"
     "count = count - 1;\n"
     "print count;",
     {"0"}}};