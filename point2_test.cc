#include "point2.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

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
  Point2_i p(Point2_i(1, 2));

  EXPECT_EQ(p.x(), 1);
  EXPECT_EQ(p.y(), 2);
}

TEST(Accessor, XY) {
  Point2_i p(1, 2);

  EXPECT_EQ(p.x(), 1);
  EXPECT_EQ(p.y(), 2);
}

TEST(Accessor, Data) {
  Point2_i p(1, 2);
  std::array<int, 2> d{1, 2};

  EXPECT_EQ(p.data(), d);
  EXPECT_EQ(p.data().size(), 2);
}

TEST(Accessor, Size) {
  Point2_i p;

  EXPECT_EQ(p.Size(), 2);
}

TEST(Mutator, Set) {
  Point2_i p(1, 2);
  p.Set(3, 4);

  EXPECT_EQ(p.x(), 3);
  EXPECT_EQ(p.y(), 4);
}

TEST(Mutator, SetX) {
  Point2_i p(1, 2);
  p.SetX(3);

  EXPECT_EQ(p.x(), 3);
  EXPECT_EQ(p.y(), 2);
}

TEST(Mutator, SetY) {
  Point2_i p(1, 2);
  p.SetY(4);

  EXPECT_EQ(p.x(), 1);
  EXPECT_EQ(p.y(), 4);
}

TEST(Mutator, Shift) {
  Point2_i p(1, 2);
  p.Shift(3, 4);

  EXPECT_EQ(p.x(), 4);
  EXPECT_EQ(p.y(), 6);
}

TEST(Mutator, ShiftX) {
  Point2_i p(1, 2);
  p.ShiftX(3);

  EXPECT_EQ(p.x(), 4);
  EXPECT_EQ(p.y(), 2);
}

TEST(Mutator, ShiftY) {
  Point2_i p(1, 2);
  p.ShiftY(4);

  EXPECT_EQ(p.x(), 1);
  EXPECT_EQ(p.y(), 6);
}

TEST(Operator, Assignment) {
  Point2_i p1(1, 2);
  Point2_i p2;
  p2 = p1;

  EXPECT_EQ(p2.x(), 1);
  EXPECT_EQ(p2.y(), 2);
}

TEST(Operator, Equality) {
  Point2_i p1(1, 2);
  Point2_i p2(1, 2);

  EXPECT_TRUE(p1 == p2);
  EXPECT_EQ(p1, p2);
}

TEST(Operator, NotEquality) {
  Point2_i p1(1, 2);
  Point2_i p2(1, 3);
  Point2_i p3(2, 2);
  Point2_i p4(3, 4);

  EXPECT_FALSE(p1 != p1);
  EXPECT_TRUE(p1 != p2);
  EXPECT_TRUE(p1 != p3);
  EXPECT_TRUE(p1 != p4);
  EXPECT_NE(p1, p2);
  EXPECT_NE(p1, p3);
  EXPECT_NE(p1, p4);
}

TEST(Operator, Inequality1) {
  Point2_i p1(1, 2);
  Point2_i p2(2, 1);

  EXPECT_TRUE(p1 < p2);
  EXPECT_TRUE(p2 > p1);
  EXPECT_TRUE(p1 <= p2);
  EXPECT_TRUE(p2 >= p1);
}

TEST(Operator, Inequality2) {
  Point2_i p1(1, 2);
  Point2_i p2(1, 3);

  EXPECT_TRUE(p1 < p2);
  EXPECT_TRUE(p2 > p1);
  EXPECT_TRUE(p1 <= p2);
  EXPECT_TRUE(p2 >= p1);
}

TEST(Operator, Inequality3) {
  Point2_i p1(1, 2);
  Point2_i p2(2, 3);

  EXPECT_TRUE(p1 < p2);
  EXPECT_TRUE(p2 > p1);
  EXPECT_TRUE(p1 <= p2);
  EXPECT_TRUE(p2 >= p1);
}

TEST(Operator, Subscript) {
  Point2_i p(1, 2);

  EXPECT_EQ(p[0], 1);
  EXPECT_EQ(p[1], 2);
}

TEST(Operator, PointAdditionAssignment) {
  Point2_i p1(1, 2);
  Point2_i p2(3, 4);
  p1 += p2;

  EXPECT_EQ(p1.x(), 4);
  EXPECT_EQ(p1.y(), 6);
  EXPECT_EQ(p2.x(), 3);
  EXPECT_EQ(p2.y(), 4);
}

TEST(Operator, PointSubstractionAssignment) {
  Point2_i p1(1, 2);
  Point2_i p2(3, 4);
  p1 -= p2;

  EXPECT_EQ(p1.x(), -2);
  EXPECT_EQ(p1.y(), -2);
  EXPECT_EQ(p2.x(), 3);
  EXPECT_EQ(p2.y(), 4);
}

TEST(Operator, PointAddition) {
  Point2_i p1(1, 2);
  Point2_i p2(3, 4);
  Point2_i p3 = (p1 + p2);

  EXPECT_EQ(p1.x(), 1);
  EXPECT_EQ(p1.y(), 2);
  EXPECT_EQ(p2.x(), 3);
  EXPECT_EQ(p2.y(), 4);
  EXPECT_EQ(p3.x(), 4);
  EXPECT_EQ(p3.y(), 6);
}

TEST(Operator, PointSubstraction) {
  Point2_i p1(1, 2);
  Point2_i p2(3, 4);
  Point2_i p3 = (p1 - p2);

  EXPECT_EQ(p1.x(), 1);
  EXPECT_EQ(p1.y(), 2);
  EXPECT_EQ(p2.x(), 3);
  EXPECT_EQ(p2.y(), 4);
  EXPECT_EQ(p3.x(), -2);
  EXPECT_EQ(p3.y(), -2);
}

TEST(Operator, PointPositive) {
  Point2_i p1(1, 2);
  Point2_i p2 = +p1;

  EXPECT_EQ(p1.x(), 1);
  EXPECT_EQ(p1.y(), 2);
  EXPECT_EQ(p2.x(), 1);
  EXPECT_EQ(p2.y(), 2);
}

TEST(Operator, PointNegative) {
  Point2_i p1(1, 2);
  Point2_i p2 = -p1;

  EXPECT_EQ(p1.x(), 1);
  EXPECT_EQ(p1.y(), 2);
  EXPECT_EQ(p2.x(), -1);
  EXPECT_EQ(p2.y(), -2);
}

TEST(Operator, IntegerAdditionAssignment) {
  Point2_i p1(1, 2);
  p1 += 1;

  EXPECT_EQ(p1.x(), 2);
  EXPECT_EQ(p1.y(), 3);
}

TEST(Operator, IntegerSubstractionAssignment) {
  Point2_i p1(1, 2);
  p1 -= 1;

  EXPECT_EQ(p1.x(), 0);
  EXPECT_EQ(p1.y(), 1);
}

TEST(Operator, IntegerMultiplicationAssignment) {
  Point2_i p1(1, 2);
  p1 *= 2;

  EXPECT_EQ(p1.x(), 2);
  EXPECT_EQ(p1.y(), 4);
}

TEST(Operator, IntegerDivisionAssignment) {
  Point2_i p1(2, 4);
  p1 /= 2;

  EXPECT_EQ(p1.x(), 1);
  EXPECT_EQ(p1.y(), 2);
}

TEST(Operator, IntegerAddition) {
  Point2_i p1(1, 2);
  Point2_i p2 = p1 + 1;

  EXPECT_EQ(p1.x(), 1);
  EXPECT_EQ(p1.y(), 2);
  EXPECT_EQ(p2.x(), 2);
  EXPECT_EQ(p2.y(), 3);
}

TEST(Operator, IntegerSubstraction) {
  Point2_i p1(1, 2);
  Point2_i p2 = p1 - 1;

  EXPECT_EQ(p1.x(), 1);
  EXPECT_EQ(p1.y(), 2);
  EXPECT_EQ(p2.x(), 0);
  EXPECT_EQ(p2.y(), 1);
}

TEST(Operator, IntegerMultiplication) {
  Point2_i p1(1, 2);
  Point2_i p2 = p1 * 2;

  EXPECT_EQ(p1.x(), 1);
  EXPECT_EQ(p1.y(), 2);
  EXPECT_EQ(p2.x(), 2);
  EXPECT_EQ(p2.y(), 4);
}

TEST(Operator, IntegerDivision) {
  Point2_i p1(2, 4);
  Point2_i p2 = p1 / 2;

  EXPECT_EQ(p1.x(), 2);
  EXPECT_EQ(p1.y(), 4);
  EXPECT_EQ(p2.x(), 1);
  EXPECT_EQ(p2.y(), 2);
}

TEST(StringConversion, ToString) {
  Point2_i p1(1, 2);

  EXPECT_THAT(p1.ToString(), "(1, 2)");
}

TEST(StringConversion, AbslStringify) {
  Point2_i p1(1, 2);
  std::string s = absl::StrFormat("%v", p1);

  EXPECT_THAT(s, "(1, 2)");
}

}  // namespace moab