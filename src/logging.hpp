#pragma once

#include <format>

template <class... Args>
void logError(const char *where, const std::format_string<Args...> format, Args &&...args);

#include "logging.tpp"