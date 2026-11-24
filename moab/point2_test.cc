#include "point2.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cmath>
#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>

#include "absl/hash/hash_testing.h"
#include "absl/status/status.h"
#include "absl/status/status_matchers.h"
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

// data() is read-only, even on a non-const point.
TEST(Accessors, Data) {
  static_assert(
      std::is_same_v<decltype(std::declval<Point2_i&>().data()), const int*>);
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
  ASSERT_THAT(p.SetFromProto(proto), ::absl_testing::IsOk());

  EXPECT_EQ(p.x(), 1);
  EXPECT_EQ(p.y(), 2);
}

TEST(Operators, ArithmeticPointConst) {
  const Point2_i a(1, 2);
  const Point2_i b(3, 5);

  EXPECT_EQ(a + b, Point2_i(4, 7));
  EXPECT_EQ(b - a, Point2_i(2, 3));
}

TEST(StringConversion, ToStringFloatingPoint) {
  EXPECT_THAT(Point2<double>(1.5, -2.25).ToString(), StrEq("(1.5 -2.25)"));
  EXPECT_THAT(Point2<float>(0.5f, 3.0f).ToString(), StrEq("(0.5 3)"));
  EXPECT_THAT(absl::StrFormat("%v", Point2<double>(0.125, 4)),
              StrEq("(0.125 4)"));
}

TEST(StringConversion, ToStringInt64) {
  EXPECT_THAT(Point2_i64(int64_t{1} << 40, -1).ToString(),
              StrEq("(1099511627776 -1)"));
}

using ::absl_testing::IsOkAndHolds;
using ::absl_testing::StatusIs;
using ::testing::HasSubstr;

TEST(Protobuf, FromProtoInt32) {
  Point2Proto proto;
  proto.mutable_point_int32()->set_x(1);
  proto.mutable_point_int32()->set_y(-2);

  EXPECT_THAT(Point2_i::FromProto(proto), IsOkAndHolds(Point2_i(1, -2)));
}

TEST(Protobuf, FromProtoRoundTrip) {
  EXPECT_THAT(Point2_i::FromProto(Point2_i(3, -4).ToProto()),
              IsOkAndHolds(Point2_i(3, -4)));
  EXPECT_THAT(Point2_i64::FromProto(Point2_i64(int64_t{1} << 40, 5).ToProto()),
              IsOkAndHolds(Point2_i64(int64_t{1} << 40, 5)));
  EXPECT_THAT(Point2<float>::FromProto(Point2<float>(0.5f, -1.25f).ToProto()),
              IsOkAndHolds(Point2<float>(0.5f, -1.25f)));
  EXPECT_THAT(Point2<double>::FromProto(Point2<double>(0.1, 2.5).ToProto()),
              IsOkAndHolds(Point2<double>(0.1, 2.5)));
}

TEST(Protobuf, FromProtoConvertsType) {
  Point2Proto proto;
  proto.mutable_point_double()->set_x(2.0);
  proto.mutable_point_double()->set_y(-3.0);

  EXPECT_THAT(Point2_i::FromProto(proto), IsOkAndHolds(Point2_i(2, -3)));
  EXPECT_THAT(Point2<double>::FromProto(Point2_i(1, 2).ToProto()),
              IsOkAndHolds(Point2<double>(1, 2)));
}

TEST(Protobuf, FromProtoNotSet) {
  EXPECT_THAT(Point2_i::FromProto(Point2Proto()),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("[Point2] The proto has no coordinate type "
                                 "set.")));
}

TEST(Protobuf, FromProtoOutOfRange) {
  Point2Proto proto;
  proto.mutable_point_int64()->set_x(0);
  proto.mutable_point_int64()->set_y(int64_t{1} << 40);

  EXPECT_THAT(Point2_i::FromProto(proto),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("[Point2] y: coordinate 1099511627776 does "
                                 "not fit in int32.")));
}

TEST(Protobuf, FromProtoNotWholeNumber) {
  Point2Proto proto;
  proto.mutable_point_double()->set_x(1.5);
  proto.mutable_point_double()->set_y(0);

  EXPECT_THAT(Point2_i::FromProto(proto),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("[Point2] x: coordinate 1.5 is not a whole "
                                 "number")));
}

TEST(Protobuf, SetFromProtoNotSetLeavesValueUnchanged) {
  Point2_i v = Point2_i(7, 8);

  EXPECT_THAT(v.SetFromProto(Point2Proto()),
              StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_EQ(v, Point2_i(7, 8));
}

// operator[] checks the index with DCHECK, so only debug builds abort.
#ifndef NDEBUG
TEST(OperatorsDeathTest, SubscriptOutOfRange) {
  Point2_i v(1, 2);
  const Point2_i& const_v = v;

  EXPECT_DEATH((void)v[2], "Check failed: i < d_.size");
  EXPECT_DEATH((void)const_v[2], "Check failed: i < d_.size");
}
#endif

TEST(Distance, ManhattanDistance) {
  const Point2_i p(1, 2);
  const Point2_i q(4, -2);

  EXPECT_EQ(p.ManhattanDistance(q), 7);
  EXPECT_EQ(p.Distance(q), p.ManhattanDistance(q));
}

TEST(Distance, SquaredEuclideanDistance) {
  static_assert(std::is_same_v<Point2_i::area_type, int64_t>);
  static_assert(std::is_same_v<Point2<double>::area_type, double>);

  EXPECT_EQ(Point2_i(1, 2).SquaredEuclideanDistance(Point2_i(4, -2)), 25);
  EXPECT_EQ(Point2_i(4, -2).SquaredEuclideanDistance(Point2_i(1, 2)), 25);
  EXPECT_EQ(Point2_i(3, 3).SquaredEuclideanDistance(Point2_i(3, 3)), 0);
}

TEST(Distance, SquaredEuclideanDistanceNoOverflow) {
  // dx = 2e9 does not fit in an int, and dx^2 = 4e18 needs int64_t.
  EXPECT_EQ(Point2_i(-1000000000, 0)
                .SquaredEuclideanDistance(Point2_i(1000000000, 0)),
            int64_t{4000000000000000000});
}

TEST(Distance, EuclideanDistance) {
  EXPECT_DOUBLE_EQ(Point2_i(1, 2).EuclideanDistance(Point2_i(4, -2)), 5.0);
  EXPECT_DOUBLE_EQ(Point2<double>(0, 0).EuclideanDistance(Point2<double>(1, 1)),
                   std::sqrt(2.0));
}

}  // namespace moab