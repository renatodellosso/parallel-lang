#pragma once

#include <cstddef>
#include <mutex>
#include <queue>
#include <variant>

template <class T> class ConcurrentQueue {
  std::queue<T> queue;
  std::mutex mutex;

public:
  ConcurrentQueue();

  std::variant<T, nullptr_t> pop();
  void push(T val);
  int size();
};

template <class T> inline ConcurrentQueue<T>::ConcurrentQueue() {}

template <class T> inline std::variant<T, nullptr_t> ConcurrentQueue<T>::pop() {
  const std::lock_guard<std::mutex> lock(mutex);

  if (queue.size() == 0)
    return nullptr;

  auto t = queue.front();
  queue.pop();

  return t;
}

template <class T> inline void ConcurrentQueue<T>::push(T val) {
  const std::lock_guard<std::mutex> lock(mutex);
  queue.push(val);
}

template <class T> inline int ConcurrentQueue<T>::size() {
  const std::lock_guard<std::mutex> lock(mutex);
  auto size = queue.size();

  return size;
}
