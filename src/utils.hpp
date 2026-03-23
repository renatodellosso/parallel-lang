#pragma once

#include <string>

#ifdef __unix__
#include <cstring>
#endif

bool beginsWith(std::string str, std::string prefix);