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

    // Equality
    {"EqualityWorksWithEqualNumbers", "print 1 == 1;", {"true"}},
    {"EqualityWorksWithUnequalNumbers", "print 1 == 2;", {"false"}},
    {"EqualityWorksWithEqualStrings", "print \"a\" == \"a\";", {"true"}},
    {"EqualityWorksWithUnequalStrings", "print \"a\" == \"b\";", {"false"}},
    {"EqualityWorksWithEqualBools", "print true == true;", {"true"}},
    {"EqualityWorksWithUnequalBools", "print true == false;", {"false"}},
    {"EqualityDefaultsToFalseForMixedTypes", "print 1 == true;", {"false"}},
    {"EqualityWorksInConditions",
     "int count = 0;\n"
     "if (count == 0) print \"done\";",
     {"done"}},

    // Inequality
    {"InequalityWorksWithUnequalNumbers", "print 1 != 2;", {"true"}},
    {"InequalityWorksWithEqualNumbers", "print 1 != 1;", {"false"}},
    {"InequalityWorksWithUnequalStrings", "print \"a\" != \"b\";", {"true"}},
    {"InequalityWorksWithEqualStrings", "print \"a\" != \"a\";", {"false"}},
    {"InequalityWorksWithBools", "print true != false;", {"true"}},
    {"InequalityDefaultsToTrueForMixedTypes", "print 1 != true;", {"true"}},

    // Ordered comparisons
    {"LessThanWorksWithNumbers", "print 1 < 2;", {"true"}},
    {"LessThanWorksWithNumbersFalse", "print 2 < 1;", {"false"}},
    {"LessThanEqualsWorksWithNumbers", "print 2 <= 2;", {"true"}},
    {"GreaterThanWorksWithNumbers", "print 2 > 1;", {"true"}},
    {"GreaterThanEqualsWorksWithNumbers", "print 2 >= 2;", {"true"}},
    {"LessThanComparesStringsAlphabetically", "print \"a\" < \"b\";", {"true"}},
    {"GreaterThanComparesStringsAlphabetically",
     "print \"b\" > \"a\";",
     {"true"}},
    {"LessThanEqualsComparesStringsAlphabetically",
     "print \"a\" <= \"a\";",
     {"true"}},
    {"GreaterThanEqualsComparesStringsAlphabetically",
     "print \"b\" >= \"a\";",
     {"true"}},
    {"OrderedComparisonsWorkInConditions",
     "if (\"a\" < \"b\") print \"ordered\";",
     {"ordered"}},

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
    {"ElsesDoNotRunIfConditionIsTrue",
     "if (true) print \"then\"; else print \"else\";",
     {"then"}},
    {"ElsesRunIfConditionIsFalse",
     "if (false) print \"then\"; else print \"else\";",
     {"else"}},
    {"ElsesAllowBlocks",
     "if (false) { print \"then\"; } else { print \"else\"; }",
     {"else"}},
    {"ElseIfsRunFirstTrueBranch",
     "if (false) print \"if\";\n"
     "else if (true) print \"else if\";\n"
     "else print \"else\";",
     {"else if"}},
    {"ElseIfsRunFinalElse",
     "if (false) print \"if\";\n"
     "else if (false) print \"else if\";\n"
     "else print \"else\";",
     {"else"}},
    {"ElsesSetVariablesFromThenBranch",
     "int a = 0;\n"
     "if (true) a = 1; else a = 2;\n"
     "print a;",
     {"1"}},
    {"ElsesSetVariablesFromElseBranch",
     "int a = 0;\n"
     "if (false) a = 1; else a = 2;\n"
     "print a;",
     {"2"}},

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
     {"0"}},
    {"WhileLoopsCanBeInsideIfStatements",
     "if (true) {\n"
     "int a = 5;\n"
     "while (a) {\n"
     "print a;\n"
     "a = a - 1;\n"
     "}\n"
     "}",
     {"1", "2", "3", "4", "5"}},

    // Functions
    {"FunctionsCanBeDeclared", "void main() { print 1; }", {}},
    {"FunctionsCanBeDeclaredWithParameters",
     "void main(int a, string b) { print 1; }",
     {}},
    {"FunctionsCanBeDeclaredWithReturnType", "int main() { print 1; }", {}},
    {"FunctionsCanBeDeclaredWithReturnTypeAndParameters",
     "int main(int a, string b) { print 1; }",
     {}},
    {"FunctionsCanBeDeclaredWhileUsingParametersInBodies",
     "int main(int a, string b) { print a + b; }",
     {}},
    {"FunctionsCanBeDeclaredMultipleTimes",
     "void main() { print 1; }\n"
     "void extra() { print 2; }",
     {}},
    {"FunctionsCanBeDeclaredInsideFunctions",
     "void main() { \n"
     "void extra() { print 2; }\n"
     "}",
     {}},
    {"FunctionsCanShadowVariablesWithParameters",
     "int a;\n"
     "void main(int a) { \n"
     "print a;"
     "}",
     {}},

    // Calls
    {"CallsCallFunctions",
     "void main() { \n"
     "print \"Func!\";\n"
     "}\n"
     "main();",
     {"Func!"}},
    {"CallsCanCallFunctionsMultipleTimes",
     "void main() { \n"
     "print \"Func!\";\n"
     "}\n"
     "main();\n"
     "main();",
     {"Func!", "Func!"}},
    {"CallsCanCallDifferentFunctions",
     "void a() { \n"
     "print \"A\";\n"
     "}\n"
     "void b() { \n"
     "print \"B\";\n"
     "}\n"
     "a();\n"
     "b();",
     {"A", "B"}},
    {"CallsCanCallFunctionsWithIfStatementsInside",
     "void main() { \n"
     "if (true)\n"
     "print \"A\";\n"
     "if (false)\n"
     "print \"B\";\n"
     "}\n"
     "main();",
     {"A"}},
    {"CallsCanCallFunctionsWithWhileStatementsInside",
     "void main() { \n"
     "int a = 5;\n"
     "while (a) {\n"
     "print a;\n"
     "a = a - 1;\n"
     "}\n"
     "}\n"
     "main();",
     {"5", "4", "3", "2", "1"}},
    {"CallsCanCallFunctionsRecursively",
     "bool recurse = true;\n"
     "void main() {\n"
     "print recurse;\n"
     "if (recurse) {\n"
     "recurse = false;\n"
     "main();\n"
     "}"
     "}\n"
     "main();",
     {"true", "false"}},
    {"CallsAreSequencedCorrectly",
     "int a = 0;\n"
     "void main() {\n"
     "print a;\n"
     "}\n"
     "main();\n"
     "a = a + 1;\n"
     "main();\n"
     "a = a + 1;\n"
     "main();\n",
     {"0", "1", "2"}},
    {"CallsAreSequencedCorrectlyWhenWritesAreInCalls",
     "int a = 0;\n"
     "void main() {\n"
     "a = 1;\n"
     "}\n"
     "main();\n"
     "print a;\n",
     {"1"}},
    {"CallsWorkWithArguments",
     "void add(int a, int b) {\n"
     "print a + b;\n"
     "}\n"
     "add(1, 2);",
     {"3"}},
    {"CallsWorkFromEnclosedFunctionsToEnclosingFunctions",
     "void outer(int x) {\n"
     "void inner(bool y) {\n"
     "outer(0);\n"
     "}\n"
     "print x - 1;\n"
     "if (x)\n"
     "inner(true);\n"
     "print x + 1;\n"
     "}\n"
     "outer(5);\n",
     {"-1", "1", "4", "6"}},

    {"ReturnsWork", "int main() { return 1; }\nprint main();", {"1"}},
    {"ReturnsWorkWithMultipleCalls",
     "int main(bool a) {\n"
     "if (a) return 1;\n"
     "else return 2;\n"
     "}\n"
     "print main(true);\n"
     "print main(false);",
     {"1", "2"}},
    {"ReturnsWorkWithEnclosedFunctions",
     "void outer() {\n"
     "void inner() {\n"
     "return 1;\n"
     "}\n"
     "print inner();\n"
     "print \"done\";\n"
     "}\n"
     "outer();",
     {"1", "done"}},
    {"ReturnsWorkWithMultipleReturnStatementsInOneFunction",
     "int main(int a) {\n"
     "if (a == 0) return 1;\n"
     "if (a == 1) return 2;\n"
     "}\n"
     "print main(0);\n"
     "print main(1);\n", {"1", "2"}},
    {"ReturnsDoNotEndFunctionsPrematurely",
     "int main() {\n"
     "return 0;\n"
     "print \"done\";\n"
     "}\n"
     "print main();",
     {"0", "done"}},
};