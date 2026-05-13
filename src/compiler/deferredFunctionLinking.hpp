#pragma once

#include "../interpreter/function.hpp"
#include "resource.hpp"
#include <memory>

struct DeferredFunctionLinking {
  std::shared_ptr<Function> function;
  std::shared_ptr<Scope<Resource>> scope;

  bool operator==(const DeferredFunctionLinking &other) const {
    return function.get() == other.function.get();
  }
};

// Custom hasher for DeferredFunctionLinking
namespace std {
template <> struct hash<DeferredFunctionLinking> {
  std::size_t operator()(const DeferredFunctionLinking &defer) const noexcept {
    // Reinterpret the memory address as a size_t
    return reinterpret_cast<size_t>(defer.function.get());
  }
};
} // namespace std