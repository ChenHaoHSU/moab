#include "box2.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

#include "absl/hash/hash_testing.h"
#include "absl/strings/str_format.h"
#include "moab/box2.pb.h"
#include "moab/point2.h"
#include "moab/point2.pb.h"

namespace moab {

using ::testing::FieldsAre;
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

TEST(Constructors, Proto) {
  Box2Proto proto;
  proto.mutable_box_int32()->mutable_min_corner()->set_x(1);
  proto.mutable_box_int32()->mutable_min_corner()->set_y(2);
  proto.mutable_box_int32()->mutable_max_corner()->set_x(3);
  proto.mutable_box_int32()->mutable_max_corner()->set_y(4);

  Box2_i b(proto);

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

TEST(AssignmentOperators, Copy) {
  Box2_i b1(Point2_i(1, 2), Point2_i(3, 4));
  Box2_i b2;

  b2 = b1;

  EXPECT_EQ(b2.ll(), Point2_i(1, 2));
  EXPECT_EQ(b2.ur(), Point2_i(3, 4));
}

TEST(AssignmentOperators, Move) {
  Box2_i b1(Point2_i(1, 2), Point2_i(3, 4));
  Box2_i b2;

  b2 = std::move(b1);

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

TEST(Accessors, MinXMinYMaxXMaxY) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 4));

  EXPECT_EQ(b.MinX(), 1);
  EXPECT_EQ(b.MinY(), 2);
  EXPECT_EQ(b.MaxX(), 3);
  EXPECT_EQ(b.MaxY(), 4);
}

TEST(Accessors, WidthHeight) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 5));

  EXPECT_EQ(b.Width(), 2);
  EXPECT_EQ(b.Height(), 3);
}

TEST(Accessors, ToTuple) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 4));

  EXPECT_THAT(b.ToTuple(), FieldsAre(1, 2, 3, 4));
}

TEST(Accessors, Center1) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 6));

  EXPECT_EQ(b.CenterX(), 2);
  EXPECT_EQ(b.CenterY(), 4);
  EXPECT_EQ(b.Center(), Point2_i(2, 4));
}

TEST(Accessors, Center2) {
  Box2_i b(Point2_i(10, 20), Point2_i(15, 25));

  EXPECT_EQ(b.CenterX(), 12);
  EXPECT_EQ(b.CenterY(), 22);
  EXPECT_EQ(b.Center(), Point2_i(12, 22));
}

TEST(Accessors, Area) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 5));

  EXPECT_EQ(b.Area(), 6);
}

TEST(Accessors, HalfPerimeter) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 5));

  EXPECT_EQ(b.HalfPerimeter(), 5);
}

TEST(Accessors, Perimeter) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 5));

  EXPECT_EQ(b.Perimeter(), 10);
}

TEST(Mutators, SetByFourCoordinates) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 4));
  b.Set(5, 6, 7, 8);

  EXPECT_EQ(b.ll(), Point2_i(5, 6));
  EXPECT_EQ(b.ur(), Point2_i(7, 8));
}

TEST(Mutators, SetByFourCoordinatesAutoSwap) {
  Box2_i b(Point2_i(3, 4), Point2_i(1, 2));
  b.Set(7, 8, 5, 6);  // Automatically swaps.

  EXPECT_EQ(b.ll(), Point2_i(5, 6));
  EXPECT_EQ(b.ur(), Point2_i(7, 8));
}

TEST(Mutators, SetByTwoPoints) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 4));
  b.Set(Point2_i(5, 6), Point2_i(7, 8));

  EXPECT_EQ(b.ll(), Point2_i(5, 6));
  EXPECT_EQ(b.ur(), Point2_i(7, 8));
}

TEST(Mutators, SetByTwoPointsAutoSwap) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 4));
  b.Set(Point2_i(7, 8), Point2_i(5, 6));  // Automatically swaps.

  EXPECT_EQ(b.ll(), Point2_i(5, 6));
  EXPECT_EQ(b.ur(), Point2_i(7, 8));
}

TEST(Mutators, SetXL) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 4));
  b.set_xl(0);

  EXPECT_EQ(b.ll(), Point2_i(0, 2));
  EXPECT_EQ(b.ur(), Point2_i(3, 4));
}

TEST(Mutators, SetYL) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 4));
  b.set_yl(0);

  EXPECT_EQ(b.ll(), Point2_i(1, 0));
  EXPECT_EQ(b.ur(), Point2_i(3, 4));
}

TEST(Mutators, SetXH) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 4));
  b.set_xh(5);

  EXPECT_EQ(b.ll(), Point2_i(1, 2));
  EXPECT_EQ(b.ur(), Point2_i(5, 4));
}

TEST(Mutators, SetYH) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 4));
  b.set_yh(5);

  EXPECT_EQ(b.ll(), Point2_i(1, 2));
  EXPECT_EQ(b.ur(), Point2_i(3, 5));
}

TEST(Mutators, SetLL) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 4));
  b.set_ll(Point2_i(0, 1));

  EXPECT_EQ(b.ll(), Point2_i(0, 1));
  EXPECT_EQ(b.ur(), Point2_i(3, 4));
}

TEST(Mutators, SetUR) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 4));
  b.set_ur(Point2_i(5, 6));

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

TEST(Operations, Bloat) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 4));
  b.Bloat(5);

  EXPECT_EQ(b.ll(), Point2_i(-4, -3));
  EXPECT_EQ(b.ur(), Point2_i(8, 9));

  b.Bloat(-3);

  EXPECT_EQ(b.ll(), Point2_i(-1, 0));
  EXPECT_EQ(b.ur(), Point2_i(5, 6));
}

TEST(Operations, BloatXY) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 4));
  b.Bloat(5, 6);

  EXPECT_EQ(b.ll(), Point2_i(-4, -4));
  EXPECT_EQ(b.ur(), Point2_i(8, 10));

  b.Bloat(-3, -2);

  EXPECT_EQ(b.ll(), Point2_i(-1, -2));
  EXPECT_EQ(b.ur(), Point2_i(5, 8));
}

TEST(Operations, BloatX) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 4));
  b.BloatX(5);

  EXPECT_EQ(b.ll(), Point2_i(-4, 2));
  EXPECT_EQ(b.ur(), Point2_i(8, 4));

  b.BloatX(-3);

  EXPECT_EQ(b.ll(), Point2_i(-1, 2));
  EXPECT_EQ(b.ur(), Point2_i(5, 4));
}

TEST(Operations, BloatY) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 4));
  b.BloatY(5);

  EXPECT_EQ(b.ll(), Point2_i(1, -3));
  EXPECT_EQ(b.ur(), Point2_i(3, 9));

  b.BloatY(-3);

  EXPECT_EQ(b.ll(), Point2_i(1, 0));
  EXPECT_EQ(b.ur(), Point2_i(3, 6));
}

TEST(Operations, Shrink) {
  Box2_i b(Point2_i(0, 1), Point2_i(10, 20));
  b.Shrink(1);

  EXPECT_EQ(b.ll(), Point2_i(1, 2));
  EXPECT_EQ(b.ur(), Point2_i(9, 19));

  b.Shrink(-1);

  EXPECT_EQ(b.ll(), Point2_i(0, 1));
  EXPECT_EQ(b.ur(), Point2_i(10, 20));
}

TEST(Operations, ShrinkXY) {
  Box2_i b(Point2_i(0, 1), Point2_i(10, 20));
  b.Shrink(1, 2);

  EXPECT_EQ(b.ll(), Point2_i(1, 3));
  EXPECT_EQ(b.ur(), Point2_i(9, 18));

  b.Shrink(-1, -2);

  EXPECT_EQ(b.ll(), Point2_i(0, 1));
  EXPECT_EQ(b.ur(), Point2_i(10, 20));
}

TEST(Operations, ShrinkX) {
  Box2_i b(Point2_i(0, 1), Point2_i(10, 20));
  b.ShrinkX(1);

  EXPECT_EQ(b.ll(), Point2_i(1, 1));
  EXPECT_EQ(b.ur(), Point2_i(9, 20));

  b.ShrinkX(-1);

  EXPECT_EQ(b.ll(), Point2_i(0, 1));
  EXPECT_EQ(b.ur(), Point2_i(10, 20));
}

TEST(Operations, ShrinkY) {
  Box2_i b(Point2_i(0, 1), Point2_i(10, 20));
  b.ShrinkY(1);

  EXPECT_EQ(b.ll(), Point2_i(0, 2));
  EXPECT_EQ(b.ur(), Point2_i(10, 19));

  b.ShrinkY(-1);

  EXPECT_EQ(b.ll(), Point2_i(0, 1));
  EXPECT_EQ(b.ur(), Point2_i(10, 20));
}

TEST(Operations, EncompassPoint1) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 4));

  EXPECT_TRUE(b.Encompass(Point2_i(0, 1)));

  EXPECT_EQ(b.ll(), Point2_i(0, 1));
  EXPECT_EQ(b.ur(), Point2_i(3, 4));
}

TEST(Operations, EncompassPoint2) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 4));

  EXPECT_FALSE(b.Encompass(Point2_i(1, 2)));

  EXPECT_EQ(b.ll(), Point2_i(1, 2));
  EXPECT_EQ(b.ur(), Point2_i(3, 4));
}

TEST(Operations, EncompassPoint3) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 4));

  b.Encompass(Point2_i(100, -100));

  EXPECT_EQ(b.ll(), Point2_i(1, -100));
  EXPECT_EQ(b.ur(), Point2_i(100, 4));
}

TEST(Operations, EncompassBox1) {
  Box2_i b1(Point2_i(1, 2), Point2_i(3, 4));
  Box2_i b2(Point2_i(0, 1), Point2_i(5, 6));

  b1.Encompass(b2);

  EXPECT_EQ(b1.ll(), Point2_i(0, 1));
  EXPECT_EQ(b1.ur(), Point2_i(5, 6));
}

TEST(Operations, EncompassBox2) {
  Box2_i b1(Point2_i(1, 2), Point2_i(3, 4));
  Box2_i b2(Point2_i(1, 2), Point2_i(3, 4));

  b1.Encompass(b2);

  EXPECT_EQ(b1.ll(), Point2_i(1, 2));
  EXPECT_EQ(b1.ur(), Point2_i(3, 4));
}

TEST(Operations, EncompassBox3) {
  Box2_i b1(Point2_i(1, 2), Point2_i(3, 4));
  Box2_i b2(Point2_i(100, -100), Point2_i(200, -50));

  b1.Encompass(b2);

  EXPECT_EQ(b1.ll(), Point2_i(1, -100));
  EXPECT_EQ(b1.ur(), Point2_i(200, 4));
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

TEST(StringConversion, ToString) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 4));

  EXPECT_THAT(b.ToString(), StrEq("((1 2) (3 4))"));
}

TEST(StringConversion, SupportsAbslStringify) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 4));
  std::string s = absl::StrFormat("%v", b);

  EXPECT_THAT(s, StrEq("((1 2) (3 4))"));
}

TEST(Hash, SupportsAbslHash) {
  EXPECT_TRUE(absl::VerifyTypeImplementsAbslHashCorrectly({
      Box2_i(),
      Box2_i(1, 2, 3, 4),
      Box2_i(1, 2, 3, 4),
      Box2_i(Point2_i(1, 2), Point2_i(3, 4)),
      Box2_i(Point2_i(4, 5), Point2_i(7, 6)),
  }));
}

TEST(Protobuf, ToProto) {
  Box2_i b(Point2_i(1, 2), Point2_i(3, 4));
  Box2Proto proto = b.ToProto();

  EXPECT_TRUE(proto.has_box_int32());
  EXPECT_EQ(proto.box_int32().min_corner().x(), 1);
  EXPECT_EQ(proto.box_int32().min_corner().y(), 2);
  EXPECT_EQ(proto.box_int32().max_corner().x(), 3);
  EXPECT_EQ(proto.box_int32().max_corner().y(), 4);
}

TEST(Protobuf, SetFromProto) {
  Box2Proto proto;
  proto.mutable_box_int32()->mutable_min_corner()->set_x(1);
  proto.mutable_box_int32()->mutable_min_corner()->set_y(2);
  proto.mutable_box_int32()->mutable_max_corner()->set_x(3);
  proto.mutable_box_int32()->mutable_max_corner()->set_y(4);

  Box2_i b;
  b.SetFromProto(proto);

  EXPECT_EQ(b.ll(), Point2_i(1, 2));
  EXPECT_EQ(b.ur(), Point2_i(3, 4));
}

}  // namespace moab
