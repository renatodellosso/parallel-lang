#pragma once

#include <memory>
#include <shared_mutex>
#include <unordered_map>

template <class T> class Scope {
  // May be nullptr!
  std::shared_ptr<Scope> enclosing;
  std::unordered_map<std::string, std::shared_ptr<T>> vars;

  std::shared_mutex mutex;

public:
  Scope(std::shared_ptr<Scope> enclosing,
        std::unordered_map<std::string, std::shared_ptr<T>> vars);
  Scope(std::unordered_map<std::string, std::shared_ptr<T>> vars);
  Scope(std::shared_ptr<Scope> enclosing);
  Scope();

  std::shared_ptr<T> alloc(std::string key, T val = {});
  std::shared_ptr<T> get(std::string key);

  int getDepth() const;
  std::shared_ptr<Scope> getEnclosing();
  std::unordered_map<std::string, std::shared_ptr<T>> &getVarTable();
};

template <class T>
inline Scope<T>::Scope(std::shared_ptr<Scope> enclosing,
                       std::unordered_map<std::string, std::shared_ptr<T>> vars)
    : enclosing(enclosing), vars(vars) {}

template <class T>
inline Scope<T>::Scope(std::unordered_map<std::string, std::shared_ptr<T>> vars)
    : Scope(nullptr, vars) {}

template <class T>
inline Scope<T>::Scope(std::shared_ptr<Scope> enclosing)
    : Scope(enclosing, std::unordered_map<std::string, std::shared_ptr<T>>()) {}

template <class T> inline Scope<T>::Scope() : Scope(nullptr) {}

template <class T>
inline std::shared_ptr<T> Scope<T>::alloc(std::string key, T val) {
  std::unique_lock<std::shared_mutex> lock(mutex);

  auto ptr = std::make_shared<T>(val);
  vars[key] = ptr;

  return ptr;
}

template <class T> inline std::shared_ptr<T> Scope<T>::get(std::string key) {
  std::shared_lock<std::shared_mutex> lock(mutex);
  auto it = vars.find(key);
  lock.unlock(); // Unlock early since we're done reading from vars

  if (it == vars.end()) {
    // Not found here
    if (enclosing)
      return enclosing->get(key);
    else
      return nullptr;
  }

  return it->second;
}

template <class T> inline int Scope<T>::getDepth() const {
  if (enclosing)
    return enclosing->getDepth() + 1;
  return 1;
}

template <class T> inline std::shared_ptr<Scope<T>> Scope<T>::getEnclosing() {
  return enclosing;
}

template <class T>
inline std::unordered_map<std::string, std::shared_ptr<T>> &
Scope<T>::getVarTable() {
  return vars;
}