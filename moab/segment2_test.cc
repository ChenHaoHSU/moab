#include "segment2.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

#include "absl/hash/hash_testing.h"
#include "absl/strings/str_format.h"

#include "point2.h"

namespace moab {

using ::testing::Pair;
using ::testing::StrEq;

TEST(Constructors, Default) {
  Segment2_i s;

  EXPECT_EQ(s.p0(), Point2_i(0, 0));
  EXPECT_EQ(s.p1(), Point2_i(0, 0));
}

TEST(Constructors, TwoPoints) {
  Segment2_i s(Point2_i(1, 2), Point2_i(3, 4));

  EXPECT_EQ(s.p0(), Point2_i(1, 2));
  EXPECT_EQ(s.p1(), Point2_i(3, 4));
}

TEST(Constructors, MinMaxXY) {
  Segment2_i s(1, 2, 3, 4);

  EXPECT_EQ(s.p0(), Point2_i(1, 2));
  EXPECT_EQ(s.p1(), Point2_i(3, 4));
}

TEST(Constructors, Copy) {
  Segment2_i s1(Point2_i(1, 2), Point2_i(3, 4));
  Segment2_i s2(s1);

  EXPECT_EQ(s2.p0(), Point2_i(1, 2));
  EXPECT_EQ(s2.p1(), Point2_i(3, 4));
}

TEST(Constructors, Move) {
  Segment2_i s1(Point2_i(1, 2), Point2_i(3, 4));
  Segment2_i s2(std::move(s1));

  EXPECT_EQ(s2.p0(), Point2_i(1, 2));
  EXPECT_EQ(s2.p1(), Point2_i(3, 4));
}

TEST(AssignmentOperators, Copy) {
  Segment2_i s1(Point2_i(1, 2), Point2_i(3, 4));
  Segment2_i s2;

  s2 = s1;

  EXPECT_EQ(s2.p0(), Point2_i(1, 2));
  EXPECT_EQ(s2.p1(), Point2_i(3, 4));
}

TEST(AssignmentOperators, Move) {
  Segment2_i s1(Point2_i(1, 2), Point2_i(3, 4));
  Segment2_i s2;

  s2 = std::move(s1);

  EXPECT_EQ(s2.p0(), Point2_i(1, 2));
  EXPECT_EQ(s2.p1(), Point2_i(3, 4));
}

TEST(Accessors, P0P1) {
  Segment2_i s(Point2_i(1, 2), Point2_i(3, 4));

  EXPECT_EQ(s.p0(), Point2_i(1, 2));
  EXPECT_EQ(s.p1(), Point2_i(3, 4));
}

TEST(Accessors, P0P1Const) {
  const Segment2_i s(Point2_i(1, 2), Point2_i(3, 4));

  EXPECT_EQ(s.p0(), Point2_i(1, 2));
  EXPECT_EQ(s.p1(), Point2_i(3, 4));
}

TEST(Accessors, Data) {
  Segment2_i s(Point2_i(1, 2), Point2_i(3, 4));
  Point2_i* data = s.data();

  data[1] = Point2_i(5, 6);

  EXPECT_EQ(data[0], Point2_i(1, 2));
  EXPECT_EQ(data[1], Point2_i(5, 6));
}

TEST(Accessors, DataConst) {
  const Segment2_i s(Point2_i(1, 2), Point2_i(3, 4));
  const Point2_i* data = s.data();

  EXPECT_EQ(data[0], Point2_i(1, 2));
  EXPECT_EQ(data[1], Point2_i(3, 4));
}

TEST(Accessors, Size) {
  Segment2_i s(Point2_i(1, 2), Point2_i(3, 4));

  EXPECT_EQ(s.Size(), 2);
}

TEST(Accessors, ToPair) {
  Segment2_i s(Point2_i(1, 2), Point2_i(3, 4));
  std::pair<Point2_i, Point2_i> p = s.ToPair();

  EXPECT_THAT(p, Pair(Point2_i(1, 2), Point2_i(3, 4)));
}

TEST(Accessors, Length) {
  Segment2_i s(Point2_i(1, 2), Point2_i(3, 4));

  EXPECT_EQ(s.Length(), 4);
}

TEST(Accessors, XLYLXHYH1) {
  Segment2_i s(Point2_i(1, 2), Point2_i(3, 4));

  EXPECT_EQ(s.xl(), 1);
  EXPECT_EQ(s.yl(), 2);
  EXPECT_EQ(s.xh(), 3);
  EXPECT_EQ(s.yh(), 4);
}

TEST(Accessors, XLYLXHYH2) {
  Segment2_i s(Point2_i(3, 4), Point2_i(1, 2));

  EXPECT_EQ(s.xl(), 1);
  EXPECT_EQ(s.yl(), 2);
  EXPECT_EQ(s.xh(), 3);
  EXPECT_EQ(s.yh(), 4);
}

TEST(Accessors, MinMaxXY1) {
  Segment2_i s(Point2_i(1, 2), Point2_i(3, 4));

  EXPECT_EQ(s.MinX(), 1);
  EXPECT_EQ(s.MinY(), 2);
  EXPECT_EQ(s.MaxX(), 3);
  EXPECT_EQ(s.MaxY(), 4);
}

TEST(Accessors, MinMaxXY2) {
  Segment2_i s(Point2_i(3, 4), Point2_i(1, 2));

  EXPECT_EQ(s.MinX(), 1);
  EXPECT_EQ(s.MinY(), 2);
  EXPECT_EQ(s.MaxX(), 3);
  EXPECT_EQ(s.MaxY(), 4);
}

TEST(Mutators, SetByFourCoordinates) {
  Segment2_i s(Point2_i(1, 2), Point2_i(3, 4));
  s.Set(5, 6, 7, 8);

  EXPECT_EQ(s.p0(), Point2_i(5, 6));
  EXPECT_EQ(s.p1(), Point2_i(7, 8));
}

TEST(Mutators, SetByTwoPoints) {
  Segment2_i s(Point2_i(1, 2), Point2_i(3, 4));
  s.Set(Point2_i(5, 6), Point2_i(7, 8));

  EXPECT_EQ(s.p0(), Point2_i(5, 6));
  EXPECT_EQ(s.p1(), Point2_i(7, 8));
}

TEST(Mutators, SetP0P1) {
  Segment2_i s(Point2_i(1, 2), Point2_i(3, 4));
  s.SetP0(Point2_i(5, 6));
  s.SetP1(Point2_i(7, 8));

  EXPECT_EQ(s.p0(), Point2_i(5, 6));
  EXPECT_EQ(s.p1(), Point2_i(7, 8));
}

TEST(Mutators, SetP) {
  Segment2_i s(Point2_i(1, 2), Point2_i(3, 4));
  s.SetP(0, Point2_i(5, 6));
  s.SetP(1, Point2_i(7, 8));

  EXPECT_EQ(s.p0(), Point2_i(5, 6));
  EXPECT_EQ(s.p1(), Point2_i(7, 8));
}

TEST(Operations, Shift) {
  Segment2_i s(Point2_i(1, 2), Point2_i(3, 4));
  s.Shift(1, 2);

  EXPECT_EQ(s.p0(), Point2_i(2, 4));
  EXPECT_EQ(s.p1(), Point2_i(4, 6));
}

TEST(Operations, ShiftX) {
  Segment2_i s(Point2_i(1, 2), Point2_i(3, 4));
  s.ShiftX(1);

  EXPECT_EQ(s.p0(), Point2_i(2, 2));
  EXPECT_EQ(s.p1(), Point2_i(4, 4));
}

TEST(Operations, ShiftY) {
  Segment2_i s(Point2_i(1, 2), Point2_i(3, 4));
  s.ShiftY(1);

  EXPECT_EQ(s.p0(), Point2_i(1, 3));
  EXPECT_EQ(s.p1(), Point2_i(3, 5));
}

TEST(Operators, SubscriptAccess) {
  Segment2_i s(Point2_i(1, 2), Point2_i(3, 4));

  EXPECT_EQ(s[0], Point2_i(1, 2));
  EXPECT_EQ(s[1], Point2_i(3, 4));
}

TEST(Operators, SubscriptMutation) {
  Segment2_i s(Point2_i(1, 2), Point2_i(3, 4));
  s[0] = Point2_i(5, 6);
  s[1] = Point2_i(7, 8);

  EXPECT_EQ(s[0], Point2_i(5, 6));
  EXPECT_EQ(s[1], Point2_i(7, 8));
}

TEST(Operators, Equality) {
  Segment2_i s1(Point2_i(1, 2), Point2_i(3, 4));
  Segment2_i s2(Point2_i(1, 2), Point2_i(3, 4));
  Segment2_i s3(Point2_i(5, 6), Point2_i(7, 8));

  EXPECT_TRUE(s1 == s2);
  EXPECT_FALSE(s1 == s3);
  EXPECT_EQ(s1, s2);
}

TEST(Operators, NotEquality) {
  Segment2_i s1(Point2_i(1, 2), Point2_i(3, 4));
  Segment2_i s2(Point2_i(1, 2), Point2_i(3, 4));
  Segment2_i s3(Point2_i(5, 6), Point2_i(7, 8));
  Segment2_i s4(Point2_i(1, 2), Point2_i(3, 5));
  Segment2_i s5(Point2_i(1, 2), Point2_i(4, 4));

  EXPECT_FALSE(s1 != s2);
  EXPECT_TRUE(s1 != s3);
  EXPECT_TRUE(s1 != s4);
  EXPECT_TRUE(s1 != s5);
  EXPECT_NE(s1, s3);
  EXPECT_NE(s1, s4);
  EXPECT_NE(s1, s5);
}

TEST(Operators, Inequality1) {
  Segment2_i s1(Point2_i(1, 2), Point2_i(3, 4));
  Segment2_i s2(Point2_i(3, 4), Point2_i(5, 6));

  EXPECT_TRUE(s1 < s2);
  EXPECT_FALSE(s1 > s2);
  EXPECT_TRUE(s2 > s1);
  EXPECT_FALSE(s2 < s1);
  EXPECT_TRUE(s1 <= s2);
  EXPECT_FALSE(s1 >= s2);
  EXPECT_TRUE(s2 >= s1);
  EXPECT_FALSE(s2 <= s1);
}

TEST(Operators, Inequality2) {
  Segment2_i s1(Point2_i(1, 2), Point2_i(3, 4));
  Segment2_i s2(Point2_i(1, 2), Point2_i(3, 4));

  EXPECT_FALSE(s1 < s2);
  EXPECT_FALSE(s1 > s2);
  EXPECT_FALSE(s2 > s1);
  EXPECT_FALSE(s2 < s1);
  EXPECT_TRUE(s1 <= s2);
  EXPECT_TRUE(s1 >= s2);
  EXPECT_TRUE(s2 >= s1);
  EXPECT_TRUE(s2 <= s1);
}

TEST(StringConversion, ToString) {
  Segment2_i s(Point2_i(1, 2), Point2_i(3, 4));

  EXPECT_THAT(s.ToString(), StrEq("((1 2) (3 4))"));
}

TEST(StringConversion, SupportsAbslStringify) {
  Segment2_i seg(Point2_i(1, 2), Point2_i(3, 4));
  std::string s = absl::StrFormat("%v", seg);

  EXPECT_THAT(s, StrEq("((1 2) (3 4))"));
}

TEST(Hash, SupportsAbslHash) {
  EXPECT_TRUE(absl::VerifyTypeImplementsAbslHashCorrectly({
      Segment2_i(),
      Segment2_i(1, 2, 3, 4),
      Segment2_i(1, 2, 3, 4),
      Segment2_i(Point2_i(1, 2), Point2_i(3, 4)),
      Segment2_i(Point2_i(4, 5), Point2_i(7, 6)),
  }));
}
}  // namespace moab