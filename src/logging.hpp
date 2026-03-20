#pragma once

#include <format>

#define DEFINE_LOG(name) template <class... Args> \
void name(const char *where, const std::format_string<Args...> format, Args &&...args);

#define DECLARE_LOG(name) template <class... Args> \
void name(const char *where, const std::format_string<Args...> format, Args &&...args)

DEFINE_LOG(log)
DEFINE_LOG(logError)

#include "logging.tpp"