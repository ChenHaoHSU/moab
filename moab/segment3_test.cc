#include "segment3.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <type_traits>
#include <utility>

#include "absl/hash/hash_testing.h"
#include "absl/status/status.h"
#include "absl/status/status_matchers.h"
#include "absl/strings/str_format.h"
#include "moab/point3.h"
#include "moab/segment3.pb.h"

namespace moab {

using ::testing::Pair;
using ::testing::StrEq;

TEST(Constructors, Default) {
  Segment3_i s;

  EXPECT_EQ(s.p0(), Point3_i(0, 0, 0));
  EXPECT_EQ(s.p1(), Point3_i(0, 0, 0));
}

TEST(Constructors, TwoPoints) {
  Segment3_i s(Point3_i(1, 2, 3), Point3_i(4, 5, 6));

  EXPECT_EQ(s.p0(), Point3_i(1, 2, 3));
  EXPECT_EQ(s.p1(), Point3_i(4, 5, 6));
}

TEST(Constructors, MinMaxXY) {
  Segment3_i s(1, 2, 3, 4, 5, 6);

  EXPECT_EQ(s.p0(), Point3_i(1, 2, 3));
  EXPECT_EQ(s.p1(), Point3_i(4, 5, 6));
}

TEST(Constructors, Proto) {
  Segment3Proto proto;
  proto.mutable_segment_int32()->mutable_p0()->set_x(1);
  proto.mutable_segment_int32()->mutable_p0()->set_y(2);
  proto.mutable_segment_int32()->mutable_p0()->set_z(3);
  proto.mutable_segment_int32()->mutable_p1()->set_x(4);
  proto.mutable_segment_int32()->mutable_p1()->set_y(5);
  proto.mutable_segment_int32()->mutable_p1()->set_z(6);

  Segment3_i s(proto);

  EXPECT_EQ(s.p0(), Point3_i(1, 2, 3));
  EXPECT_EQ(s.p1(), Point3_i(4, 5, 6));
}

TEST(Constructors, Copy) {
  Segment3_i s1(Point3_i(1, 2, 3), Point3_i(4, 5, 6));
  Segment3_i s2(s1);

  EXPECT_EQ(s2.p0(), Point3_i(1, 2, 3));
  EXPECT_EQ(s2.p1(), Point3_i(4, 5, 6));
}

TEST(Constructors, Move) {
  Segment3_i s1(Point3_i(1, 2, 3), Point3_i(4, 5, 6));
  Segment3_i s2(std::move(s1));

  EXPECT_EQ(s2.p0(), Point3_i(1, 2, 3));
  EXPECT_EQ(s2.p1(), Point3_i(4, 5, 6));
}

TEST(AssignmentOperators, Copy) {
  Segment3_i s1(Point3_i(1, 2, 3), Point3_i(4, 5, 6));
  Segment3_i s2;

  s2 = s1;

  EXPECT_EQ(s2.p0(), Point3_i(1, 2, 3));
  EXPECT_EQ(s2.p1(), Point3_i(4, 5, 6));
}

TEST(AssignmentOperators, Move) {
  Segment3_i s1(Point3_i(1, 2, 3), Point3_i(4, 5, 6));
  Segment3_i s2;

  s2 = std::move(s1);

  EXPECT_EQ(s2.p0(), Point3_i(1, 2, 3));
  EXPECT_EQ(s2.p1(), Point3_i(4, 5, 6));
}

TEST(Accessors, P0P1) {
  Segment3_i s(Point3_i(1, 2, 3), Point3_i(4, 5, 6));

  EXPECT_EQ(s.p0(), Point3_i(1, 2, 3));
  EXPECT_EQ(s.p1(), Point3_i(4, 5, 6));
}

TEST(Accessors, P0P1Const) {
  const Segment3_i s(Point3_i(1, 2, 3), Point3_i(4, 5, 6));

  EXPECT_EQ(s.p0(), Point3_i(1, 2, 3));
  EXPECT_EQ(s.p1(), Point3_i(4, 5, 6));
}

// data() is read-only, even on a non-const segment.
TEST(Accessors, Data) {
  static_assert(std::is_same_v<decltype(std::declval<Segment3_i&>().data()),
                               const Point3_i*>);
  Segment3_i s(Point3_i(1, 2, 3), Point3_i(4, 5, 6));
  const Point3_i* data = s.data();

  EXPECT_EQ(data[0], Point3_i(1, 2, 3));
  EXPECT_EQ(data[1], Point3_i(4, 5, 6));
}

TEST(Accessors, DataConst) {
  const Segment3_i s(Point3_i(1, 2, 3), Point3_i(4, 5, 6));
  const Point3_i* data = s.data();

  EXPECT_EQ(data[0], Point3_i(1, 2, 3));
  EXPECT_EQ(data[1], Point3_i(4, 5, 6));
}

TEST(Accessors, Size) {
  Segment3_i s(Point3_i(1, 2, 3), Point3_i(4, 5, 6));

  EXPECT_EQ(s.Size(), 2);
}

TEST(Accessors, ToPair) {
  Segment3_i s(Point3_i(1, 2, 3), Point3_i(4, 5, 6));
  std::pair<Point3_i, Point3_i> p = s.ToPair();

  EXPECT_THAT(p, Pair(Point3_i(1, 2, 3), Point3_i(4, 5, 6)));
}

TEST(Accessors, Length) {
  Segment3_i s(Point3_i(1, 2, 3), Point3_i(4, 5, 6));

  EXPECT_EQ(s.Length(), 9);
}

TEST(Accessors, XLYLZLXHYHZH1) {
  Segment3_i s(Point3_i(1, 2, 3), Point3_i(4, 5, 6));

  EXPECT_EQ(s.xl(), 1);
  EXPECT_EQ(s.yl(), 2);
  EXPECT_EQ(s.zl(), 3);
  EXPECT_EQ(s.xh(), 4);
  EXPECT_EQ(s.yh(), 5);
  EXPECT_EQ(s.zh(), 6);
}

TEST(Accessors, XLYLZLXHYHZH2) {
  Segment3_i s(Point3_i(4, 5, 6), Point3_i(1, 2, 3));

  EXPECT_EQ(s.xl(), 1);
  EXPECT_EQ(s.yl(), 2);
  EXPECT_EQ(s.zl(), 3);
  EXPECT_EQ(s.xh(), 4);
  EXPECT_EQ(s.yh(), 5);
  EXPECT_EQ(s.zh(), 6);
}

TEST(Accessors, MinMaxXYZ1) {
  Segment3_i s(Point3_i(1, 2, 3), Point3_i(4, 5, 6));

  EXPECT_EQ(s.MinX(), 1);
  EXPECT_EQ(s.MinY(), 2);
  EXPECT_EQ(s.MinZ(), 3);
  EXPECT_EQ(s.MaxX(), 4);
  EXPECT_EQ(s.MaxY(), 5);
  EXPECT_EQ(s.MaxZ(), 6);
}

TEST(Accessors, MinMaxXYZ2) {
  Segment3_i s(Point3_i(4, 5, 6), Point3_i(1, 2, 3));

  EXPECT_EQ(s.MinX(), 1);
  EXPECT_EQ(s.MinY(), 2);
  EXPECT_EQ(s.MinZ(), 3);
  EXPECT_EQ(s.MaxX(), 4);
  EXPECT_EQ(s.MaxY(), 5);
  EXPECT_EQ(s.MaxZ(), 6);
}

TEST(Mutators, SetByFourCoordinates) {
  Segment3_i s(Point3_i(1, 2, 3), Point3_i(4, 5, 6));
  s.Set(7, 8, 9, 10, 11, 12);

  EXPECT_EQ(s.p0(), Point3_i(7, 8, 9));
  EXPECT_EQ(s.p1(), Point3_i(10, 11, 12));
}

TEST(Mutators, SetByTwoPoints) {
  Segment3_i s(Point3_i(1, 2, 3), Point3_i(4, 5, 6));
  s.Set(Point3_i(7, 8, 9), Point3_i(10, 11, 12));

  EXPECT_EQ(s.p0(), Point3_i(7, 8, 9));
  EXPECT_EQ(s.p1(), Point3_i(10, 11, 12));
}

TEST(Mutators, SetP0P1) {
  Segment3_i s(Point3_i(1, 2, 3), Point3_i(4, 5, 6));
  s.SetP0(Point3_i(7, 8, 9));
  s.SetP1(Point3_i(10, 11, 12));

  EXPECT_EQ(s.p0(), Point3_i(7, 8, 9));
  EXPECT_EQ(s.p1(), Point3_i(10, 11, 12));
}

TEST(Mutators, SetP) {
  Segment3_i s(Point3_i(1, 2, 3), Point3_i(4, 5, 6));
  s.SetP(0, Point3_i(7, 8, 9));
  s.SetP(1, Point3_i(10, 11, 12));

  EXPECT_EQ(s.p0(), Point3_i(7, 8, 9));
  EXPECT_EQ(s.p1(), Point3_i(10, 11, 12));
}

TEST(Operations, Shift) {
  Segment3_i s(Point3_i(1, 2, 3), Point3_i(4, 5, 6));
  s.Shift(1, 2, 3);

  EXPECT_EQ(s.p0(), Point3_i(2, 4, 6));
  EXPECT_EQ(s.p1(), Point3_i(5, 7, 9));
}

TEST(Operations, ShiftX) {
  Segment3_i s(Point3_i(1, 2, 3), Point3_i(4, 5, 6));
  s.ShiftX(1);

  EXPECT_EQ(s.p0(), Point3_i(2, 2, 3));
  EXPECT_EQ(s.p1(), Point3_i(5, 5, 6));
}

TEST(Operations, ShiftY) {
  Segment3_i s(Point3_i(1, 2, 3), Point3_i(4, 5, 6));
  s.ShiftY(1);

  EXPECT_EQ(s.p0(), Point3_i(1, 3, 3));
  EXPECT_EQ(s.p1(), Point3_i(4, 6, 6));
}

TEST(Operations, ShiftZ) {
  Segment3_i s(Point3_i(1, 2, 3), Point3_i(4, 5, 6));
  s.ShiftZ(1);

  EXPECT_EQ(s.p0(), Point3_i(1, 2, 4));
  EXPECT_EQ(s.p1(), Point3_i(4, 5, 7));
}

TEST(Operators, SubscriptAccess) {
  Segment3_i s(Point3_i(1, 2, 3), Point3_i(4, 5, 6));

  EXPECT_EQ(s[0], Point3_i(1, 2, 3));
  EXPECT_EQ(s[1], Point3_i(4, 5, 6));
}

TEST(Operators, SubscriptMutation) {
  Segment3_i s(Point3_i(1, 2, 3), Point3_i(4, 5, 6));
  s[0] = Point3_i(7, 8, 9);
  s[1] = Point3_i(10, 11, 12);

  EXPECT_EQ(s[0], Point3_i(7, 8, 9));
  EXPECT_EQ(s[1], Point3_i(10, 11, 12));
}

TEST(Operators, Equality) {
  Segment3_i s1(Point3_i(1, 2, 3), Point3_i(4, 5, 6));
  Segment3_i s2(Point3_i(1, 2, 3), Point3_i(4, 5, 6));
  Segment3_i s3(Point3_i(7, 8, 9), Point3_i(10, 11, 12));

  EXPECT_TRUE(s1 == s2);
  EXPECT_FALSE(s1 == s3);
  EXPECT_EQ(s1, s2);
}

TEST(Operators, NotEquality) {
  Segment3_i s1(Point3_i(1, 2, 3), Point3_i(4, 5, 6));
  Segment3_i s2(Point3_i(1, 2, 3), Point3_i(4, 5, 6));
  Segment3_i s3(Point3_i(7, 8, 9), Point3_i(10, 11, 12));
  Segment3_i s4(Point3_i(1, 2, 3), Point3_i(3, 5, 6));
  Segment3_i s5(Point3_i(1, 2, 3), Point3_i(4, 4, 4));

  EXPECT_FALSE(s1 != s2);
  EXPECT_TRUE(s1 != s3);
  EXPECT_TRUE(s1 != s4);
  EXPECT_TRUE(s1 != s5);
  EXPECT_NE(s1, s3);
  EXPECT_NE(s1, s4);
  EXPECT_NE(s1, s5);
}

TEST(Operators, Inequality1) {
  Segment3_i s1(Point3_i(1, 2, 3), Point3_i(4, 5, 6));
  Segment3_i s2(Point3_i(4, 5, 6), Point3_i(7, 8, 9));

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
  Segment3_i s1(Point3_i(1, 2, 3), Point3_i(4, 5, 6));
  Segment3_i s2(Point3_i(1, 2, 3), Point3_i(4, 5, 6));

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
  Segment3_i s(Point3_i(1, 2, 3), Point3_i(4, 5, 6));

  EXPECT_THAT(s.ToString(), StrEq("((1 2 3) (4 5 6))"));
}

TEST(StringConversion, SupportsAbslStringify) {
  Segment3_i seg(Point3_i(1, 2, 3), Point3_i(4, 5, 6));
  std::string s = absl::StrFormat("%v", seg);

  EXPECT_THAT(s, StrEq("((1 2 3) (4 5 6))"));
}

TEST(Hash, SupportsAbslHash) {
  EXPECT_TRUE(absl::VerifyTypeImplementsAbslHashCorrectly({
      Segment3_i(),
      Segment3_i(1, 2, 3, 4, 5, 6),
      Segment3_i(1, 2, 3, 4, 5, 6),
      Segment3_i(Point3_i(1, 2, 3), Point3_i(4, 5, 6)),
      Segment3_i(Point3_i(7, 8, 9), Point3_i(10, 11, 12)),
  }));
}

TEST(Protobuf, ToProto) {
  Segment3_i s(Point3_i(1, 2, 3), Point3_i(4, 5, 6));
  Segment3Proto proto = s.ToProto();

  EXPECT_TRUE(proto.has_segment_int32());
  EXPECT_EQ(proto.segment_int32().p0().x(), 1);
  EXPECT_EQ(proto.segment_int32().p0().y(), 2);
  EXPECT_EQ(proto.segment_int32().p0().z(), 3);
  EXPECT_EQ(proto.segment_int32().p1().x(), 4);
  EXPECT_EQ(proto.segment_int32().p1().y(), 5);
  EXPECT_EQ(proto.segment_int32().p1().z(), 6);
}

TEST(Protobuf, SetFromProto) {
  Segment3Proto proto;
  proto.mutable_segment_int32()->mutable_p0()->set_x(1);
  proto.mutable_segment_int32()->mutable_p0()->set_y(2);
  proto.mutable_segment_int32()->mutable_p0()->set_z(3);
  proto.mutable_segment_int32()->mutable_p1()->set_x(4);
  proto.mutable_segment_int32()->mutable_p1()->set_y(5);
  proto.mutable_segment_int32()->mutable_p1()->set_z(6);

  Segment3_i s;
  ASSERT_THAT(s.SetFromProto(proto), ::absl_testing::IsOk());

  EXPECT_EQ(s.p0(), Point3_i(1, 2, 3));
  EXPECT_EQ(s.p1(), Point3_i(4, 5, 6));
}

using ::absl_testing::IsOkAndHolds;
using ::absl_testing::StatusIs;
using ::testing::HasSubstr;

TEST(Protobuf, FromProtoRoundTrip) {
  EXPECT_THAT(Segment3_i::FromProto(Segment3_i(6, 5, 4, 3, 2, 1).ToProto()),
              IsOkAndHolds(Segment3_i(6, 5, 4, 3, 2, 1)));
}

TEST(Protobuf, FromProtoNotSet) {
  EXPECT_THAT(Segment3_i::FromProto(Segment3Proto()),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("[Segment3] The proto has no coordinate type "
                                 "set.")));
}

TEST(Protobuf, FromProtoNotWholeNumber) {
  Segment3Proto proto;
  proto.mutable_segment_double()->mutable_p0()->set_z(2.25);

  EXPECT_THAT(Segment3_i::FromProto(proto),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("[Segment3] p0.z: coordinate 2.25 is not a "
                                 "whole number")));
}

TEST(Protobuf, SetFromProtoNotSetLeavesValueUnchanged) {
  Segment3_i v = Segment3_i(1, 2, 3, 4, 5, 6);

  EXPECT_THAT(v.SetFromProto(Segment3Proto()),
              StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_EQ(v, Segment3_i(1, 2, 3, 4, 5, 6));
}

// operator[] checks the index with DCHECK, so only debug builds abort.
#ifndef NDEBUG
TEST(OperatorsDeathTest, SubscriptOutOfRange) {
  Segment3_i v(0, 0, 0, 1, 1, 1);
  const Segment3_i& const_v = v;

  EXPECT_DEATH((void)v[2], "Check failed: i < d_.size");
  EXPECT_DEATH((void)const_v[2], "Check failed: i < d_.size");
}
#endif

}  // namespace moab
