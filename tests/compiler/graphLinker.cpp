#include "../../src/compiler/graphLinker.hpp"
#include <gtest/gtest.h>

TEST(constructor, createsDefaultResources)
{
  GraphLinker *linker = new GraphLinker(std::make_shared<BlockExpression>(BlockExpression()));
}