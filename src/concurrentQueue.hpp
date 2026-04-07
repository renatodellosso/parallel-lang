#pragma once

#include <queue>
#include <mutex>

template <class T>
class ConcurrentQueue
{
  std::queue<T> queue;
  std::mutex mutex;

public:
  ConcurrentQueue();

  T pop();
  void push(T val);
  int size();
};

template <class T>
inline ConcurrentQueue<T>::ConcurrentQueue() {}

template <class T>
inline T ConcurrentQueue<T>::pop()
{
  mutex.lock();

  auto t = queue.front();
  queue.pop();

  mutex.unlock();

  return t;
}

template <class T>
inline void ConcurrentQueue<T>::push(T val)
{
  mutex.lock();
  queue.push(val);
  mutex.unlock();
}

template <class T>
inline int ConcurrentQueue<T>::size()
{
  mutex.lock();
  auto size = queue.size();
  mutex.unlock();

  return size;
}
