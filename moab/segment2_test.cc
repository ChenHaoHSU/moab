#include "segment2.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>
#include <type_traits>
#include <utility>

#include "absl/hash/hash_testing.h"
#include "absl/status/status.h"
#include "absl/status/status_matchers.h"
#include "absl/strings/str_format.h"
#include "moab/point2.h"
#include "moab/segment2.pb.h"

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

TEST(Constructors, Proto) {
  Segment2Proto proto;
  proto.mutable_segment_int32()->mutable_p0()->set_x(1);
  proto.mutable_segment_int32()->mutable_p0()->set_y(2);
  proto.mutable_segment_int32()->mutable_p1()->set_x(3);
  proto.mutable_segment_int32()->mutable_p1()->set_y(4);

  Segment2_i s(proto);

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

// data() is read-only, even on a non-const segment.
TEST(Accessors, Data) {
  static_assert(std::is_same_v<decltype(std::declval<Segment2_i&>().data()),
                               const Point2_i*>);
  Segment2_i s(Point2_i(1, 2), Point2_i(3, 4));
  const Point2_i* data = s.data();

  EXPECT_EQ(data[0], Point2_i(1, 2));
  EXPECT_EQ(data[1], Point2_i(3, 4));
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

TEST(Protobuf, ToProto) {
  Segment2_i s(Point2_i(1, 2), Point2_i(3, 4));
  Segment2Proto proto = s.ToProto();

  EXPECT_TRUE(proto.has_segment_int32());
  EXPECT_EQ(proto.segment_int32().p0().x(), 1);
  EXPECT_EQ(proto.segment_int32().p0().y(), 2);
  EXPECT_EQ(proto.segment_int32().p1().x(), 3);
  EXPECT_EQ(proto.segment_int32().p1().y(), 4);
}

TEST(Protobuf, SetFromProto) {
  Segment2Proto proto;
  proto.mutable_segment_int32()->mutable_p0()->set_x(1);
  proto.mutable_segment_int32()->mutable_p0()->set_y(2);
  proto.mutable_segment_int32()->mutable_p1()->set_x(3);
  proto.mutable_segment_int32()->mutable_p1()->set_y(4);

  Segment2_i s;
  ASSERT_THAT(s.SetFromProto(proto), ::absl_testing::IsOk());

  EXPECT_EQ(s.p0(), Point2_i(1, 2));
  EXPECT_EQ(s.p1(), Point2_i(3, 4));
}

// Boost.Polygon sees p0 as LOW and p1 as HIGH, without reordering.
TEST(BoostPolygon, LowHigh) {
  Segment2_i s(10, 0, 0, 0);

  EXPECT_EQ(boost::polygon::low(s), Point2_i(10, 0));
  EXPECT_EQ(boost::polygon::high(s), Point2_i(0, 0));
}

TEST(BoostPolygon, SetLowThenHigh) {
  Segment2_i s(0, 0, 10, 0);

  boost::polygon::set(s, boost::polygon::LOW, Point2_i(20, 0));
  boost::polygon::set(s, boost::polygon::HIGH, Point2_i(30, 0));

  EXPECT_EQ(s, Segment2_i(20, 0, 30, 0));
}

TEST(BoostPolygon, SetHighThenLow) {
  Segment2_i s(0, 0, 10, 0);

  boost::polygon::set(s, boost::polygon::HIGH, Point2_i(-10, 0));
  boost::polygon::set(s, boost::polygon::LOW, Point2_i(-20, 0));

  EXPECT_EQ(s, Segment2_i(-20, 0, -10, 0));
}

TEST(BoostPolygon, SetDegenerate) {
  Segment2_i s(5, 5, 5, 5);

  boost::polygon::set(s, boost::polygon::LOW, Point2_i(0, 0));
  boost::polygon::set(s, boost::polygon::HIGH, Point2_i(9, 9));

  EXPECT_EQ(s, Segment2_i(0, 0, 9, 9));
}

TEST(BoostPolygon, Construct) {
  Segment2_i s =
      boost::polygon::construct<Segment2_i>(Point2_i(3, 4), Point2_i(1, 2));

  EXPECT_EQ(s, Segment2_i(3, 4, 1, 2));
}

TEST(BoostPolygon, Convolve) {
  Segment2_i s(0, 0, 10, 0);

  boost::polygon::convolve(s, Point2_i(100, 0));

  EXPECT_EQ(s, Segment2_i(100, 0, 110, 0));
}

TEST(BoostPolygon, ConvolveReversed) {
  Segment2_i s(10, 0, 0, 0);

  boost::polygon::convolve(s, Point2_i(0, 7));

  EXPECT_EQ(s, Segment2_i(10, 7, 0, 7));
}

TEST(BoostPolygon, Move) {
  Segment2_i s(0, 0, 10, 5);

  boost::polygon::move(s, boost::polygon::HORIZONTAL, 3);
  boost::polygon::move(s, boost::polygon::VERTICAL, -2);

  EXPECT_EQ(s, Segment2_i(3, -2, 13, 3));
}

TEST(BoostPolygon, ScaleUp) {
  Segment2_i s(1, 2, 3, 4);

  boost::polygon::scale_up(s, 10);

  EXPECT_EQ(s, Segment2_i(10, 20, 30, 40));
}

TEST(StringConversion, ToStringFloatingPoint) {
  EXPECT_THAT(Segment2<double>(0.5, 1, 2, 3.5).ToString(),
              StrEq("((0.5 1) (2 3.5))"));
}

using ::absl_testing::IsOkAndHolds;
using ::absl_testing::StatusIs;
using ::testing::HasSubstr;

TEST(Protobuf, FromProtoRoundTrip) {
  EXPECT_THAT(Segment2_i::FromProto(Segment2_i(4, 3, 2, 1).ToProto()),
              IsOkAndHolds(Segment2_i(4, 3, 2, 1)));
  EXPECT_THAT(
      Segment2<double>::FromProto(Segment2<double>(0.5, 1, 2, 3.5).ToProto()),
      IsOkAndHolds(Segment2<double>(0.5, 1, 2, 3.5)));
}

TEST(Protobuf, FromProtoNotSet) {
  EXPECT_THAT(Segment2_i::FromProto(Segment2Proto()),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("[Segment2] The proto has no coordinate type "
                                 "set.")));
}

TEST(Protobuf, FromProtoOutOfRange) {
  Segment2Proto proto;
  proto.mutable_segment_int64()->mutable_p0()->set_x(0);
  proto.mutable_segment_int64()->mutable_p1()->set_x(int64_t{1} << 40);

  EXPECT_THAT(Segment2_i::FromProto(proto),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("[Segment2] p1.x: coordinate 1099511627776 "
                                 "does not fit in int32.")));
}

TEST(Protobuf, SetFromProtoNotSetLeavesValueUnchanged) {
  Segment2_i v = Segment2_i(1, 2, 3, 4);

  EXPECT_THAT(v.SetFromProto(Segment2Proto()),
              StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_EQ(v, Segment2_i(1, 2, 3, 4));
}

// operator[] checks the index with DCHECK, so only debug builds abort.
#ifndef NDEBUG
TEST(OperatorsDeathTest, SubscriptOutOfRange) {
  Segment2_i v(0, 0, 1, 1);
  const Segment2_i& const_v = v;

  EXPECT_DEATH((void)v[2], "Check failed: i < d_.size");
  EXPECT_DEATH((void)const_v[2], "Check failed: i < d_.size");
}
#endif

}  // namespace moab
