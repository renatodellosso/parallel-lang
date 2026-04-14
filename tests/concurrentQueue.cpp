#include "../src/concurrentQueue.hpp"
#include <cstddef>
#include <gtest/gtest.h>

TEST(size, sizeStartAtZero) {
  ConcurrentQueue<int> queue;

  EXPECT_EQ(queue.size(), 0);
}

TEST(push, incrementsSize) {
  ConcurrentQueue<int> queue;

  for (int i = 1; i <= 10; i++) {
    queue.push(i);
    EXPECT_EQ(queue.size(), i);
  }
}

TEST(pushPop, pushesToBack) {
  ConcurrentQueue<int> queue;

  queue.push(1);
  queue.push(2);
  queue.push(3);

  EXPECT_EQ(std::get<int>(queue.pop()), 1);
  EXPECT_EQ(std::get<int>(queue.pop()), 2);
  EXPECT_EQ(std::get<int>(queue.pop()), 3);
}

TEST(pushPop, returnsNullptrIfEmpty) {
  ConcurrentQueue<int> queue;

  EXPECT_EQ(std::get<nullptr_t>(queue.pop()), nullptr);
}