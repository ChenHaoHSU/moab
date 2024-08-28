#include "point3.h"

#include <gtest/gtest.h>

namespace moab {

TEST(Constructor, Default) {
  Point3_i p;

  EXPECT_EQ(p.x(), 0);
  EXPECT_EQ(p.y(), 0);
  EXPECT_EQ(p.z(), 0);
}

TEST(Constructor, XY) {
  Point3_i p(1, 2, 3);

  EXPECT_EQ(p.x(), 1);
  EXPECT_EQ(p.y(), 2);
  EXPECT_EQ(p.z(), 3);
}

TEST(Constructor, Copy) {
  Point3_i p1(1, 2, 3);
  Point3_i p2(p1);

  EXPECT_EQ(p2.x(), 1);
  EXPECT_EQ(p2.y(), 2);
  EXPECT_EQ(p2.z(), 3);
}

TEST(Constructor, Move) {
  Point3_i p1(1, 2, 3);
  Point3_i p2(std::move(p1));

  EXPECT_EQ(p2.x(), 1);
  EXPECT_EQ(p2.y(), 2);
  EXPECT_EQ(p2.z(), 3);
}

TEST(Operator, Equal) {
  Point3_i p1(1, 2, 3);
  Point3_i p2 = p1;

  EXPECT_EQ(p2.x(), 1);
  EXPECT_EQ(p2.y(), 2);
  EXPECT_EQ(p2.z(), 3);

  Point3_i p3;
  EXPECT_EQ(p3.x(), 0);
  EXPECT_EQ(p3.y(), 0);
  EXPECT_EQ(p3.z(), 0);

  p3 = p2;
  EXPECT_EQ(p3.x(), 1);
  EXPECT_EQ(p3.y(), 2);
  EXPECT_EQ(p3.z(), 3);
}

}  // namespace moab