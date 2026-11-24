#include "box2.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <list>
#include <optional>
#include <set>
#include <string>
#include <type_traits>
#include <vector>

#include "absl/hash/hash_testing.h"
#include "absl/status/status.h"
#include "absl/status/status_matchers.h"
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

TEST(Utilities, BoundingBoxVector1) {
  std::vector<Point2_i> points = {Point2_i(1, 2)};

  std::optional<Box2_i> box = Box2_i::BoundingBox(points);
  ASSERT_TRUE(box.has_value());

  EXPECT_EQ(box->ll(), Point2_i(1, 2));
  EXPECT_EQ(box->ur(), Point2_i(1, 2));
}

TEST(Utilities, BoundingBoxVector2) {
  std::vector<Point2_i> points = {
      Point2_i(1, 2),
      Point2_i(3, 4),
      Point2_i(-1, 5),
      Point2_i(0, 0),
  };

  std::optional<Box2_i> box = Box2_i::BoundingBox(points);
  ASSERT_TRUE(box.has_value());

  EXPECT_EQ(box->ll(), Point2_i(-1, 0));
  EXPECT_EQ(box->ur(), Point2_i(3, 5));
}

TEST(Utilities, BoundingBoxVector3) {
  std::vector<Point2_i> points = {
      Point2_i(1, 2),
      Point2_i(1, 2),
      Point2_i(1, 2),
      Point2_i(1, 2),
  };

  std::optional<Box2_i> box = Box2_i::BoundingBox(points);
  ASSERT_TRUE(box.has_value());

  EXPECT_EQ(box->ll(), Point2_i(1, 2));
  EXPECT_EQ(box->ur(), Point2_i(1, 2));
}

TEST(Utilities, BoundingBoxVector4) {
  std::vector<Point2_i> points = {
      Point2_i(-10, -20), Point2_i(10, 20), Point2_i(-5, 5),
      Point2_i(5, -5),    Point2_i(0, 0),
  };

  std::optional<Box2_i> box = Box2_i::BoundingBox(points);
  ASSERT_TRUE(box.has_value());

  EXPECT_EQ(box->ll(), Point2_i(-10, -20));
  EXPECT_EQ(box->ur(), Point2_i(10, 20));
}

TEST(Utilities, BoundingBoxVectorEmptyContainer) {
  std::vector<Point2_i> points;

  EXPECT_EQ(Box2_i::BoundingBox(points), std::nullopt);
}

TEST(Utilities, BoundingBoxArray) {
  std::array<Point2_i, 3> points = {
      Point2_i(1, 2),
      Point2_i(3, 4),
      Point2_i(-1, 5),
  };

  std::optional<Box2_i> box = Box2_i::BoundingBox(points);
  ASSERT_TRUE(box.has_value());

  EXPECT_EQ(box->ll(), Point2_i(-1, 2));
  EXPECT_EQ(box->ur(), Point2_i(3, 5));
}

TEST(Utilities, BoundingBoxArrayEmptyContainer) {
  std::array<Point2_i, 0> points = {};

  EXPECT_EQ(Box2_i::BoundingBox(points), std::nullopt);
}

TEST(Utilities, BoundingBoxInitializerList) {
  std::optional<Box2_i> box = Box2_i::BoundingBox({
      Point2_i(1, 2),
      Point2_i(3, 4),
      Point2_i(-1, 5),
  });
  ASSERT_TRUE(box.has_value());

  EXPECT_EQ(box->ll(), Point2_i(-1, 2));
  EXPECT_EQ(box->ur(), Point2_i(3, 5));
}

TEST(Utilities, BoundingBoxInitializerListEmptyContainer) {
  EXPECT_EQ(Box2_i::BoundingBox({}), std::nullopt);
}

TEST(Utilities, BoundingBoxList) {
  std::list<Point2_i> points = {
      Point2_i(1, 2),
      Point2_i(3, 4),
      Point2_i(-1, 5),
  };

  std::optional<Box2_i> box = Box2_i::BoundingBox(points);
  ASSERT_TRUE(box.has_value());

  EXPECT_EQ(box->ll(), Point2_i(-1, 2));
  EXPECT_EQ(box->ur(), Point2_i(3, 5));
}

TEST(Utilities, BoundingBoxListEmptyContainer) {
  std::list<Point2_i> points;

  EXPECT_EQ(Box2_i::BoundingBox(points), std::nullopt);
}

TEST(Utilities, BoundingBoxSet) {
  std::set<Point2_i> points = {
      Point2_i(1, 2),
      Point2_i(3, 4),
      Point2_i(-1, 5),
  };

  std::optional<Box2_i> box = Box2_i::BoundingBox(points);
  ASSERT_TRUE(box.has_value());

  EXPECT_EQ(box->ll(), Point2_i(-1, 2));
  EXPECT_EQ(box->ur(), Point2_i(3, 5));
}

TEST(Utilities, BoundingBoxSetEmptyContainer) {
  std::set<Point2_i> points;

  EXPECT_EQ(Box2_i::BoundingBox(points), std::nullopt);
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
  ASSERT_THAT(b.SetFromProto(proto), ::absl_testing::IsOk());

  EXPECT_EQ(b.ll(), Point2_i(1, 2));
  EXPECT_EQ(b.ur(), Point2_i(3, 4));
}

TEST(Accessors, AreaNoOverflow) {
  static_assert(std::is_same_v<Box2_i::area_type, int64_t>);
  static_assert(std::is_same_v<Box2_i64::area_type, int64_t>);
  static_assert(std::is_same_v<Box2<double>::area_type, double>);

  // 100000 x 100000 does not fit in an int.
  EXPECT_EQ(Box2_i(0, 0, 100000, 100000).Area(), int64_t{10000000000});
  EXPECT_EQ(Box2_i(-100000, -100000, 100000, 100000).Area(),
            int64_t{40000000000});
}

TEST(Accessors, AreaFloatingPoint) {
  EXPECT_DOUBLE_EQ(Box2<double>(0, 0, 1.5, 2.5).Area(), 3.75);
}

TEST(Protobuf, RoundTripInt64) {
  const int64_t big = int64_t{1} << 40;
  Box2_i64 b(-big, 0, big, big + 1);

  Box2Proto proto = b.ToProto();

  ASSERT_TRUE(proto.has_box_int64());
  EXPECT_EQ(proto.box_int64().min_corner().x(), -big);
  EXPECT_EQ(proto.box_int64().min_corner().y(), 0);
  EXPECT_EQ(proto.box_int64().max_corner().x(), big);
  EXPECT_EQ(proto.box_int64().max_corner().y(), big + 1);
  EXPECT_EQ(Box2_i64(proto), b);
}

TEST(Protobuf, RoundTripFloat) {
  Box2<float> b(0.5f, 1.25f, 2.0f, 3.0f);

  Box2Proto proto = b.ToProto();

  ASSERT_TRUE(proto.has_box_float());
  EXPECT_FLOAT_EQ(proto.box_float().min_corner().x(), 0.5f);
  EXPECT_FLOAT_EQ(proto.box_float().max_corner().y(), 3.0f);
  EXPECT_EQ(Box2<float>(proto), b);
}

TEST(Protobuf, RoundTripDouble) {
  Box2<double> b(-0.001, 0.25, 12.5, 7.75);

  Box2Proto proto = b.ToProto();

  ASSERT_TRUE(proto.has_box_double());
  EXPECT_DOUBLE_EQ(proto.box_double().min_corner().x(), -0.001);
  EXPECT_DOUBLE_EQ(proto.box_double().min_corner().y(), 0.25);
  EXPECT_DOUBLE_EQ(proto.box_double().max_corner().x(), 12.5);
  EXPECT_DOUBLE_EQ(proto.box_double().max_corner().y(), 7.75);
  EXPECT_EQ(Box2<double>(proto), b);
}

TEST(StringConversion, ToStringFloatingPoint) {
  EXPECT_THAT(Box2<double>(0.5, 1, 2, 3.5).ToString(),
              StrEq("((0.5 1) (2 3.5))"));
}

using ::absl_testing::IsOkAndHolds;
using ::absl_testing::StatusIs;
using ::testing::HasSubstr;

TEST(Protobuf, FromProtoRoundTrip) {
  EXPECT_THAT(Box2_i::FromProto(Box2_i(-1, 2, 3, 4).ToProto()),
              IsOkAndHolds(Box2_i(-1, 2, 3, 4)));
  EXPECT_THAT(Box2_i64::FromProto(
                  Box2_i64(0, 0, int64_t{1} << 40, int64_t{1} << 41).ToProto()),
              IsOkAndHolds(Box2_i64(0, 0, int64_t{1} << 40, int64_t{1} << 41)));
  EXPECT_THAT(Box2<double>::FromProto(Box2<double>(0.5, 1, 2, 3.5).ToProto()),
              IsOkAndHolds(Box2<double>(0.5, 1, 2, 3.5)));
}

TEST(Protobuf, FromProtoNormalizesCorners) {
  Box2Proto proto;
  proto.mutable_box_int32()->mutable_min_corner()->set_x(10);
  proto.mutable_box_int32()->mutable_min_corner()->set_y(20);
  proto.mutable_box_int32()->mutable_max_corner()->set_x(0);
  proto.mutable_box_int32()->mutable_max_corner()->set_y(5);

  EXPECT_THAT(Box2_i::FromProto(proto), IsOkAndHolds(Box2_i(0, 5, 10, 20)));
}

TEST(Protobuf, FromProtoNotSet) {
  EXPECT_THAT(Box2_i::FromProto(Box2Proto()),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("[Box2] The proto has no coordinate type "
                                 "set.")));
}

TEST(Protobuf, FromProtoOutOfRange) {
  Box2Proto proto;
  proto.mutable_box_double()->mutable_max_corner()->set_y(3e9);

  EXPECT_THAT(Box2_i::FromProto(proto),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("[Box2] max_corner.y: coordinate")));
}

TEST(Protobuf, SetFromProtoNotSetLeavesValueUnchanged) {
  Box2_i v = Box2_i(1, 2, 3, 4);

  EXPECT_THAT(v.SetFromProto(Box2Proto()),
              StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_EQ(v, Box2_i(1, 2, 3, 4));
}

// operator[] checks the index with DCHECK, so only debug builds abort.
#ifndef NDEBUG
TEST(OperatorsDeathTest, SubscriptOutOfRange) {
  Box2_i v(0, 0, 1, 1);
  const Box2_i& const_v = v;

  EXPECT_DEATH((void)v[2], "Check failed: i < d_.size");
  EXPECT_DEATH((void)const_v[2], "Check failed: i < d_.size");
}
#endif

}  // namespace moab
