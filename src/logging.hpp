#pragma once

#include <format>

#define DEFINE_LOG(name) template <class... Args> \
void name(const char *where, const std::format_string<Args...> format, Args &&...args);

#define DECLARE_LOG(name, type)                                                          \
  template <class... Args>                                                               \
  void name(const char *where, const std::format_string<Args...> format, Args &&...args) \
  {                                                                                      \
    logBase(type, where, format, std::forward<Args>(args)...);                           \
  }

DEFINE_LOG(log)
DEFINE_LOG(logError)
DEFINE_LOG(logWarning)

#include "logging.tpp"