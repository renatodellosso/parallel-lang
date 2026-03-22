#include "tokenizer.hpp"

std::unique_ptr<std::vector<Token>> tokenize(std::ifstream stream)
{
  auto tokens = std::make_unique<std::vector<Token>>();

  std::string word;
  while (stream >> word)
  {
  }

  return tokens;
}