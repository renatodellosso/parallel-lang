#pragma once

#include "token.hpp"
#include <fstream>
#include <memory>
#include <vector>

class Tokenizer {
private:
  std::istream &stream;
  std::unique_ptr<std::vector<Token>> tokens;
  int line;

  void skipWhitespace();
  void parseToken();

public:
  Tokenizer(std::istream &stream)
      : stream(stream), tokens(std::make_unique<std::vector<Token>>()),
        line(1) {}
  /**
   * Parses everything from the stream
   */
  void parse();
  /**
   * Closes the tokenizer and returns the tokens
   */
  std::unique_ptr<std::vector<Token>> close();
};