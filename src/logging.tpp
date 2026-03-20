#include <iostream>

template <class... Args>
void logError(const char *where, std::format_string<Args...> format, Args &&...args)
{
  std::cout << "[ERROR at " << where << "]: " << std::format(format, args...) << "\n";
};