#include "point3.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "absl/hash/hash_testing.h"
#include "absl/status/status.h"
#include "absl/status/status_matchers.h"
#include "absl/strings/str_format.h"
#include "boost/geometry/core/access.hpp"
#include "boost/geometry/core/coordinate_dimension.hpp"
#include "boost/geometry/core/coordinate_system.hpp"
#include "boost/geometry/core/coordinate_type.hpp"
#include "boost/geometry/core/cs.hpp"
#include "boost/geometry/core/tag.hpp"
#include "boost/geometry/core/tags.hpp"
#include "moab/point2.h"
#include "moab/point3.pb.h"

// The tests follow the order of point3.h.

namespace moab {
namespace {

namespace bg = boost::geometry;

using ::absl_testing::IsOk;
using ::absl_testing::IsOkAndHolds;
using ::absl_testing::StatusIs;
using ::testing::FieldsAre;
using ::testing::HasSubstr;
using ::testing::StrEq;

// The behavior common to all coordinate types runs as typed tests over these.
// The values used there are small integers, which are exact in every type.
using CoordinateTypes = ::testing::Types<int32_t, int64_t, float, double>;

// Checks the coordinates of 'p'. The coordinate parameters are not deduced, so
// int literals can be passed for any T.
template <typename T>
void ExpectXyz(const Point3<T>& p, typename Point3<T>::coordinate_type x,
               typename Point3<T>::coordinate_type y,
               typename Point3<T>::coordinate_type z) {
  EXPECT_EQ(p.x(), x);
  EXPECT_EQ(p.y(), y);
  EXPECT_EQ(p.z(), z);
}

// Returns a proto holding (x, y, z) in the variant 'c'.
Point3Proto MakeProto(Point3Proto::TypeVariantCase c, int x, int y, int z) {
  Point3Proto proto;
  switch (c) {
    case Point3Proto::kPointInt32:
      proto.mutable_point_int32()->set_x(x);
      proto.mutable_point_int32()->set_y(y);
      proto.mutable_point_int32()->set_z(z);
      break;
    case Point3Proto::kPointInt64:
      proto.mutable_point_int64()->set_x(x);
      proto.mutable_point_int64()->set_y(y);
      proto.mutable_point_int64()->set_z(z);
      break;
    case Point3Proto::kPointFloat:
      proto.mutable_point_float()->set_x(x);
      proto.mutable_point_float()->set_y(y);
      proto.mutable_point_float()->set_z(z);
      break;
    case Point3Proto::kPointDouble:
      proto.mutable_point_double()->set_x(x);
      proto.mutable_point_double()->set_y(y);
      proto.mutable_point_double()->set_z(z);
      break;
    case Point3Proto::TYPE_VARIANT_NOT_SET:
      break;
  }
  return proto;
}

// The four proto variants, each named for SCOPED_TRACE.
const std::vector<std::pair<std::string, Point3Proto::TypeVariantCase>>&
ProtoVariants() {
  static const auto* const kVariants =
      new std::vector<std::pair<std::string, Point3Proto::TypeVariantCase>>{
          {"point_int32", Point3Proto::kPointInt32},
          {"point_int64", Point3Proto::kPointInt64},
          {"point_float", Point3Proto::kPointFloat},
          {"point_double", Point3Proto::kPointDouble},
      };
  return *kVariants;
}

// The proto variant ToProto writes for coordinate type T.
template <typename T>
Point3Proto::TypeVariantCase VariantOf() {
  if constexpr (std::is_same_v<T, int32_t>) {
    return Point3Proto::kPointInt32;
  } else if constexpr (std::is_same_v<T, int64_t>) {
    return Point3Proto::kPointInt64;
  } else if constexpr (std::is_same_v<T, float>) {
    return Point3Proto::kPointFloat;
  } else {
    return Point3Proto::kPointDouble;
  }
}

// The sub-message of variant VariantOf<T>() in 'proto'.
template <typename T>
const auto& VariantMessage(const Point3Proto& proto) {
  if constexpr (std::is_same_v<T, int32_t>) {
    return proto.point_int32();
  } else if constexpr (std::is_same_v<T, int64_t>) {
    return proto.point_int64();
  } else if constexpr (std::is_same_v<T, float>) {
    return proto.point_float();
  } else {
    return proto.point_double();
  }
}

// =============================================================
// Type aliases
// =============================================================

TEST(Point3TypeAliases, AreaTypeIsInt64ForIntegralCoordinates) {
  static_assert(std::is_same_v<Point3_i::area_type, int64_t>);
  static_assert(std::is_same_v<Point3_i32::area_type, int64_t>);
  static_assert(std::is_same_v<Point3_i64::area_type, int64_t>);
  static_assert(std::is_same_v<Point3<float>::area_type, float>);
  static_assert(std::is_same_v<Point3<double>::area_type, double>);
  static_assert(std::is_same_v<Point3_i::coordinate_type, int>);
  static_assert(std::is_same_v<Point3_i64::coordinate_type, int64_t>);
}

// =============================================================
// Constructors
// =============================================================

template <typename T>
class Point3Constructors : public ::testing::Test {};
TYPED_TEST_SUITE(Point3Constructors, CoordinateTypes);

TYPED_TEST(Point3Constructors, DefaultIsOrigin) {
  Point3<TypeParam> p;

  ExpectXyz(p, 0, 0, 0);
}

TYPED_TEST(Point3Constructors, XYZ) {
  Point3<TypeParam> p(1, 2, 3);

  ExpectXyz(p, 1, 2, 3);
}

TYPED_TEST(Point3Constructors, ProtoOfEveryVariant) {
  for (const auto& [name, variant] : ProtoVariants()) {
    SCOPED_TRACE(name);
    Point3<TypeParam> p(MakeProto(variant, 1, 2, 3));

    ExpectXyz(p, 1, 2, 3);
  }
}

TYPED_TEST(Point3Constructors, Copy) {
  Point3<TypeParam> p1(1, 2, 3);
  Point3<TypeParam> p2(p1);

  ExpectXyz(p2, 1, 2, 3);
}

TYPED_TEST(Point3Constructors, Move) {
  Point3<TypeParam> p1(1, 2, 3);
  Point3<TypeParam> p2(std::move(p1));

  ExpectXyz(p2, 1, 2, 3);
}

// The proto constructor CHECKs the result of SetFromProto, in every build.
TEST(Point3ConstructorsDeathTest, ProtoWithoutCoordinateTypeAborts) {
  EXPECT_DEATH(Point3_i{Point3Proto()}, "no coordinate type set");
}

// =============================================================
// Assignment operators
// =============================================================

template <typename T>
class Point3AssignmentOperators : public ::testing::Test {};
TYPED_TEST_SUITE(Point3AssignmentOperators, CoordinateTypes);

TYPED_TEST(Point3AssignmentOperators, Copy) {
  Point3<TypeParam> p1(1, 2, 3);
  Point3<TypeParam> p2;

  p2 = p1;

  ExpectXyz(p2, 1, 2, 3);
}

TYPED_TEST(Point3AssignmentOperators, Move) {
  Point3<TypeParam> p1(1, 2, 3);
  Point3<TypeParam> p2;

  p2 = std::move(p1);

  ExpectXyz(p2, 1, 2, 3);
}

// =============================================================
// Accessors
// =============================================================

template <typename T>
class Point3Accessors : public ::testing::Test {};
TYPED_TEST_SUITE(Point3Accessors, CoordinateTypes);

// data() is read-only, even on a non-const point.
TYPED_TEST(Point3Accessors, DataIsReadOnlyXyzArray) {
  static_assert(
      std::is_same_v<decltype(std::declval<Point3<TypeParam>&>().data()),
                     const TypeParam*>);
  Point3<TypeParam> p(1, 2, 3);
  const TypeParam* d = p.data();

  ASSERT_NE(d, nullptr);
  EXPECT_EQ(d[0], 1);
  EXPECT_EQ(d[1], 2);
  EXPECT_EQ(d[2], 3);
}

TYPED_TEST(Point3Accessors, DataOfConstPoint) {
  const Point3<TypeParam> p(1, 2, 3);
  const TypeParam* d = p.data();

  ASSERT_NE(d, nullptr);
  EXPECT_EQ(d[0], 1);
  EXPECT_EQ(d[1], 2);
  EXPECT_EQ(d[2], 3);
}

TYPED_TEST(Point3Accessors, SizeIsThree) {
  Point3<TypeParam> p;

  EXPECT_EQ(p.Size(), 3u);
}

TYPED_TEST(Point3Accessors, To2DDropsZ) {
  Point3<TypeParam> p(1, 2, 3);

  EXPECT_EQ(p.To2D(), Point2<TypeParam>(1, 2));
}

TYPED_TEST(Point3Accessors, ToTuple) {
  Point3<TypeParam> p(1, 2, 3);

  EXPECT_THAT(p.ToTuple(), FieldsAre(1, 2, 3));
}

// =============================================================
// Mutators
// =============================================================

template <typename T>
class Point3Mutators : public ::testing::Test {};
TYPED_TEST_SUITE(Point3Mutators, CoordinateTypes);

TYPED_TEST(Point3Mutators, Set) {
  Point3<TypeParam> p(1, 2, 3);
  p.Set(4, 5, 6);

  ExpectXyz(p, 4, 5, 6);
}

TYPED_TEST(Point3Mutators, SetXChangesOnlyX) {
  Point3<TypeParam> p(1, 2, 3);
  p.SetX(4);

  ExpectXyz(p, 4, 2, 3);
}

TYPED_TEST(Point3Mutators, SetYChangesOnlyY) {
  Point3<TypeParam> p(1, 2, 3);
  p.SetY(4);

  ExpectXyz(p, 1, 4, 3);
}

TYPED_TEST(Point3Mutators, SetZChangesOnlyZ) {
  Point3<TypeParam> p(1, 2, 3);
  p.SetZ(4);

  ExpectXyz(p, 1, 2, 4);
}

TYPED_TEST(Point3Mutators, SetDim) {
  Point3<TypeParam> p(1, 2, 3);
  p.SetDim(0, 4);
  p.SetDim(1, 5);
  p.SetDim(2, 6);

  ExpectXyz(p, 4, 5, 6);
}

// =============================================================
// Operations
// =============================================================

template <typename T>
class Point3Operations : public ::testing::Test {};
TYPED_TEST_SUITE(Point3Operations, CoordinateTypes);

TYPED_TEST(Point3Operations, Shift) {
  Point3<TypeParam> p(1, 2, 3);
  p.Shift(4, 5, 6);

  ExpectXyz(p, 5, 7, 9);
}

TYPED_TEST(Point3Operations, ShiftXChangesOnlyX) {
  Point3<TypeParam> p(1, 2, 3);
  p.ShiftX(3);

  ExpectXyz(p, 4, 2, 3);
}

TYPED_TEST(Point3Operations, ShiftYChangesOnlyY) {
  Point3<TypeParam> p(1, 2, 3);
  p.ShiftY(4);

  ExpectXyz(p, 1, 6, 3);
}

TYPED_TEST(Point3Operations, ShiftZChangesOnlyZ) {
  Point3<TypeParam> p(1, 2, 3);
  p.ShiftZ(5);

  ExpectXyz(p, 1, 2, 8);
}

// Rotating (1, 2, 3) in every ordered axis pair. Counterclockwise by 90 in
// (a1, a2) maps (p[a1], p[a2]) to (-p[a2], p[a1]); 180 negates both; 270 maps
// them to (p[a2], -p[a1]), which is 90 in (a2, a1). The third coordinate is
// unchanged.
TYPED_TEST(Point3Operations, RotateInEveryAxisPair) {
  struct Case {
    std::string name;
    std::size_t axis1;
    std::size_t axis2;
    Point3<TypeParam> rotate90;
    Point3<TypeParam> rotate180;
    Point3<TypeParam> rotate270;
  };
  using P = Point3<TypeParam>;
  const std::vector<Case> cases = {
      {"XY", 0, 1, P(-2, 1, 3), P(-1, -2, 3), P(2, -1, 3)},
      {"YX", 1, 0, P(2, -1, 3), P(-1, -2, 3), P(-2, 1, 3)},
      {"XZ", 0, 2, P(-3, 2, 1), P(-1, 2, -3), P(3, 2, -1)},
      {"ZX", 2, 0, P(3, 2, -1), P(-1, 2, -3), P(-3, 2, 1)},
      {"YZ", 1, 2, P(1, -3, 2), P(1, -2, -3), P(1, 3, -2)},
      {"ZY", 2, 1, P(1, 3, -2), P(1, -2, -3), P(1, -3, 2)},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    P p90(1, 2, 3);
    p90.Rotate90(c.axis1, c.axis2);
    EXPECT_EQ(p90, c.rotate90);

    P p180(1, 2, 3);
    p180.Rotate180(c.axis1, c.axis2);
    EXPECT_EQ(p180, c.rotate180);

    P p270(1, 2, 3);
    p270.Rotate270(c.axis1, c.axis2);
    EXPECT_EQ(p270, c.rotate270);
  }
}

TYPED_TEST(Point3Operations, FourRotate90sAreIdentity) {
  Point3<TypeParam> p(1, 2, 3);
  for (int i = 0; i < 4; ++i) p.Rotate90(0, 2);

  ExpectXyz(p, 1, 2, 3);
}

// =============================================================
// Operators
// =============================================================

template <typename T>
class Point3Operators : public ::testing::Test {};
TYPED_TEST_SUITE(Point3Operators, CoordinateTypes);

TYPED_TEST(Point3Operators, SubscriptAccess) {
  Point3<TypeParam> p(1, 2, 3);

  EXPECT_EQ(p[0], 1);
  EXPECT_EQ(p[1], 2);
  EXPECT_EQ(p[2], 3);
}

TYPED_TEST(Point3Operators, SubscriptAccessOnConstPoint) {
  const Point3<TypeParam> p(1, 2, 3);

  EXPECT_EQ(p[0], 1);
  EXPECT_EQ(p[1], 2);
  EXPECT_EQ(p[2], 3);
}

TYPED_TEST(Point3Operators, SubscriptMutation) {
  Point3<TypeParam> p(1, 2, 3);
  p[0] = 4;
  p[1] = 5;
  p[2] = 6;

  EXPECT_EQ(p[0], 4);
  EXPECT_EQ(p[1], 5);
  EXPECT_EQ(p[2], 6);
}

TYPED_TEST(Point3Operators, Equality) {
  Point3<TypeParam> p1(1, 2, 3);
  Point3<TypeParam> p2(1, 2, 3);
  Point3<TypeParam> p3(1, 2, 4);

  EXPECT_TRUE(p1 == p2);
  EXPECT_FALSE(p1 == p3);
  EXPECT_EQ(p1, p2);
}

TYPED_TEST(Point3Operators, NotEqualWhenAnyCoordinateDiffers) {
  Point3<TypeParam> p1(1, 2, 3);
  Point3<TypeParam> p2(1, 2, 3);
  Point3<TypeParam> p3(1, 3, 4);
  Point3<TypeParam> p4(2, 2, 4);
  Point3<TypeParam> p5(3, 4, 5);

  EXPECT_FALSE(p1 != p2);
  EXPECT_TRUE(p1 != p3);
  EXPECT_TRUE(p1 != p4);
  EXPECT_TRUE(p1 != p5);
  EXPECT_NE(p1, p3);
  EXPECT_NE(p1, p4);
  EXPECT_NE(p1, p5);
}

// operator< is lexicographic: x decides, then y, then z.
TYPED_TEST(Point3Operators, OrderIsLexicographicXThenYThenZ) {
  struct Case {
    std::string name;
    Point3<TypeParam> a;
    Point3<TypeParam> b;
    int expected;  // -1: a < b, 0: a == b, 1: a > b.
  };
  using P = Point3<TypeParam>;
  const std::vector<Case> cases = {
      {"XDecidesOverYAndZ", P(1, 2, 3), P(2, 1, 4), -1},
      {"YDecidesWhenXEqual", P(1, 2, 3), P(1, 3, 4), -1},
      {"ZDecidesWhenXYEqual", P(1, 2, 3), P(1, 2, 4), -1},
      {"AllSmaller", P(1, 2, 3), P(2, 3, 4), -1},
      {"XGreater", P(2, 1, 0), P(1, 2, 3), 1},
      {"YGreater", P(1, 3, 0), P(1, 2, 3), 1},
      {"ZGreater", P(1, 2, 4), P(1, 2, 3), 1},
      {"Equal", P(1, 2, 3), P(1, 2, 3), 0},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    EXPECT_EQ(c.a < c.b, c.expected < 0);
    EXPECT_EQ(c.b > c.a, c.expected < 0);
    EXPECT_EQ(c.a > c.b, c.expected > 0);
    EXPECT_EQ(c.b < c.a, c.expected > 0);
    EXPECT_EQ(c.a <= c.b, c.expected <= 0);
    EXPECT_EQ(c.b >= c.a, c.expected <= 0);
    EXPECT_EQ(c.a >= c.b, c.expected >= 0);
    EXPECT_EQ(c.b <= c.a, c.expected >= 0);
  }
}

TYPED_TEST(Point3Operators, PointAdditionAssignment) {
  Point3<TypeParam> p1(1, 2, 3);
  Point3<TypeParam> p2(4, 5, 6);
  p1 += p2;

  ExpectXyz(p1, 5, 7, 9);
  ExpectXyz(p2, 4, 5, 6);
}

TYPED_TEST(Point3Operators, PointSubtractionAssignment) {
  Point3<TypeParam> p1(1, 2, 3);
  Point3<TypeParam> p2(4, 5, 6);
  p1 -= p2;

  ExpectXyz(p1, -3, -3, -3);
  ExpectXyz(p2, 4, 5, 6);
}

TYPED_TEST(Point3Operators, PointAddition) {
  Point3<TypeParam> p1(1, 2, 3);
  Point3<TypeParam> p2(4, 5, 6);
  Point3<TypeParam> p3 = (p1 + p2);

  ExpectXyz(p1, 1, 2, 3);
  ExpectXyz(p2, 4, 5, 6);
  ExpectXyz(p3, 5, 7, 9);
}

TYPED_TEST(Point3Operators, PointSubtraction) {
  Point3<TypeParam> p1(1, 2, 3);
  Point3<TypeParam> p2(4, 5, 6);
  Point3<TypeParam> p3 = (p1 - p2);

  ExpectXyz(p1, 1, 2, 3);
  ExpectXyz(p2, 4, 5, 6);
  ExpectXyz(p3, -3, -3, -3);
}

TYPED_TEST(Point3Operators, PointArithmeticOnConstPoints) {
  const Point3<TypeParam> a(1, 2, 3);
  const Point3<TypeParam> b(4, 6, 8);

  EXPECT_EQ(a + b, Point3<TypeParam>(5, 8, 11));
  EXPECT_EQ(b - a, Point3<TypeParam>(3, 4, 5));
}

TYPED_TEST(Point3Operators, UnaryPlusCopies) {
  Point3<TypeParam> p1(1, 2, 3);
  Point3<TypeParam> p2 = +p1;

  ExpectXyz(p1, 1, 2, 3);
  ExpectXyz(p2, 1, 2, 3);
}

TYPED_TEST(Point3Operators, UnaryMinusNegatesEveryCoordinate) {
  Point3<TypeParam> p1(1, 2, 3);
  Point3<TypeParam> p2 = -p1;

  ExpectXyz(p1, 1, 2, 3);
  ExpectXyz(p2, -1, -2, -3);
}

// The scalar operators apply the scalar to x, y and z.
TYPED_TEST(Point3Operators, ScalarAdditionAssignment) {
  Point3<TypeParam> p1(1, 2, 3);
  p1 += 1;

  ExpectXyz(p1, 2, 3, 4);
}

TYPED_TEST(Point3Operators, ScalarSubtractionAssignment) {
  Point3<TypeParam> p1(1, 2, 3);
  p1 -= 1;

  ExpectXyz(p1, 0, 1, 2);
}

TYPED_TEST(Point3Operators, ScalarMultiplicationAssignment) {
  Point3<TypeParam> p1(1, 2, 3);
  p1 *= 2;

  ExpectXyz(p1, 2, 4, 6);
}

TYPED_TEST(Point3Operators, ScalarDivisionAssignment) {
  Point3<TypeParam> p1(2, 4, 6);
  p1 /= 2;

  ExpectXyz(p1, 1, 2, 3);
}

TYPED_TEST(Point3Operators, ScalarAddition) {
  Point3<TypeParam> p1(1, 2, 3);
  Point3<TypeParam> p2 = p1 + 1;

  ExpectXyz(p1, 1, 2, 3);
  ExpectXyz(p2, 2, 3, 4);
}

TYPED_TEST(Point3Operators, ScalarSubtraction) {
  Point3<TypeParam> p1(1, 2, 3);
  Point3<TypeParam> p2 = p1 - 1;

  ExpectXyz(p1, 1, 2, 3);
  ExpectXyz(p2, 0, 1, 2);
}

TYPED_TEST(Point3Operators, ScalarMultiplication) {
  Point3<TypeParam> p1(1, 2, 3);
  Point3<TypeParam> p2 = p1 * 2;

  ExpectXyz(p1, 1, 2, 3);
  ExpectXyz(p2, 2, 4, 6);
}

TYPED_TEST(Point3Operators, ScalarDivision) {
  Point3<TypeParam> p1(2, 4, 6);
  Point3<TypeParam> p2 = p1 / 2;

  ExpectXyz(p1, 2, 4, 6);
  ExpectXyz(p2, 1, 2, 3);
}

// Integer division truncates every coordinate toward zero.
TEST(Point3OperatorsInteger, ScalarDivisionTruncatesTowardZero) {
  EXPECT_EQ(Point3_i(7, -7, 3) / 2, Point3_i(3, -3, 1));
}

TEST(Point3OperatorsFloating, ScalarDivisionIsExact) {
  EXPECT_EQ(Point3<double>(7, -7, 3) / 2, Point3<double>(3.5, -3.5, 1.5));
}

// operator[] checks the index with DCHECK, so only debug builds abort.
#ifndef NDEBUG
TEST(Point3OperatorsDeathTest, SubscriptOutOfRange) {
  Point3_i v(1, 2, 3);
  const Point3_i& const_v = v;

  EXPECT_DEATH((void)v[3], "Check failed: i < d_.size");
  EXPECT_DEATH((void)const_v[3], "Check failed: i < d_.size");
}
#endif

// =============================================================
// Distance
// =============================================================

template <typename T>
class Point3Distance : public ::testing::Test {};
TYPED_TEST_SUITE(Point3Distance, CoordinateTypes);

TYPED_TEST(Point3Distance, DistanceIsManhattan) {
  Point3<TypeParam> p1(0, 0, 0);
  Point3<TypeParam> p2(1, 2, 3);
  Point3<TypeParam> p3(10, 20, 30);

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

// |4 - 1| + |6 - 2| + |15 - 3| = 3 + 4 + 12.
TYPED_TEST(Point3Distance, ManhattanDistance) {
  const Point3<TypeParam> p(1, 2, 3);
  const Point3<TypeParam> q(4, 6, 15);

  EXPECT_EQ(p.ManhattanDistance(q), 19);
  EXPECT_EQ(q.ManhattanDistance(p), 19);
  EXPECT_EQ(p.Distance(q), p.ManhattanDistance(q));
}

// 3^2 + 4^2 + 12^2 = 169 = 13^2.
TYPED_TEST(Point3Distance, SquaredEuclideanDistance) {
  const Point3<TypeParam> p(1, 2, 3);
  const Point3<TypeParam> q(4, 6, 15);

  const typename Point3<TypeParam>::area_type d = p.SquaredEuclideanDistance(q);
  EXPECT_EQ(d, 169);
  EXPECT_EQ(q.SquaredEuclideanDistance(p), 169);
  EXPECT_EQ(p.SquaredEuclideanDistance(p), 0);
}

TYPED_TEST(Point3Distance, EuclideanDistance) {
  const Point3<TypeParam> p(1, 2, 3);
  const Point3<TypeParam> q(4, 6, 15);

  EXPECT_DOUBLE_EQ(p.EuclideanDistance(q), 13.0);
  EXPECT_DOUBLE_EQ(q.EuclideanDistance(p), 13.0);
  EXPECT_DOUBLE_EQ(p.EuclideanDistance(p), 0.0);
}

// The squared distance is computed in int64_t, so it does not overflow int.
TEST(Point3DistanceInteger, SquaredEuclideanDistanceDoesNotOverflowInt) {
  static_assert(std::is_same_v<Point3_i::area_type, int64_t>);

  EXPECT_EQ(Point3_i(1, 2, 3).SquaredEuclideanDistance(Point3_i(4, 6, 15)),
            169);
  EXPECT_EQ(Point3_i(0, 0, -1000000000)
                .SquaredEuclideanDistance(Point3_i(0, 0, 1000000000)),
            int64_t{4000000000000000000});
}

TEST(Point3DistanceInteger, EuclideanDistanceOfInt) {
  EXPECT_DOUBLE_EQ(Point3_i(1, 2, 3).EuclideanDistance(Point3_i(4, 6, 15)),
                   13.0);
}

// =============================================================
// String conversion
// =============================================================

template <typename T>
class Point3StringConversion : public ::testing::Test {};
TYPED_TEST_SUITE(Point3StringConversion, CoordinateTypes);

TYPED_TEST(Point3StringConversion, ToString) {
  Point3<TypeParam> p(1, 2, 3);

  EXPECT_THAT(p.ToString(), StrEq("(1 2 3)"));
}

TYPED_TEST(Point3StringConversion, SupportsAbslStringify) {
  Point3<TypeParam> p(1, 2, 3);
  std::string s = absl::StrFormat("%v", p);

  EXPECT_THAT(s, StrEq("(1 2 3)"));
}

TYPED_TEST(Point3StringConversion, StreamOperator) {
  std::ostringstream os;
  os << Point3<TypeParam>(1, -2, 3) << ";";

  EXPECT_THAT(os.str(), StrEq("(1 -2 3);"));
}

TEST(Point3StringConversionPerType, ToStringFloatingPoint) {
  EXPECT_THAT(Point3<double>(1.5, -2.25, 0).ToString(), StrEq("(1.5 -2.25 0)"));
  EXPECT_THAT(Point3<float>(0.5f, 3.0f, 0.75f).ToString(),
              StrEq("(0.5 3 0.75)"));
}

TEST(Point3StringConversionPerType, ToStringInt64) {
  EXPECT_THAT(Point3_i64(int64_t{1} << 40, -1, 2).ToString(),
              StrEq("(1099511627776 -1 2)"));
}

// =============================================================
// Hash
// =============================================================

template <typename T>
class Point3Hash : public ::testing::Test {};
TYPED_TEST_SUITE(Point3Hash, CoordinateTypes);

TYPED_TEST(Point3Hash, SupportsAbslHash) {
  using P = Point3<TypeParam>;
  EXPECT_TRUE(absl::VerifyTypeImplementsAbslHashCorrectly({
      P(),
      P(1, 2, 3),
      P(0, 0, 0),
      P(2, 3, 4),
      P(0, -1, -2),
  }));
}

// =============================================================
// Protobuf
// =============================================================

template <typename T>
class Point3Protobuf : public ::testing::Test {};
TYPED_TEST_SUITE(Point3Protobuf, CoordinateTypes);

// ToProto writes the variant of the coordinate type.
TYPED_TEST(Point3Protobuf, ToProtoWritesVariantOfCoordinateType) {
  Point3Proto proto = Point3<TypeParam>(1, -2, 3).ToProto();

  EXPECT_EQ(proto.type_variant_case(), VariantOf<TypeParam>());
  EXPECT_EQ(VariantMessage<TypeParam>(proto).x(), 1);
  EXPECT_EQ(VariantMessage<TypeParam>(proto).y(), -2);
  EXPECT_EQ(VariantMessage<TypeParam>(proto).z(), 3);
}

// Every coordinate type reads every proto variant.
TYPED_TEST(Point3Protobuf, FromProtoOfEveryVariant) {
  for (const auto& [name, variant] : ProtoVariants()) {
    SCOPED_TRACE(name);
    EXPECT_THAT(Point3<TypeParam>::FromProto(MakeProto(variant, 1, -2, 3)),
                IsOkAndHolds(Point3<TypeParam>(1, -2, 3)));
  }
}

TYPED_TEST(Point3Protobuf, FromProtoRoundTrip) {
  const Point3<TypeParam> p(-4, 5, -6);

  EXPECT_THAT(Point3<TypeParam>::FromProto(p.ToProto()), IsOkAndHolds(p));
}

TYPED_TEST(Point3Protobuf, SetFromProtoOfEveryVariant) {
  for (const auto& [name, variant] : ProtoVariants()) {
    SCOPED_TRACE(name);
    Point3<TypeParam> p;
    ASSERT_THAT(p.SetFromProto(MakeProto(variant, 1, 2, 3)), IsOk());

    ExpectXyz(p, 1, 2, 3);
  }
}

TYPED_TEST(Point3Protobuf, FromProtoNotSetIsInvalidArgument) {
  EXPECT_THAT(Point3<TypeParam>::FromProto(Point3Proto()),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("[Point3] The proto has no coordinate type "
                                 "set.")));
}

TYPED_TEST(Point3Protobuf, SetFromProtoNotSetLeavesValueUnchanged) {
  Point3<TypeParam> v(7, 8, 9);

  EXPECT_THAT(v.SetFromProto(Point3Proto()),
              StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_EQ(v, Point3<TypeParam>(7, 8, 9));
}

TEST(Point3ProtobufPerType, ToProtoInt) {
  Point3_i p(1, 2, 3);
  Point3Proto proto = p.ToProto();

  EXPECT_TRUE(proto.has_point_int32());
  EXPECT_EQ(proto.point_int32().x(), 1);
  EXPECT_EQ(proto.point_int32().y(), 2);
  EXPECT_EQ(proto.point_int32().z(), 3);
}

TEST(Point3ProtobufPerType, FromProtoRoundTripKeepsLargeAndFractionalValues) {
  EXPECT_THAT(Point3_i::FromProto(Point3_i(1, -2, 3).ToProto()),
              IsOkAndHolds(Point3_i(1, -2, 3)));
  EXPECT_THAT(
      Point3_i64::FromProto(Point3_i64(int64_t{1} << 40, 5, -6).ToProto()),
      IsOkAndHolds(Point3_i64(int64_t{1} << 40, 5, -6)));
  EXPECT_THAT(Point3<double>::FromProto(Point3<double>(0.1, 2.5, -3).ToProto()),
              IsOkAndHolds(Point3<double>(0.1, 2.5, -3)));
}

TEST(Point3ProtobufPerType, FromProtoXNotFinite) {
  Point3Proto proto;
  proto.mutable_point_double()->set_x(std::numeric_limits<double>::infinity());

  EXPECT_THAT(Point3<double>::FromProto(proto),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("[Point3] x: coordinate inf is not finite")));
}

TEST(Point3ProtobufPerType, FromProtoYDoesNotFitInt32) {
  Point3Proto proto;
  proto.mutable_point_int64()->set_y(int64_t{1} << 40);

  EXPECT_THAT(Point3_i::FromProto(proto),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("[Point3] y: coordinate 1099511627776 does "
                                 "not fit in int32")));
}

TEST(Point3ProtobufPerType, FromProtoZNotWholeNumber) {
  Point3Proto proto;
  proto.mutable_point_float()->set_x(0);
  proto.mutable_point_float()->set_y(0);
  proto.mutable_point_float()->set_z(0.5f);

  EXPECT_THAT(Point3_i::FromProto(proto),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("[Point3] z: coordinate 0.5 is not a whole "
                                 "number")));
}

TEST(Point3ProtobufPerType, FromProtoDoubleDoesNotFitFloat) {
  Point3Proto proto;
  proto.mutable_point_double()->set_z(1e300);

  EXPECT_THAT(Point3<float>::FromProto(proto),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("[Point3] z: coordinate 1e+300 does not fit "
                                 "in float")));
}

TEST(Point3ProtobufPerType, SetFromProtoErrorLeavesValueUnchanged) {
  Point3Proto proto;
  proto.mutable_point_float()->set_y(0.5f);
  Point3_i v(7, 8, 9);

  EXPECT_THAT(v.SetFromProto(proto),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("[Point3] y: coordinate 0.5")));
  EXPECT_EQ(v, Point3_i(7, 8, 9));
}

// =============================================================
// Boost geometry traits
// =============================================================

template <typename T>
class Point3BoostGeometry : public ::testing::Test {};
TYPED_TEST_SUITE(Point3BoostGeometry, CoordinateTypes);

TYPED_TEST(Point3BoostGeometry, Traits) {
  using P = Point3<TypeParam>;
  static_assert(std::is_same_v<typename bg::tag<P>::type, bg::point_tag>);
  static_assert(bg::dimension<P>::value == 3);
  static_assert(
      std::is_same_v<typename bg::coordinate_type<P>::type, TypeParam>);
  static_assert(std::is_same_v<typename bg::coordinate_system<P>::type,
                               bg::cs::cartesian>);
}

TYPED_TEST(Point3BoostGeometry, Get) {
  const Point3<TypeParam> p(1, 2, 3);

  EXPECT_EQ(bg::get<0>(p), 1);
  EXPECT_EQ(bg::get<1>(p), 2);
  EXPECT_EQ(bg::get<2>(p), 3);
}

TYPED_TEST(Point3BoostGeometry, Set) {
  Point3<TypeParam> p(1, 2, 3);
  bg::set<0>(p, 4);
  bg::set<1>(p, 5);
  bg::set<2>(p, 6);

  ExpectXyz(p, 4, 5, 6);
}

}  // namespace
}  // namespace moab
