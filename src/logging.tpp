#include <iostream>
#include <utility>

template <class... Args>
static void logBase(const char *type, const char *where,
                    const std::format_string<Args...> format, Args &&...args) {
  // Build up a single string, as only each << is thread-safe for cout
  std::string msg =
      std::format("[{} at {}]: {}\n", type, where,
                  std::format(format, std::forward<Args>(args)...));
  std::cout << msg;
}

DECLARE_LOG(log, "Info")

DECLARE_LOG(logError, "Error")

DECLARE_LOG(logWarning, "Warning")