#pragma once

#include <string>

// strncmp is in cstring on Ubunuty
#ifdef __unix__
#include <cstring>
#endif

bool beginsWith(std::string str, std::string prefix);
bool isInteger(const std::string &s);