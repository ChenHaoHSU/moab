#include "point3.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

#include "absl/hash/hash_testing.h"
#include "absl/strings/str_format.h"

namespace moab {

using ::testing::Pair;
using ::testing::StrEq;

TEST(Constructor, Default) {
  Point3_i p;

  EXPECT_EQ(p.x(), 0);
  EXPECT_EQ(p.y(), 0);
  EXPECT_EQ(p.z(), 0);
}

TEST(Constructor, XYZ) {
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

TEST(AssignmentOperators, Copy) {
  Point3_i p1(1, 2, 3);
  Point3_i p2;

  p2 = p1;

  EXPECT_EQ(p2.x(), 1);
  EXPECT_EQ(p2.y(), 2);
  EXPECT_EQ(p2.z(), 3);
}

TEST(AssignmentOperators, Move) {
  Point3_i p1(1, 2, 3);
  Point3_i p2;

  p2 = std::move(p1);

  EXPECT_EQ(p2.x(), 1);
  EXPECT_EQ(p2.y(), 2);
  EXPECT_EQ(p2.z(), 3);
}

TEST(Accessors, Data) {
  Point3_i p(1, 2, 3);
  const int* d = p.data();

  ASSERT_NE(d, nullptr);
  EXPECT_EQ(d[0], 1);
  EXPECT_EQ(d[1], 2);
  EXPECT_EQ(d[2], 3);
}

TEST(Accessors, Size) {
  Point3_i p;

  EXPECT_EQ(p.Size(), 3);
}

TEST(Mutators, Set) {
  Point3_i p(1, 2, 3);
  p.Set(4, 5, 6);

  EXPECT_EQ(p.x(), 4);
  EXPECT_EQ(p.y(), 5);
  EXPECT_EQ(p.z(), 6);
}

TEST(Mutators, SetX) {
  Point3_i p(1, 2, 3);
  p.SetX(4);

  EXPECT_EQ(p.x(), 4);
  EXPECT_EQ(p.y(), 2);
  EXPECT_EQ(p.z(), 3);
}

TEST(Mutators, SetY) {
  Point3_i p(1, 2, 3);
  p.SetY(4);

  EXPECT_EQ(p.x(), 1);
  EXPECT_EQ(p.y(), 4);
  EXPECT_EQ(p.z(), 3);
}

TEST(Mutators, SetZ) {
  Point3_i p(1, 2, 3);
  p.SetZ(4);

  EXPECT_EQ(p.x(), 1);
  EXPECT_EQ(p.y(), 2);
  EXPECT_EQ(p.z(), 4);
}

TEST(Mutators, SetDim) {
  Point3_i p(1, 2, 3);
  p.SetDim(0, 4);
  p.SetDim(1, 5);
  p.SetDim(2, 6);

  EXPECT_EQ(p.x(), 4);
  EXPECT_EQ(p.y(), 5);
  EXPECT_EQ(p.z(), 6);
}

TEST(Operations, Shift) {
  Point3_i p(1, 2, 3);
  p.Shift(4, 5, 6);

  EXPECT_EQ(p.x(), 5);
  EXPECT_EQ(p.y(), 7);
  EXPECT_EQ(p.z(), 9);
}

TEST(Operations, ShiftX) {
  Point3_i p(1, 2, 3);
  p.ShiftX(3);

  EXPECT_EQ(p.x(), 4);
  EXPECT_EQ(p.y(), 2);
  EXPECT_EQ(p.z(), 3);
}

TEST(Operations, ShiftY) {
  Point3_i p(1, 2, 3);
  p.ShiftY(4);

  EXPECT_EQ(p.x(), 1);
  EXPECT_EQ(p.y(), 6);
  EXPECT_EQ(p.z(), 3);
}

TEST(Operations, ShiftZ) {
  Point3_i p(1, 2, 3);
  p.ShiftZ(5);

  EXPECT_EQ(p.x(), 1);
  EXPECT_EQ(p.y(), 2);
  EXPECT_EQ(p.z(), 8);
}

TEST(Operations, Rotate90XY) {
  Point3_i p(1, 2, 3);
  p.Rotate90(0, 1);

  EXPECT_EQ(p.x(), -2);
  EXPECT_EQ(p.y(), 1);
  EXPECT_EQ(p.z(), 3);
}

TEST(Operations, Rotate180XY) {
  Point3_i p(1, 2, 3);
  p.Rotate180(0, 1);

  EXPECT_EQ(p.x(), -1);
  EXPECT_EQ(p.y(), -2);
  EXPECT_EQ(p.z(), 3);
}

TEST(Operations, Rotate90YZ) {
  Point3_i p(1, 2, 3);
  p.Rotate90(1, 2);

  EXPECT_EQ(p.x(), 1);
  EXPECT_EQ(p.y(), -3);
  EXPECT_EQ(p.z(), 2);
}

TEST(Operations, Rotate180YZ) {
  Point3_i p(1, 2, 3);
  p.Rotate180(1, 2);

  EXPECT_EQ(p.x(), 1);
  EXPECT_EQ(p.y(), -2);
  EXPECT_EQ(p.z(), -3);
}

TEST(Operations, Rotate90ZX) {
  Point3_i p(1, 2, 3);
  p.Rotate90(2, 0);

  EXPECT_EQ(p.x(), 3);
  EXPECT_EQ(p.y(), 2);
  EXPECT_EQ(p.z(), -1);
}

TEST(Operations, Rotate180ZX) {
  Point3_i p(1, 2, 3);
  p.Rotate180(2, 0);

  EXPECT_EQ(p.x(), -1);
  EXPECT_EQ(p.y(), 2);
  EXPECT_EQ(p.z(), -3);
}

TEST(Operators, SubscriptAccess) {
  Point3_i p(1, 2, 3);

  EXPECT_EQ(p[0], 1);
  EXPECT_EQ(p[1], 2);
  EXPECT_EQ(p[2], 3);
}

TEST(Operators, SubscriptMutation) {
  Point3_i p(1, 2, 3);
  p[0] = 4;
  p[1] = 5;
  p[2] = 6;

  EXPECT_EQ(p[0], 4);
  EXPECT_EQ(p[1], 5);
  EXPECT_EQ(p[2], 6);
}

TEST(Operators, Equality) {
  Point3_i p1(1, 2, 3);
  Point3_i p2(1, 2, 3);
  Point3_i p3(1, 2, 4);

  EXPECT_TRUE(p1 == p2);
  EXPECT_FALSE(p1 == p3);
  EXPECT_EQ(p1, p2);
}

TEST(Operators, NotEquality) {
  Point3_i p1(1, 2, 3);
  Point3_i p2(1, 2, 3);
  Point3_i p3(1, 3, 4);
  Point3_i p4(2, 2, 4);
  Point3_i p5(3, 4, 5);

  EXPECT_FALSE(p1 != p2);
  EXPECT_TRUE(p1 != p3);
  EXPECT_TRUE(p1 != p4);
  EXPECT_TRUE(p1 != p5);
  EXPECT_NE(p1, p3);
  EXPECT_NE(p1, p4);
  EXPECT_NE(p1, p5);
}

TEST(Operators, Inequality1) {
  Point3_i p1(1, 2, 3);
  Point3_i p2(2, 1, 4);

  EXPECT_TRUE(p1 < p2);
  EXPECT_TRUE(p2 > p1);
  EXPECT_TRUE(p1 <= p2);
  EXPECT_TRUE(p2 >= p1);
}

TEST(Operators, Inequality2) {
  Point3_i p1(1, 2, 3);
  Point3_i p2(1, 3, 4);

  EXPECT_TRUE(p1 < p2);
  EXPECT_TRUE(p2 > p1);
  EXPECT_TRUE(p1 <= p2);
  EXPECT_TRUE(p2 >= p1);
}

TEST(Operators, Inequality3) {
  Point3_i p1(1, 2, 3);
  Point3_i p2(1, 2, 4);

  EXPECT_TRUE(p1 < p2);
  EXPECT_TRUE(p2 > p1);
  EXPECT_TRUE(p1 <= p2);
  EXPECT_TRUE(p2 >= p1);
}

TEST(Operators, Inequality4) {
  Point3_i p1(1, 2, 3);
  Point3_i p2(2, 3, 4);

  EXPECT_TRUE(p1 < p2);
  EXPECT_TRUE(p2 > p1);
  EXPECT_TRUE(p1 <= p2);
  EXPECT_TRUE(p2 >= p1);
}

TEST(Operators, PointAdditionAssignment) {
  Point3_i p1(1, 2, 3);
  Point3_i p2(4, 5, 6);
  p1 += p2;

  EXPECT_EQ(p1.x(), 5);
  EXPECT_EQ(p1.y(), 7);
  EXPECT_EQ(p1.z(), 9);
  EXPECT_EQ(p2.x(), 4);
  EXPECT_EQ(p2.y(), 5);
  EXPECT_EQ(p2.z(), 6);
}

TEST(Operators, PointSubstractionAssignment) {
  Point3_i p1(1, 2, 3);
  Point3_i p2(4, 5, 6);
  p1 -= p2;

  EXPECT_EQ(p1.x(), -3);
  EXPECT_EQ(p1.y(), -3);
  EXPECT_EQ(p1.z(), -3);
  EXPECT_EQ(p2.x(), 4);
  EXPECT_EQ(p2.y(), 5);
  EXPECT_EQ(p2.z(), 6);
}

TEST(Operators, PointAddition) {
  Point3_i p1(1, 2, 3);
  Point3_i p2(4, 5, 6);
  Point3_i p3 = (p1 + p2);

  EXPECT_EQ(p1.x(), 1);
  EXPECT_EQ(p1.y(), 2);
  EXPECT_EQ(p1.z(), 3);
  EXPECT_EQ(p2.x(), 4);
  EXPECT_EQ(p2.y(), 5);
  EXPECT_EQ(p2.z(), 6);
  EXPECT_EQ(p3.x(), 5);
  EXPECT_EQ(p3.y(), 7);
  EXPECT_EQ(p3.z(), 9);
}

TEST(Operators, PointSubstraction) {
  Point3_i p1(1, 2, 3);
  Point3_i p2(4, 5, 6);
  Point3_i p3 = (p1 - p2);

  EXPECT_EQ(p1.x(), 1);
  EXPECT_EQ(p1.y(), 2);
  EXPECT_EQ(p1.z(), 3);
  EXPECT_EQ(p2.x(), 4);
  EXPECT_EQ(p2.y(), 5);
  EXPECT_EQ(p2.z(), 6);
  EXPECT_EQ(p3.x(), -3);
  EXPECT_EQ(p3.y(), -3);
  EXPECT_EQ(p3.z(), -3);
}

TEST(Operators, PointPositive) {
  Point3_i p1(1, 2, 3);
  Point3_i p2 = +p1;

  EXPECT_EQ(p1.x(), 1);
  EXPECT_EQ(p1.y(), 2);
  EXPECT_EQ(p1.z(), 3);
  EXPECT_EQ(p2.x(), 1);
  EXPECT_EQ(p2.y(), 2);
  EXPECT_EQ(p2.z(), 3);
}

TEST(Operators, PointNegative) {
  Point3_i p1(1, 2, 3);
  Point3_i p2 = -p1;

  EXPECT_EQ(p1.x(), 1);
  EXPECT_EQ(p1.y(), 2);
  EXPECT_EQ(p1.z(), 3);
  EXPECT_EQ(p2.x(), -1);
  EXPECT_EQ(p2.y(), -2);
  EXPECT_EQ(p2.z(), -3);
}

TEST(Operators, IntegerAdditionAssignment) {
  Point3_i p1(1, 2, 3);
  p1 += 1;

  EXPECT_EQ(p1.x(), 2);
  EXPECT_EQ(p1.y(), 3);
  EXPECT_EQ(p1.z(), 4);
}

TEST(Operators, IntegerSubstractionAssignment) {
  Point3_i p1(1, 2, 3);
  p1 -= 1;

  EXPECT_EQ(p1.x(), 0);
  EXPECT_EQ(p1.y(), 1);
  EXPECT_EQ(p1.z(), 2);
}

TEST(Operators, IntegerMultiplicationAssignment) {
  Point3_i p1(1, 2, 3);
  p1 *= 2;

  EXPECT_EQ(p1.x(), 2);
  EXPECT_EQ(p1.y(), 4);
  EXPECT_EQ(p1.z(), 6);
}

TEST(Operators, IntegerDivisionAssignment) {
  Point3_i p1(2, 4, 6);
  p1 /= 2;

  EXPECT_EQ(p1.x(), 1);
  EXPECT_EQ(p1.y(), 2);
  EXPECT_EQ(p1.z(), 3);
}

TEST(Operators, IntegerAddition) {
  Point3_i p1(1, 2, 3);
  Point3_i p2 = p1 + 1;

  EXPECT_EQ(p1.x(), 1);
  EXPECT_EQ(p1.y(), 2);
  EXPECT_EQ(p1.z(), 3);
  EXPECT_EQ(p2.x(), 2);
  EXPECT_EQ(p2.y(), 3);
  EXPECT_EQ(p2.z(), 4);
}

TEST(Operators, IntegerSubstraction) {
  Point3_i p1(1, 2, 3);
  Point3_i p2 = p1 - 1;

  EXPECT_EQ(p1.x(), 1);
  EXPECT_EQ(p1.y(), 2);
  EXPECT_EQ(p1.z(), 3);
  EXPECT_EQ(p2.x(), 0);
  EXPECT_EQ(p2.y(), 1);
  EXPECT_EQ(p2.z(), 2);
}

TEST(Operators, IntegerMultiplication) {
  Point3_i p1(1, 2, 3);
  Point3_i p2 = p1 * 2;

  EXPECT_EQ(p1.x(), 1);
  EXPECT_EQ(p1.y(), 2);
  EXPECT_EQ(p1.z(), 3);
  EXPECT_EQ(p2.x(), 2);
  EXPECT_EQ(p2.y(), 4);
  EXPECT_EQ(p2.z(), 6);
}

TEST(Operators, IntegerDivision) {
  Point3_i p1(2, 4, 6);
  Point3_i p2 = p1 / 2;

  EXPECT_EQ(p1.x(), 2);
  EXPECT_EQ(p1.y(), 4);
  EXPECT_EQ(p1.z(), 6);
  EXPECT_EQ(p2.x(), 1);
  EXPECT_EQ(p2.y(), 2);
  EXPECT_EQ(p2.z(), 3);
}

TEST(StringConversion, ToString) {
  Point3_i p(1, 2, 3);

  EXPECT_THAT(p.ToString(), StrEq("(1 2 3)"));
}

TEST(StringConversion, SupportsAbslStringify) {
  Point3_i p(1, 2, 3);
  std::string s = absl::StrFormat("%v", p);

  EXPECT_THAT(s, StrEq("(1 2 3)"));
}

TEST(Hash, SupportsAbslHash) {
  EXPECT_TRUE(absl::VerifyTypeImplementsAbslHashCorrectly({
      Point3_i(),
      Point3_i(1, 2, 3),
      Point3_i(0, 0, 0),
      Point3_i(2, 3, 4),
      Point3_i(0, -1, -2),
  }));
}

}  // namespace moab