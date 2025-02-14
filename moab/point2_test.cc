#include "point2.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

#include "absl/hash/hash_testing.h"
#include "absl/strings/str_format.h"
#include "moab/point2.pb.h"

namespace moab {

using ::testing::Pair;
using ::testing::StrEq;

TEST(Constructors, Default) {
  Point2_i p;

  EXPECT_EQ(p.x(), 0);
  EXPECT_EQ(p.y(), 0);
}

TEST(Constructors, XY) {
  Point2_i p(1, 2);

  EXPECT_EQ(p.x(), 1);
  EXPECT_EQ(p.y(), 2);
}

TEST(Constructors, Proto) {
  Point2Proto proto;
  proto.mutable_point_int32()->set_x(1);
  proto.mutable_point_int32()->set_y(2);

  Point2_i p(proto);

  EXPECT_EQ(p.x(), 1);
  EXPECT_EQ(p.y(), 2);
}

TEST(Constructors, Copy) {
  Point2_i p1(1, 2);
  Point2_i p2(p1);

  EXPECT_EQ(p2.x(), 1);
  EXPECT_EQ(p2.y(), 2);
}

TEST(Constructors, Move) {
  Point2_i p(Point2_i(1, 2));

  EXPECT_EQ(p.x(), 1);
  EXPECT_EQ(p.y(), 2);
}

TEST(AssignmentOperators, Copy) {
  Point2_i p1(1, 2);
  Point2_i p2;

  p2 = p1;

  EXPECT_EQ(p2.x(), 1);
  EXPECT_EQ(p2.y(), 2);
}

TEST(AssignmentOperators, Move) {
  Point2_i p1(1, 2);
  Point2_i p2;

  p2 = std::move(p1);

  EXPECT_EQ(p2.x(), 1);
  EXPECT_EQ(p2.y(), 2);
}

TEST(Accessors, XY) {
  Point2_i p(1, 2);

  EXPECT_EQ(p.x(), 1);
  EXPECT_EQ(p.y(), 2);
}

TEST(Accessors, Data) {
  Point2_i p(1, 2);
  const int* d = p.data();

  ASSERT_NE(d, nullptr);
  EXPECT_EQ(d[0], 1);
  EXPECT_EQ(d[1], 2);
}

TEST(Accessors, Size) {
  Point2_i p;

  EXPECT_EQ(p.Size(), 2);
}

TEST(Accessors, ToPair) {
  Point2_i p(1, 2);
  std::pair<int, int> p_pair = p.ToPair();

  EXPECT_THAT(p_pair, Pair(1, 2));
}

TEST(Mutators, Set) {
  Point2_i p(1, 2);
  p.Set(3, 4);

  EXPECT_EQ(p.x(), 3);
  EXPECT_EQ(p.y(), 4);
}

TEST(Mutators, SetX) {
  Point2_i p(1, 2);
  p.SetX(3);

  EXPECT_EQ(p.x(), 3);
  EXPECT_EQ(p.y(), 2);
}

TEST(Mutators, SetY) {
  Point2_i p(1, 2);
  p.SetY(4);

  EXPECT_EQ(p.x(), 1);
  EXPECT_EQ(p.y(), 4);
}

TEST(Mutators, SetDim) {
  Point2_i p(1, 2);
  p.SetDim(0, 3);
  p.SetDim(1, 4);

  EXPECT_EQ(p.x(), 3);
  EXPECT_EQ(p.y(), 4);
}

TEST(Operations, Shift) {
  Point2_i p(1, 2);
  p.Shift(3, 4);

  EXPECT_EQ(p.x(), 4);
  EXPECT_EQ(p.y(), 6);
}

TEST(Operations, ShiftX) {
  Point2_i p(1, 2);
  p.ShiftX(3);

  EXPECT_EQ(p.x(), 4);
  EXPECT_EQ(p.y(), 2);
}

TEST(Operations, ShiftY) {
  Point2_i p(1, 2);
  p.ShiftY(4);

  EXPECT_EQ(p.x(), 1);
  EXPECT_EQ(p.y(), 6);
}

TEST(Operations, Rotate90) {
  Point2_i p(1, 2);
  p.Rotate90();

  EXPECT_EQ(p.x(), -2);
  EXPECT_EQ(p.y(), 1);
}

TEST(Operations, Rotate180) {
  Point2_i p(1, 2);
  p.Rotate180();

  EXPECT_EQ(p.x(), -1);
  EXPECT_EQ(p.y(), -2);
}

TEST(Operators, SubscriptAccess) {
  Point2_i p(1, 2);

  EXPECT_EQ(p[0], 1);
  EXPECT_EQ(p[1], 2);
}

TEST(Operators, SubscriptMutation) {
  Point2_i p(1, 2);
  p[0] = 3;
  p[1] = 4;

  EXPECT_EQ(p[0], 3);
  EXPECT_EQ(p[1], 4);
}

TEST(Operators, Equality) {
  Point2_i p1(1, 2);
  Point2_i p2(1, 2);
  Point2_i p3(1, 3);

  EXPECT_TRUE(p1 == p2);
  EXPECT_FALSE(p1 == p3);
  EXPECT_EQ(p1, p2);
}

TEST(Operators, NotEquality) {
  Point2_i p1(1, 2);
  Point2_i p2(1, 2);
  Point2_i p3(1, 3);
  Point2_i p4(2, 2);
  Point2_i p5(3, 4);

  EXPECT_FALSE(p1 != p2);
  EXPECT_TRUE(p1 != p3);
  EXPECT_TRUE(p1 != p4);
  EXPECT_TRUE(p1 != p5);
  EXPECT_NE(p1, p3);
  EXPECT_NE(p1, p4);
  EXPECT_NE(p1, p5);
}

TEST(Operators, Inequality1) {
  Point2_i p1(1, 2);
  Point2_i p2(2, 1);

  EXPECT_TRUE(p1 < p2);
  EXPECT_TRUE(p2 > p1);
  EXPECT_TRUE(p1 <= p2);
  EXPECT_TRUE(p2 >= p1);
}

TEST(Operators, Inequality2) {
  Point2_i p1(1, 2);
  Point2_i p2(1, 3);

  EXPECT_TRUE(p1 < p2);
  EXPECT_TRUE(p2 > p1);
  EXPECT_TRUE(p1 <= p2);
  EXPECT_TRUE(p2 >= p1);
}

TEST(Operators, Inequality3) {
  Point2_i p1(1, 2);
  Point2_i p2(2, 3);

  EXPECT_TRUE(p1 < p2);
  EXPECT_TRUE(p2 > p1);
  EXPECT_TRUE(p1 <= p2);
  EXPECT_TRUE(p2 >= p1);
}

TEST(Operators, PointAdditionAssignment) {
  Point2_i p1(1, 2);
  Point2_i p2(3, 4);
  p1 += p2;

  EXPECT_EQ(p1.x(), 4);
  EXPECT_EQ(p1.y(), 6);
  EXPECT_EQ(p2.x(), 3);
  EXPECT_EQ(p2.y(), 4);
}

TEST(Operators, PointSubstractionAssignment) {
  Point2_i p1(1, 2);
  Point2_i p2(3, 4);
  p1 -= p2;

  EXPECT_EQ(p1.x(), -2);
  EXPECT_EQ(p1.y(), -2);
  EXPECT_EQ(p2.x(), 3);
  EXPECT_EQ(p2.y(), 4);
}

TEST(Operators, PointAddition) {
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

TEST(Operators, PointSubstraction) {
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

TEST(Operators, PointPositive) {
  Point2_i p1(1, 2);
  Point2_i p2 = +p1;

  EXPECT_EQ(p1.x(), 1);
  EXPECT_EQ(p1.y(), 2);
  EXPECT_EQ(p2.x(), 1);
  EXPECT_EQ(p2.y(), 2);
}

TEST(Operators, PointNegative) {
  Point2_i p1(1, 2);
  Point2_i p2 = -p1;

  EXPECT_EQ(p1.x(), 1);
  EXPECT_EQ(p1.y(), 2);
  EXPECT_EQ(p2.x(), -1);
  EXPECT_EQ(p2.y(), -2);
}

TEST(Operators, IntegerAdditionAssignment) {
  Point2_i p1(1, 2);
  p1 += 1;

  EXPECT_EQ(p1.x(), 2);
  EXPECT_EQ(p1.y(), 3);
}

TEST(Operators, IntegerSubstractionAssignment) {
  Point2_i p1(1, 2);
  p1 -= 1;

  EXPECT_EQ(p1.x(), 0);
  EXPECT_EQ(p1.y(), 1);
}

TEST(Operators, IntegerMultiplicationAssignment) {
  Point2_i p1(1, 2);
  p1 *= 2;

  EXPECT_EQ(p1.x(), 2);
  EXPECT_EQ(p1.y(), 4);
}

TEST(Operators, IntegerDivisionAssignment) {
  Point2_i p1(2, 4);
  p1 /= 2;

  EXPECT_EQ(p1.x(), 1);
  EXPECT_EQ(p1.y(), 2);
}

TEST(Operators, IntegerAddition) {
  Point2_i p1(1, 2);
  Point2_i p2 = p1 + 1;

  EXPECT_EQ(p1.x(), 1);
  EXPECT_EQ(p1.y(), 2);
  EXPECT_EQ(p2.x(), 2);
  EXPECT_EQ(p2.y(), 3);
}

TEST(Operators, IntegerSubstraction) {
  Point2_i p1(1, 2);
  Point2_i p2 = p1 - 1;

  EXPECT_EQ(p1.x(), 1);
  EXPECT_EQ(p1.y(), 2);
  EXPECT_EQ(p2.x(), 0);
  EXPECT_EQ(p2.y(), 1);
}

TEST(Operators, IntegerMultiplication) {
  Point2_i p1(1, 2);
  Point2_i p2 = p1 * 2;

  EXPECT_EQ(p1.x(), 1);
  EXPECT_EQ(p1.y(), 2);
  EXPECT_EQ(p2.x(), 2);
  EXPECT_EQ(p2.y(), 4);
}

TEST(Operators, IntegerDivision) {
  Point2_i p1(2, 4);
  Point2_i p2 = p1 / 2;

  EXPECT_EQ(p1.x(), 2);
  EXPECT_EQ(p1.y(), 4);
  EXPECT_EQ(p2.x(), 1);
  EXPECT_EQ(p2.y(), 2);
}

TEST(Distance, Distance) {
  Point2_i p1(0, 0);
  Point2_i p2(10, 0);
  Point2_i p3(-1, -2);

  EXPECT_EQ(p1.Distance(p1), 0);
  EXPECT_EQ(p1.Distance(p2), 10);
  EXPECT_EQ(p1.Distance(p3), 3);
  EXPECT_EQ(p2.Distance(p1), 10);
  EXPECT_EQ(p2.Distance(p2), 0);
  EXPECT_EQ(p2.Distance(p3), 13);
  EXPECT_EQ(p3.Distance(p1), 3);
  EXPECT_EQ(p3.Distance(p2), 13);
  EXPECT_EQ(p3.Distance(p3), 0);
}

TEST(StringConversion, ToString) {
  Point2_i p(1, 2);

  EXPECT_THAT(p.ToString(), StrEq("(1 2)"));
}

TEST(StringConversion, SupportsAbslStringify) {
  Point2_i p(1, 2);
  std::string s = absl::StrFormat("%v", p);

  EXPECT_THAT(s, StrEq("(1 2)"));
}

TEST(Hash, SupportsAbslHash) {
  EXPECT_TRUE(absl::VerifyTypeImplementsAbslHashCorrectly({
      Point2_i(),
      Point2_i(1, 2),
      Point2_i(0, 0),
      Point2_i(2, 3),
      Point2_i(0, -1),
  }));
}

TEST(Protobuf, ToProto) {
  Point2_i p(1, 2);
  Point2Proto proto = p.ToProto();

  EXPECT_TRUE(proto.has_point_int32());
  EXPECT_EQ(proto.point_int32().x(), 1);
  EXPECT_EQ(proto.point_int32().y(), 2);
}

TEST(Protobuf, SetFromProto) {
  Point2Proto proto;
  proto.mutable_point_int32()->set_x(1);
  proto.mutable_point_int32()->set_y(2);

  Point2_i p;
  p.SetFromProto(proto);

  EXPECT_EQ(p.x(), 1);
  EXPECT_EQ(p.y(), 2);
}

}  // namespace moab