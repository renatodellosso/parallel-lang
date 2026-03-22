#include <iostream>
#include <utility>

template <class... Args>
static void logBase(const char *type, const char *where,
                    const std::format_string<Args...> format, Args &&...args)
{
  std::cout << "[" << type << " at " << where << "]: " << std::format(format, std::forward<Args>(args)...) << "\n";
}

DECLARE_LOG(log, "Info")

DECLARE_LOG(logError, "Error")

DECLARE_LOG(logWarning, "Warning")