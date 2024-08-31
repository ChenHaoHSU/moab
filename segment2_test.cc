#include "segment2.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

#include "absl/hash/hash_testing.h"
#include "absl/strings/str_format.h"

namespace moab {

using ::testing::Pair;
using ::testing::StrEq;
TEST(Constructors, Default) {
  Segment2_i s;

  EXPECT_EQ(s.p1().x(), 0);
  EXPECT_EQ(s.p1().y(), 0);
  EXPECT_EQ(s.p2().x(), 0);
  EXPECT_EQ(s.p2().y(), 0);
}
}  // namespace moab