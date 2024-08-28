#include "box2.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

#include "absl/hash/hash_testing.h"
#include "point2.h"

namespace moab {

using ::testing::Pair;
using ::testing::StrEq;

TEST(Constructors, Default) {
  Box2_i b;

  EXPECT_EQ(b.ll(), Point2_i(0, 0));
  EXPECT_EQ(b.ur(), Point2_i(0, 0));
}

TEST(Constructors, TwoPoints) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 4));

  EXPECT_EQ(b.ll(), Point2_i(1, 2));
  EXPECT_EQ(b.ur(), Point2_i(3, 4));
}

TEST(Constructors, MinMaxXY) {
  Box2_i b(1, 2, 3, 4);

  EXPECT_EQ(b.ll(), Point2_i(1, 2));
  EXPECT_EQ(b.ur(), Point2_i(3, 4));
}

TEST(Constructors, Copy) {
  Box2_i b1(Point2_i(1, 2), Point2_i(3, 4));
  Box2_i b2(b1);

  EXPECT_EQ(b2.ll(), Point2_i(1, 2));
  EXPECT_EQ(b2.ur(), Point2_i(3, 4));
}

TEST(Constructors, Move) {
  Box2_i b1(Point2_i(1, 2), Point2_i(3, 4));
  Box2_i b2(std::move(b1));

  EXPECT_EQ(b2.ll(), Point2_i(1, 2));
  EXPECT_EQ(b2.ur(), Point2_i(3, 4));
}

TEST(Accessors, LLCornerURCorner) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 4));

  EXPECT_EQ(b.ll(), Point2_i(1, 2));
  EXPECT_EQ(b.ur(), Point2_i(3, 4));
}

TEST(Accessors, MinCornerMaxCorner) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 4));

  EXPECT_EQ(b.MinCorner(), Point2_i(1, 2));
  EXPECT_EQ(b.MaxCorner(), Point2_i(3, 4));
}

TEST(Accessors, XLYLXHYH) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 4));

  EXPECT_EQ(b.xl(), 1);
  EXPECT_EQ(b.yl(), 2);
  EXPECT_EQ(b.xh(), 3);
  EXPECT_EQ(b.yh(), 4);
}

TEST(Accessors, WidthHeight) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 5));

  EXPECT_EQ(b.Width(), 2);
  EXPECT_EQ(b.Height(), 3);
}

TEST(Accessors, Area) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 5));

  EXPECT_EQ(b.Area(), 6);
}

TEST(Accessors, Perimeter) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 5));

  EXPECT_EQ(b.Perimeter(), 10);
}

TEST(Mutators, Set) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 4));
  b.Set(Point2_i(5, 6), Point2_i(7, 8));

  EXPECT_EQ(b.ll(), Point2_i(5, 6));
  EXPECT_EQ(b.ur(), Point2_i(7, 8));
}

TEST(Mutators, SetLL) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 4));
  b.SetLL(Point2_i(0, 1));

  EXPECT_EQ(b.ll(), Point2_i(0, 1));
  EXPECT_EQ(b.ur(), Point2_i(3, 4));
}

TEST(Mutators, SetUR) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 4));
  b.SetUR(Point2_i(5, 6));

  EXPECT_EQ(b.ll(), Point2_i(1, 2));
  EXPECT_EQ(b.ur(), Point2_i(5, 6));
}

TEST(Mutators, SetMinCorner) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 4));
  b.SetMinCorner(Point2_i(0, 1));

  EXPECT_EQ(b.ll(), Point2_i(0, 1));
  EXPECT_EQ(b.ur(), Point2_i(3, 4));
}

TEST(Mutators, SetMaxCorner) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 4));
  b.SetMaxCorner(Point2_i(5, 6));

  EXPECT_EQ(b.ll(), Point2_i(1, 2));
  EXPECT_EQ(b.ur(), Point2_i(5, 6));
}

TEST(Operations, Shift) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 4));
  b.Shift(1, 2);

  EXPECT_EQ(b.ll(), Point2_i(2, 4));
  EXPECT_EQ(b.ur(), Point2_i(4, 6));
}

TEST(Operations, ShiftX) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 4));
  b.ShiftX(1);

  EXPECT_EQ(b.ll(), Point2_i(2, 2));
  EXPECT_EQ(b.ur(), Point2_i(4, 4));
}

TEST(Operations, ShiftY) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 4));
  b.ShiftY(1);

  EXPECT_EQ(b.ll(), Point2_i(1, 3));
  EXPECT_EQ(b.ur(), Point2_i(3, 5));
}

TEST(Operators, Assignment) {
  Box2_i b1(Point2_i(1, 2), Point2_i(3, 4));
  Box2_i b2;

  b2 = b1;

  EXPECT_EQ(b2.ll(), Point2_i(1, 2));
  EXPECT_EQ(b2.ur(), Point2_i(3, 4));
}

TEST(Operators, SubscriptAccess) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 4));

  EXPECT_EQ(b[0], Point2_i(1, 2));
  EXPECT_EQ(b[1], Point2_i(3, 4));
}

TEST(Operators, SubscriptMutation) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 4));
  b[0] = Point2_i(5, 6);
  b[1] = Point2_i(7, 8);

  EXPECT_EQ(b[0], Point2_i(5, 6));
  EXPECT_EQ(b[1], Point2_i(7, 8));
}

TEST(Operators, Equality) {
  Box2_i b1(Point2_i(1, 2), Point2_i(3, 4));
  Box2_i b2(Point2_i(1, 2), Point2_i(3, 4));
  Box2_i b3(Point2_i(5, 6), Point2_i(7, 8));

  EXPECT_TRUE(b1 == b2);
  EXPECT_FALSE(b1 == b3);
  EXPECT_EQ(b1, b2);
}

TEST(Operators, NotEquality) {
  Box2_i b1(Point2_i(1, 2), Point2_i(3, 4));
  Box2_i b2(Point2_i(1, 2), Point2_i(3, 4));
  Box2_i b3(Point2_i(5, 6), Point2_i(7, 8));
  Box2_i b4(Point2_i(1, 2), Point2_i(3, 5));
  Box2_i b5(Point2_i(1, 2), Point2_i(4, 4));

  EXPECT_FALSE(b1 != b2);
  EXPECT_TRUE(b1 != b3);
  EXPECT_TRUE(b1 != b4);
  EXPECT_TRUE(b1 != b5);
  EXPECT_NE(b1, b3);
  EXPECT_NE(b1, b4);
  EXPECT_NE(b1, b5);
}

TEST(Operators, Inequality1) {
  Box2_i b1(Point2_i(1, 2), Point2_i(3, 4));
  Box2_i b2(Point2_i(3, 4), Point2_i(5, 6));

  EXPECT_TRUE(b1 < b2);
  EXPECT_FALSE(b1 > b2);
  EXPECT_TRUE(b2 > b1);
  EXPECT_FALSE(b2 < b1);
  EXPECT_TRUE(b1 <= b2);
  EXPECT_FALSE(b1 >= b2);
  EXPECT_TRUE(b2 >= b1);
  EXPECT_FALSE(b2 <= b1);
}

TEST(Operators, Inequality2) {
  Box2_i b1(Point2_i(1, 2), Point2_i(3, 4));
  Box2_i b2(Point2_i(1, 2), Point2_i(3, 4));

  EXPECT_FALSE(b1 < b2);
  EXPECT_FALSE(b1 > b2);
  EXPECT_FALSE(b2 > b1);
  EXPECT_FALSE(b2 < b1);
  EXPECT_TRUE(b1 <= b2);
  EXPECT_TRUE(b1 >= b2);
  EXPECT_TRUE(b2 >= b1);
  EXPECT_TRUE(b2 <= b1);
}

}  // namespace moab