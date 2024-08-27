#include "point2.h"

#include <gtest/gtest.h>

namespace moab {

TEST(Constructor, Default) {
  Point2_i p;

  EXPECT_EQ(p.x(), 0);
  EXPECT_EQ(p.y(), 0);
}

TEST(Constructor, XY) {
  Point2_i p(1, 2);

  EXPECT_EQ(p.x(), 1);
  EXPECT_EQ(p.y(), 2);
}

TEST(Constructor, Copy) {
  Point2_i p1(1, 2);
  Point2_i p2(p1);

  EXPECT_EQ(p2.x(), 1);
  EXPECT_EQ(p2.y(), 2);
}

TEST(Constructor, Move) {
  Point2_i p1(1, 2);
  Point2_i p2(std::move(p1));

  EXPECT_EQ(p2.x(), 1);
  EXPECT_EQ(p2.y(), 2);
}

TEST(Operator, Equal) {
  Point2_i p1(1, 2);
  Point2_i p2 = p1;

  EXPECT_EQ(p2.x(), 1);
  EXPECT_EQ(p2.y(), 2);

  Point2_i p3;
  EXPECT_EQ(p3.x(), 0);
  EXPECT_EQ(p3.y(), 0);

  p3 = p2;
  EXPECT_EQ(p3.x(), 1);
  EXPECT_EQ(p3.y(), 2);
}

}  // namespace moab