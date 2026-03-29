#pragma once

enum class InstructionType
{
  Block,
  GetLiteral,
  GetIdentifier,
  Set,
  Add,
  Subtract,
  Multiply,
  Divide,
  Negate,
  CompareEquals,
  CompareLessThan,
  CompareLessThanEquals,
  CompareGreaterThan,
  CompareGreaterThanEquals
};
