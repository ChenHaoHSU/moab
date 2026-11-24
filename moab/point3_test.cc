#include "point3.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <string>

#include "absl/hash/hash_testing.h"
#include "absl/status/status.h"
#include "absl/status/status_matchers.h"
#include "absl/strings/str_format.h"
#include "moab/point2.h"
#include "moab/point3.pb.h"

namespace moab {

using ::testing::FieldsAre;
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

TEST(Constructor, Proto) {
  Point3Proto proto;
  proto.mutable_point_int32()->set_x(1);
  proto.mutable_point_int32()->set_y(2);
  proto.mutable_point_int32()->set_z(3);

  Point3_i p(proto);

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

TEST(Accessors, To2D) {
  Point3_i p(1, 2, 3);

  EXPECT_EQ(p.To2D(), Point2_i(1, 2));
}

TEST(Accessors, ToTuple) {
  Point3_i p(1, 2, 3);

  EXPECT_THAT(p.ToTuple(), FieldsAre(1, 2, 3));
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

TEST(Distance, Distance) {
  Point3_i p1(0, 0, 0);
  Point3_i p2(1, 2, 3);
  Point3_i p3(10, 20, 30);

  EXPECT_EQ(p1.Distance(p1), 0);
  EXPECT_EQ(p1.Distance(p2), 6);
  EXPECT_EQ(p1.Distance(p3), 60);
  EXPECT_EQ(p2.Distance(p1), 6);
  EXPECT_EQ(p2.Distance(p2), 0);
  EXPECT_EQ(p2.Distance(p3), 54);
  EXPECT_EQ(p3.Distance(p1), 60);
  EXPECT_EQ(p3.Distance(p2), 54);
  EXPECT_EQ(p3.Distance(p3), 0);
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

TEST(Protobuf, ToProto) {
  Point3_i p(1, 2, 3);
  Point3Proto proto = p.ToProto();

  EXPECT_TRUE(proto.has_point_int32());
  EXPECT_EQ(proto.point_int32().x(), 1);
  EXPECT_EQ(proto.point_int32().y(), 2);
  EXPECT_EQ(proto.point_int32().z(), 3);
}

TEST(Protobuf, SetFromProto) {
  Point3Proto proto;
  proto.mutable_point_int32()->set_x(1);
  proto.mutable_point_int32()->set_y(2);
  proto.mutable_point_int32()->set_z(3);

  Point3_i p;
  ASSERT_THAT(p.SetFromProto(proto), ::absl_testing::IsOk());

  EXPECT_EQ(p.x(), 1);
  EXPECT_EQ(p.y(), 2);
  EXPECT_EQ(p.z(), 3);
}

TEST(Operators, ArithmeticPointConst) {
  const Point3_i a(1, 2, 3);
  const Point3_i b(4, 6, 8);

  EXPECT_EQ(a + b, Point3_i(5, 8, 11));
  EXPECT_EQ(b - a, Point3_i(3, 4, 5));
}

TEST(StringConversion, ToStringFloatingPoint) {
  EXPECT_THAT(Point3<double>(1.5, -2.25, 0).ToString(), StrEq("(1.5 -2.25 0)"));
  EXPECT_THAT(Point3<float>(0.5f, 3.0f, 0.75f).ToString(),
              StrEq("(0.5 3 0.75)"));
}

TEST(StringConversion, ToStringInt64) {
  EXPECT_THAT(Point3_i64(int64_t{1} << 40, -1, 2).ToString(),
              StrEq("(1099511627776 -1 2)"));
}

using ::absl_testing::IsOkAndHolds;
using ::absl_testing::StatusIs;
using ::testing::HasSubstr;

TEST(Protobuf, FromProtoRoundTrip) {
  EXPECT_THAT(Point3_i::FromProto(Point3_i(1, -2, 3).ToProto()),
              IsOkAndHolds(Point3_i(1, -2, 3)));
  EXPECT_THAT(
      Point3_i64::FromProto(Point3_i64(int64_t{1} << 40, 5, -6).ToProto()),
      IsOkAndHolds(Point3_i64(int64_t{1} << 40, 5, -6)));
  EXPECT_THAT(Point3<double>::FromProto(Point3<double>(0.1, 2.5, -3).ToProto()),
              IsOkAndHolds(Point3<double>(0.1, 2.5, -3)));
}

TEST(Protobuf, FromProtoNotSet) {
  EXPECT_THAT(Point3_i::FromProto(Point3Proto()),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("[Point3] The proto has no coordinate type "
                                 "set.")));
}

TEST(Protobuf, FromProtoOutOfRange) {
  Point3Proto proto;
  proto.mutable_point_float()->set_x(0);
  proto.mutable_point_float()->set_y(0);
  proto.mutable_point_float()->set_z(0.5f);

  EXPECT_THAT(Point3_i::FromProto(proto),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("[Point3] z: coordinate 0.5 is not a whole "
                                 "number")));
}

TEST(Protobuf, SetFromProtoNotSetLeavesValueUnchanged) {
  Point3_i v = Point3_i(7, 8, 9);

  EXPECT_THAT(v.SetFromProto(Point3Proto()),
              StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_EQ(v, Point3_i(7, 8, 9));
}

}  // namespace moab