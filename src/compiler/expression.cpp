#include "expression.hpp"

std::string Expression::toString() const
{
  switch (type)
  {
  case InstructionType::Block:
    return "Block";
  case InstructionType::GetLiteral:
    return "GetLiteral";
  case InstructionType::GetIdentifier:
    return "GetIdentifier";
  case InstructionType::Set:
    return "Set";
  case InstructionType::Add:
    return "Add";
  case InstructionType::Subtract:
    return "Subtract";
  case InstructionType::Multiply:
    return "Multiply";
  case InstructionType::Divide:
    return "Divide";
  case InstructionType::Negate:
    return "Negate";
  case InstructionType::CompareEquals:
    return "CompareEquals";
  case InstructionType::CompareLessThan:
    return "CompareLessThan";
  case InstructionType::CompareLessThanEquals:
    return "CompareLessThanEquals";
  case InstructionType::CompareGreaterThan:
    return "CompareGreaterThan";
  case InstructionType::CompareGreaterThanEquals:
    return "CompareGreaterThanEquals";
  default:
    return "Unknown Expression Type";
  }
}

std::string Expression::toByteCode() const
{
  return std::to_string((int)type);
}

std::string RootExpression::toString() const
{
  return Expression::toString() + "(" + token.raw + ")";
}

std::string RootExpression::toByteCode() const
{
  return Expression::toByteCode() + " " + token.raw;
}

std::string UnaryExpression::toString() const
{
  return Expression::toString() + "(" + root.get()->toString() + ")";
}

std::string UnaryExpression::toByteCode() const
{
  return root->toByteCode() + "\n" + Expression::toByteCode();
}

std::string BinaryExpression::toString() const
{
  return Expression::toString() + "(" + left.get()->toString() + ", " + right.get()->toString() + ")";
}

std::string BinaryExpression::toByteCode() const
{
  return left->toByteCode() + "\n" + right->toByteCode() + "\n" + Expression::toByteCode();
}

std::string BlockExpression::toString() const
{
  auto str = Expression::toString() + " {\n";

  // Use references
  for (auto &line : expressions)
  {
    str += line.get()->toString() + "\n";
  }

  return str + "}";
}

std::string BlockExpression::toByteCode() const
{
  std::string str;

  // Use references
  for (auto &line : expressions)
  {
    str += line->toByteCode() + "\n";
  }

  return str;
}
