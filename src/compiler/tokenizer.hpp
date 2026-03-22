#pragma once

#include "token.hpp"
#include <memory>
#include <vector>
#include <fstream>

std::unique_ptr<std::vector<Token>> tokenize(std::ifstream stream);