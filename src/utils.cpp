#include "utils.hpp"
#include <algorithm>

bool beginsWith(std::string str, std::string prefix) {
  return std::strncmp(str.c_str(), prefix.c_str(), prefix.length()) == 0;
}

bool isInteger(const std::string &s) {
  return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit) ||
         s[0] == '-' && std::all_of(s.begin() + 1, s.end(), ::isdigit);
}

std::string formatNs(std::chrono::nanoseconds time) {
  auto ns = time.count();
  auto micros = ns / 1000;
  auto ms = micros / 1000;
  auto secs = ms / 1000;
  auto mins = secs / 60;

  std::string msg = "";

  if (mins > 0)
    msg += std::to_string(mins) + "m";
  if (secs > 0)
    msg += std::to_string(secs % 60) + "s";
  if (ms > 0)
    msg += std::to_string(ms % 1000) + "ms";
  if (micros > 0)
    msg += std::to_string(micros % 1000) + "μs";
  if (ns > 0)
    msg += std::to_string(ns % 1000) + "ns";

  return msg;
}
