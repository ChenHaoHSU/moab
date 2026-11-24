#include "point2.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cmath>
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
#include "boost/polygon/isotropy.hpp"
#include "boost/polygon/point_concept.hpp"
#include "moab/point2.pb.h"

// The tests follow the order of point2.h.
//
// Behavior common to every coordinate type runs as typed tests over
// CoordinateTypes (suites without a suffix). Behavior specific to some types
// (integer rounding, overflow, float printing, conversion errors) runs as
// plain tests (suites with a "Types" or "Errors" suffix).

namespace moab {
namespace {

namespace bg = ::boost::geometry;
namespace gtl = ::boost::polygon;

using ::absl_testing::IsOk;
using ::absl_testing::IsOkAndHolds;
using ::absl_testing::StatusIs;
using ::testing::HasSubstr;
using ::testing::Pair;
using ::testing::StrEq;

// =============================================================
// Helpers
// =============================================================

using CoordinateTypes = ::testing::Types<int32_t, int64_t, float, double>;

// The four coordinate types a Point2Proto can hold.
struct ProtoVariant {
  std::string name;
  Point2Proto::TypeVariantCase variant;
};

const std::vector<ProtoVariant>& ProtoVariants() {
  static const auto* const kVariants = new std::vector<ProtoVariant>{
      {"int32", Point2Proto::kPointInt32},
      {"int64", Point2Proto::kPointInt64},
      {"float", Point2Proto::kPointFloat},
      {"double", Point2Proto::kPointDouble},
  };
  return *kVariants;
}

// Returns a Point2Proto holding (x, y) in 'variant'. The values must be exact
// in the variant's coordinate type.
Point2Proto MakeProto(Point2Proto::TypeVariantCase variant, int64_t x,
                      int64_t y) {
  Point2Proto proto;
  switch (variant) {
    case Point2Proto::kPointInt32:
      proto.mutable_point_int32()->set_x(static_cast<int32_t>(x));
      proto.mutable_point_int32()->set_y(static_cast<int32_t>(y));
      break;
    case Point2Proto::kPointInt64:
      proto.mutable_point_int64()->set_x(x);
      proto.mutable_point_int64()->set_y(y);
      break;
    case Point2Proto::kPointFloat:
      proto.mutable_point_float()->set_x(static_cast<float>(x));
      proto.mutable_point_float()->set_y(static_cast<float>(y));
      break;
    case Point2Proto::kPointDouble:
      proto.mutable_point_double()->set_x(static_cast<double>(x));
      proto.mutable_point_double()->set_y(static_cast<double>(y));
      break;
    case Point2Proto::TYPE_VARIANT_NOT_SET:
      break;
  }
  return proto;
}

// Returns the coordinates stored in 'proto', whatever its variant.
std::pair<double, double> ProtoCoordinates(const Point2Proto& proto) {
  switch (proto.type_variant_case()) {
    case Point2Proto::kPointInt32:
      return {proto.point_int32().x(), proto.point_int32().y()};
    case Point2Proto::kPointInt64:
      return {static_cast<double>(proto.point_int64().x()),
              static_cast<double>(proto.point_int64().y())};
    case Point2Proto::kPointFloat:
      return {proto.point_float().x(), proto.point_float().y()};
    case Point2Proto::kPointDouble:
      return {proto.point_double().x(), proto.point_double().y()};
    case Point2Proto::TYPE_VARIANT_NOT_SET:
      break;
  }
  ADD_FAILURE() << "The proto has no coordinate type set.";
  return {0, 0};
}

// The proto variant ToProto writes for coordinate type T.
template <typename T>
Point2Proto::TypeVariantCase VariantOf() {
  if constexpr (std::is_same_v<T, int32_t>) {
    return Point2Proto::kPointInt32;
  } else if constexpr (std::is_same_v<T, int64_t>) {
    return Point2Proto::kPointInt64;
  } else if constexpr (std::is_same_v<T, float>) {
    return Point2Proto::kPointFloat;
  } else {
    static_assert(std::is_same_v<T, double>);
    return Point2Proto::kPointDouble;
  }
}

// Typed suites. GoogleTest cannot mix TYPED_TEST and TEST in one suite, so
// the plain tests use different suite names.
template <typename T>
class Point2Constructors : public ::testing::Test {};
TYPED_TEST_SUITE(Point2Constructors, CoordinateTypes);

template <typename T>
class Point2Accessors : public ::testing::Test {};
TYPED_TEST_SUITE(Point2Accessors, CoordinateTypes);

template <typename T>
class Point2Mutators : public ::testing::Test {};
TYPED_TEST_SUITE(Point2Mutators, CoordinateTypes);

template <typename T>
class Point2Operations : public ::testing::Test {};
TYPED_TEST_SUITE(Point2Operations, CoordinateTypes);

template <typename T>
class Point2Operators : public ::testing::Test {};
TYPED_TEST_SUITE(Point2Operators, CoordinateTypes);

template <typename T>
class Point2Distance : public ::testing::Test {};
TYPED_TEST_SUITE(Point2Distance, CoordinateTypes);

template <typename T>
class Point2StringConversion : public ::testing::Test {};
TYPED_TEST_SUITE(Point2StringConversion, CoordinateTypes);

template <typename T>
class Point2Hash : public ::testing::Test {};
TYPED_TEST_SUITE(Point2Hash, CoordinateTypes);

template <typename T>
class Point2Protobuf : public ::testing::Test {};
TYPED_TEST_SUITE(Point2Protobuf, CoordinateTypes);

template <typename T>
class Point2BoostGeometry : public ::testing::Test {};
TYPED_TEST_SUITE(Point2BoostGeometry, CoordinateTypes);

template <typename T>
class Point2BoostPolygon : public ::testing::Test {};
TYPED_TEST_SUITE(Point2BoostPolygon, CoordinateTypes);

// =============================================================
// Type aliases
// =============================================================

TEST(Point2Types, CoordinateType) {
  static_assert(std::is_same_v<Point2<int32_t>::coordinate_type, int32_t>);
  static_assert(std::is_same_v<Point2<int64_t>::coordinate_type, int64_t>);
  static_assert(std::is_same_v<Point2<float>::coordinate_type, float>);
  static_assert(std::is_same_v<Point2<double>::coordinate_type, double>);
}

// area_type is int64_t for integral coordinates, and T otherwise.
TEST(Point2Types, AreaType) {
  static_assert(std::is_same_v<Point2<int32_t>::area_type, int64_t>);
  static_assert(std::is_same_v<Point2<int64_t>::area_type, int64_t>);
  static_assert(std::is_same_v<Point2<float>::area_type, float>);
  static_assert(std::is_same_v<Point2<double>::area_type, double>);
  static_assert(std::is_same_v<Point2_i::area_type, int64_t>);
}

// =============================================================
// Constructors and assignment operators
// =============================================================

TYPED_TEST(Point2Constructors, DefaultIsOrigin) {
  const Point2<TypeParam> p;

  EXPECT_EQ(p.x(), 0);
  EXPECT_EQ(p.y(), 0);
}

TYPED_TEST(Point2Constructors, XY) {
  const Point2<TypeParam> p(1, 2);

  EXPECT_EQ(p.x(), 1);
  EXPECT_EQ(p.y(), 2);
}

// The proto constructor reads every proto variant into every coordinate type.
TYPED_TEST(Point2Constructors, ProtoOfEachVariant) {
  for (const ProtoVariant& v : ProtoVariants()) {
    SCOPED_TRACE(v.name);
    const Point2<TypeParam> p(MakeProto(v.variant, 1, 2));

    EXPECT_EQ(p.x(), 1);
    EXPECT_EQ(p.y(), 2);
  }
}

TYPED_TEST(Point2Constructors, Copy) {
  const Point2<TypeParam> p1(1, 2);
  const Point2<TypeParam> p2(p1);

  EXPECT_EQ(p2.x(), 1);
  EXPECT_EQ(p2.y(), 2);
}

TYPED_TEST(Point2Constructors, Move) {
  const Point2<TypeParam> p(Point2<TypeParam>(1, 2));

  EXPECT_EQ(p.x(), 1);
  EXPECT_EQ(p.y(), 2);
}

TYPED_TEST(Point2Constructors, CopyAssignment) {
  const Point2<TypeParam> p1(1, 2);
  Point2<TypeParam> p2;

  p2 = p1;

  EXPECT_EQ(p2.x(), 1);
  EXPECT_EQ(p2.y(), 2);
}

TYPED_TEST(Point2Constructors, MoveAssignment) {
  Point2<TypeParam> p1(1, 2);
  Point2<TypeParam> p2;

  p2 = std::move(p1);

  EXPECT_EQ(p2.x(), 1);
  EXPECT_EQ(p2.y(), 2);
}

TEST(Point2ConstructorsTypes, ProtoInt32) {
  Point2Proto proto;
  proto.mutable_point_int32()->set_x(1);
  proto.mutable_point_int32()->set_y(2);

  const Point2_i p(proto);

  EXPECT_EQ(p.x(), 1);
  EXPECT_EQ(p.y(), 2);
}

// The proto constructor CHECKs the status of SetFromProto, so it aborts in
// every build mode.
TEST(Point2ConstructorsDeathTest, ProtoConstructorAbortsOnInvalidProto) {
  EXPECT_DEATH(Point2_i{Point2Proto()}, "no coordinate type set");

  Point2Proto out_of_range;
  out_of_range.mutable_point_int64()->set_x(int64_t{1} << 40);
  out_of_range.mutable_point_int64()->set_y(0);
  EXPECT_DEATH(Point2_i{out_of_range}, "does not fit in int32");
}

// =============================================================
// Accessors
// =============================================================

TYPED_TEST(Point2Accessors, XY) {
  const Point2<TypeParam> p(1, 2);

  EXPECT_EQ(p.x(), 1);
  EXPECT_EQ(p.y(), 2);
}

// data() is read-only, even on a non-const point.
TYPED_TEST(Point2Accessors, DataIsReadOnly) {
  using T = TypeParam;
  static_assert(
      std::is_same_v<decltype(std::declval<Point2<T>&>().data()), const T*>);
  static_assert(
      std::is_same_v<decltype(std::declval<const Point2<T>&>().data()),
                     const T*>);
  Point2<T> p(1, 2);
  const T* d = p.data();

  ASSERT_NE(d, nullptr);
  EXPECT_EQ(d[0], 1);
  EXPECT_EQ(d[1], 2);
}

TYPED_TEST(Point2Accessors, Size) {
  const Point2<TypeParam> p;

  EXPECT_EQ(p.Size(), 2u);
}

TYPED_TEST(Point2Accessors, ToPair) {
  using T = TypeParam;
  const Point2<T> p(1, 2);
  const std::pair<T, T> p_pair = p.ToPair();

  EXPECT_THAT(p_pair, Pair(1, 2));
}

// =============================================================
// Mutators
// =============================================================

TYPED_TEST(Point2Mutators, Set) {
  Point2<TypeParam> p(1, 2);
  p.Set(3, 4);

  EXPECT_EQ(p.x(), 3);
  EXPECT_EQ(p.y(), 4);
}

TYPED_TEST(Point2Mutators, SetX) {
  Point2<TypeParam> p(1, 2);
  p.SetX(3);

  EXPECT_EQ(p.x(), 3);
  EXPECT_EQ(p.y(), 2);
}

TYPED_TEST(Point2Mutators, SetY) {
  Point2<TypeParam> p(1, 2);
  p.SetY(4);

  EXPECT_EQ(p.x(), 1);
  EXPECT_EQ(p.y(), 4);
}

TYPED_TEST(Point2Mutators, SetDim) {
  Point2<TypeParam> p(1, 2);
  p.SetDim(0, 3);

  EXPECT_EQ(p.x(), 3);
  EXPECT_EQ(p.y(), 2);

  p.SetDim(1, 4);

  EXPECT_EQ(p.x(), 3);
  EXPECT_EQ(p.y(), 4);
}

// =============================================================
// Operations
// =============================================================

TYPED_TEST(Point2Operations, Shift) {
  Point2<TypeParam> p(1, 2);
  p.Shift(3, 4);

  EXPECT_EQ(p.x(), 4);
  EXPECT_EQ(p.y(), 6);

  p.Shift(-5, -7);

  EXPECT_EQ(p.x(), -1);
  EXPECT_EQ(p.y(), -1);
}

TYPED_TEST(Point2Operations, ShiftX) {
  Point2<TypeParam> p(1, 2);
  p.ShiftX(3);

  EXPECT_EQ(p.x(), 4);
  EXPECT_EQ(p.y(), 2);
}

TYPED_TEST(Point2Operations, ShiftY) {
  Point2<TypeParam> p(1, 2);
  p.ShiftY(4);

  EXPECT_EQ(p.x(), 1);
  EXPECT_EQ(p.y(), 6);
}

// Rotate90 turns (x, y) counterclockwise around the origin into (-y, x).
TYPED_TEST(Point2Operations, Rotate90IsCounterclockwiseAroundOrigin) {
  using P = Point2<TypeParam>;
  struct Case {
    std::string name;
    P p;
    P expected;
  };
  const std::vector<Case> cases = {
      {"first quadrant to second", P(1, 2), P(-2, 1)},
      {"positive x axis to positive y axis", P(3, 0), P(0, 3)},
      {"positive y axis to negative x axis", P(0, 3), P(-3, 0)},
      {"negative x axis to negative y axis", P(-3, 0), P(0, -3)},
      {"negative y axis to positive x axis", P(0, -3), P(3, 0)},
      {"third quadrant to fourth", P(-1, -2), P(2, -1)},
      {"origin stays", P(0, 0), P(0, 0)},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    P p = c.p;
    p.Rotate90();

    EXPECT_EQ(p, c.expected);
  }
}

TYPED_TEST(Point2Operations, Rotate90FourTimesIsIdentity) {
  Point2<TypeParam> p(1, 2);
  for (int i = 0; i < 4; ++i) p.Rotate90();

  EXPECT_EQ(p, Point2<TypeParam>(1, 2));
}

// Rotate180 turns (x, y) around the origin into (-x, -y).
TYPED_TEST(Point2Operations, Rotate180AroundOrigin) {
  using P = Point2<TypeParam>;
  struct Case {
    std::string name;
    P p;
    P expected;
  };
  const std::vector<Case> cases = {
      {"first quadrant to third", P(1, 2), P(-1, -2)},
      {"second quadrant to fourth", P(-3, 4), P(3, -4)},
      {"x axis", P(5, 0), P(-5, 0)},
      {"origin stays", P(0, 0), P(0, 0)},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    P p = c.p;
    p.Rotate180();

    EXPECT_EQ(p, c.expected);
  }
}

TYPED_TEST(Point2Operations, Rotate180EqualsTwoRotate90) {
  Point2<TypeParam> p(1, 2);
  Point2<TypeParam> q(1, 2);
  p.Rotate180();
  q.Rotate90();
  q.Rotate90();

  EXPECT_EQ(p, q);
}

// =============================================================
// Operators
// =============================================================

TYPED_TEST(Point2Operators, SubscriptAccess) {
  Point2<TypeParam> p(1, 2);
  const Point2<TypeParam>& const_p = p;

  EXPECT_EQ(p[0], 1);
  EXPECT_EQ(p[1], 2);
  EXPECT_EQ(const_p[0], 1);
  EXPECT_EQ(const_p[1], 2);
}

TYPED_TEST(Point2Operators, SubscriptMutation) {
  Point2<TypeParam> p(1, 2);
  p[0] = 3;
  p[1] = 4;

  EXPECT_EQ(p[0], 3);
  EXPECT_EQ(p[1], 4);
  EXPECT_EQ(p.x(), 3);
  EXPECT_EQ(p.y(), 4);
}

// operator[] checks the index with DCHECK, so only debug builds abort.
#ifndef NDEBUG
TEST(Point2OperatorsDeathTest, SubscriptOutOfRange) {
  Point2_i v(1, 2);
  const Point2_i& const_v = v;

  EXPECT_DEATH((void)v[2], "Check failed: i < d_.size");
  EXPECT_DEATH((void)const_v[2], "Check failed: i < d_.size");
}
#endif

TYPED_TEST(Point2Operators, Equality) {
  using P = Point2<TypeParam>;
  const P p1(1, 2);
  const P p2(1, 2);
  const P p3(1, 3);

  EXPECT_TRUE(p1 == p2);
  EXPECT_FALSE(p1 == p3);
  EXPECT_EQ(p1, p2);
}

TYPED_TEST(Point2Operators, NotEquality) {
  using P = Point2<TypeParam>;
  const P p1(1, 2);
  const P p2(1, 2);
  const P p3(1, 3);
  const P p4(2, 2);
  const P p5(3, 4);

  EXPECT_FALSE(p1 != p2);
  EXPECT_TRUE(p1 != p3);
  EXPECT_TRUE(p1 != p4);
  EXPECT_TRUE(p1 != p5);
  EXPECT_NE(p1, p3);
  EXPECT_NE(p1, p4);
  EXPECT_NE(p1, p5);
}

// operator< is lexicographic: x decides, and y only breaks a tie in x. Each
// case has a < b, and every relation is checked in both argument orders.
TYPED_TEST(Point2Operators, OrderIsLexicographic) {
  using P = Point2<TypeParam>;
  struct Case {
    std::string name;
    P a;
    P b;
  };
  const std::vector<Case> cases = {
      {"smaller x wins over larger y", P(1, 2), P(2, 1)},
      {"equal x, smaller y", P(1, 2), P(1, 3)},
      {"both smaller", P(1, 2), P(2, 3)},
      {"negative x", P(-5, 9), P(0, -9)},
      {"equal x, negative y", P(0, -1), P(0, 0)},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);

    EXPECT_TRUE(c.a < c.b);
    EXPECT_TRUE(c.b > c.a);
    EXPECT_TRUE(c.a <= c.b);
    EXPECT_TRUE(c.b >= c.a);
    EXPECT_FALSE(c.b < c.a);
    EXPECT_FALSE(c.a > c.b);
    EXPECT_FALSE(c.b <= c.a);
    EXPECT_FALSE(c.a >= c.b);
  }
}

TYPED_TEST(Point2Operators, OrderOfEqualPoints) {
  const Point2<TypeParam> a(1, 2);
  const Point2<TypeParam> b(1, 2);

  EXPECT_FALSE(a < b);
  EXPECT_FALSE(a > b);
  EXPECT_TRUE(a <= b);
  EXPECT_TRUE(a >= b);
}

TYPED_TEST(Point2Operators, PointAdditionAssignment) {
  using P = Point2<TypeParam>;
  P p1(1, 2);
  const P p2(3, 4);
  P& result = (p1 += p2);

  EXPECT_EQ(&result, &p1);
  EXPECT_EQ(p1.x(), 4);
  EXPECT_EQ(p1.y(), 6);
  EXPECT_EQ(p2.x(), 3);
  EXPECT_EQ(p2.y(), 4);
}

TYPED_TEST(Point2Operators, PointSubtractionAssignment) {
  using P = Point2<TypeParam>;
  P p1(1, 2);
  const P p2(3, 4);
  P& result = (p1 -= p2);

  EXPECT_EQ(&result, &p1);
  EXPECT_EQ(p1.x(), -2);
  EXPECT_EQ(p1.y(), -2);
  EXPECT_EQ(p2.x(), 3);
  EXPECT_EQ(p2.y(), 4);
}

TYPED_TEST(Point2Operators, PointAddition) {
  using P = Point2<TypeParam>;
  const P p1(1, 2);
  const P p2(3, 4);
  const P p3 = p1 + p2;

  EXPECT_EQ(p1.x(), 1);
  EXPECT_EQ(p1.y(), 2);
  EXPECT_EQ(p2.x(), 3);
  EXPECT_EQ(p2.y(), 4);
  EXPECT_EQ(p3.x(), 4);
  EXPECT_EQ(p3.y(), 6);
}

TYPED_TEST(Point2Operators, PointSubtraction) {
  using P = Point2<TypeParam>;
  const P p1(1, 2);
  const P p2(3, 4);
  const P p3 = p1 - p2;

  EXPECT_EQ(p1.x(), 1);
  EXPECT_EQ(p1.y(), 2);
  EXPECT_EQ(p2.x(), 3);
  EXPECT_EQ(p2.y(), 4);
  EXPECT_EQ(p3.x(), -2);
  EXPECT_EQ(p3.y(), -2);
}

TYPED_TEST(Point2Operators, PointArithmeticOnConstPoints) {
  using P = Point2<TypeParam>;
  const P a(1, 2);
  const P b(3, 5);

  EXPECT_EQ(a + b, P(4, 7));
  EXPECT_EQ(b - a, P(2, 3));
}

TYPED_TEST(Point2Operators, UnaryPlus) {
  using P = Point2<TypeParam>;
  const P p1(1, 2);
  const P p2 = +p1;

  EXPECT_EQ(p1.x(), 1);
  EXPECT_EQ(p1.y(), 2);
  EXPECT_EQ(p2.x(), 1);
  EXPECT_EQ(p2.y(), 2);
}

TYPED_TEST(Point2Operators, UnaryMinus) {
  using P = Point2<TypeParam>;
  const P p1(1, 2);
  const P p2 = -p1;

  EXPECT_EQ(p1.x(), 1);
  EXPECT_EQ(p1.y(), 2);
  EXPECT_EQ(p2.x(), -1);
  EXPECT_EQ(p2.y(), -2);
}

// The scalar operators apply the scalar to both coordinates.
TYPED_TEST(Point2Operators, ScalarAdditionAssignment) {
  Point2<TypeParam> p1(1, 2);
  Point2<TypeParam>& result = (p1 += 1);

  EXPECT_EQ(&result, &p1);
  EXPECT_EQ(p1.x(), 2);
  EXPECT_EQ(p1.y(), 3);
}

TYPED_TEST(Point2Operators, ScalarSubtractionAssignment) {
  Point2<TypeParam> p1(1, 2);
  Point2<TypeParam>& result = (p1 -= 1);

  EXPECT_EQ(&result, &p1);
  EXPECT_EQ(p1.x(), 0);
  EXPECT_EQ(p1.y(), 1);
}

TYPED_TEST(Point2Operators, ScalarMultiplicationAssignment) {
  Point2<TypeParam> p1(1, 2);
  Point2<TypeParam>& result = (p1 *= 2);

  EXPECT_EQ(&result, &p1);
  EXPECT_EQ(p1.x(), 2);
  EXPECT_EQ(p1.y(), 4);
}

TYPED_TEST(Point2Operators, ScalarDivisionAssignment) {
  Point2<TypeParam> p1(2, 4);
  Point2<TypeParam>& result = (p1 /= 2);

  EXPECT_EQ(&result, &p1);
  EXPECT_EQ(p1.x(), 1);
  EXPECT_EQ(p1.y(), 2);
}

TYPED_TEST(Point2Operators, ScalarAddition) {
  using P = Point2<TypeParam>;
  const P p1(1, 2);
  const P p2 = p1 + 1;

  EXPECT_EQ(p1.x(), 1);
  EXPECT_EQ(p1.y(), 2);
  EXPECT_EQ(p2.x(), 2);
  EXPECT_EQ(p2.y(), 3);
}

TYPED_TEST(Point2Operators, ScalarSubtraction) {
  using P = Point2<TypeParam>;
  const P p1(1, 2);
  const P p2 = p1 - 1;

  EXPECT_EQ(p1.x(), 1);
  EXPECT_EQ(p1.y(), 2);
  EXPECT_EQ(p2.x(), 0);
  EXPECT_EQ(p2.y(), 1);
}

TYPED_TEST(Point2Operators, ScalarMultiplication) {
  using P = Point2<TypeParam>;
  const P p1(1, 2);
  const P p2 = p1 * 2;

  EXPECT_EQ(p1.x(), 1);
  EXPECT_EQ(p1.y(), 2);
  EXPECT_EQ(p2.x(), 2);
  EXPECT_EQ(p2.y(), 4);
}

TYPED_TEST(Point2Operators, ScalarDivision) {
  using P = Point2<TypeParam>;
  const P p1(2, 4);
  const P p2 = p1 / 2;

  EXPECT_EQ(p1.x(), 2);
  EXPECT_EQ(p1.y(), 4);
  EXPECT_EQ(p2.x(), 1);
  EXPECT_EQ(p2.y(), 2);
}

// Integer division truncates toward zero, as C++ integer division does.
TEST(Point2OperatorsTypes, IntegerDivisionTruncatesTowardZero) {
  EXPECT_EQ(Point2_i(7, -7) / 2, Point2_i(3, -3));
  EXPECT_EQ(Point2_i64(7, -7) / 2, Point2_i64(3, -3));

  Point2_i p(7, -7);
  p /= 2;
  EXPECT_EQ(p, Point2_i(3, -3));
}

TEST(Point2OperatorsTypes, FloatingPointDivisionKeepsFractions) {
  EXPECT_EQ(Point2<double>(1, 3) / 2, Point2<double>(0.5, 1.5));
  EXPECT_EQ(Point2<float>(1, 3) / 2, Point2<float>(0.5f, 1.5f));
}

// =============================================================
// Distance
// =============================================================

// Each case is checked in both directions; every distance is symmetric.
TYPED_TEST(Point2Distance, Cases) {
  using T = TypeParam;
  using P = Point2<T>;
  static_assert(std::is_same_v<decltype(P().ManhattanDistance(P())), T>);
  static_assert(std::is_same_v<decltype(P().Distance(P())), T>);
  static_assert(std::is_same_v<decltype(P().SquaredEuclideanDistance(P())),
                               typename P::area_type>);
  static_assert(std::is_same_v<decltype(P().EuclideanDistance(P())), double>);
  struct Case {
    std::string name;
    P a;
    P b;
    T manhattan;
    typename P::area_type squared_euclidean;
    double euclidean;
  };
  const std::vector<Case> cases = {
      {"same point at origin", P(0, 0), P(0, 0), 0, 0, 0.0},
      {"same point off origin", P(3, 3), P(3, 3), 0, 0, 0.0},
      {"along the x axis", P(0, 0), P(10, 0), 10, 100, 10.0},
      {"along the y axis", P(0, 5), P(0, -2), 7, 49, 7.0},
      {"origin to third quadrant", P(0, 0), P(-1, -2), 3, 5, std::sqrt(5.0)},
      {"across quadrants", P(10, 0), P(-1, -2), 13, 125, std::sqrt(125.0)},
      {"3-4-5 triangle", P(1, 2), P(4, -2), 7, 25, 5.0},
      {"diagonal", P(0, 0), P(1, 1), 2, 2, std::sqrt(2.0)},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);

    EXPECT_EQ(c.a.ManhattanDistance(c.b), c.manhattan);
    EXPECT_EQ(c.b.ManhattanDistance(c.a), c.manhattan);
    EXPECT_EQ(c.a.Distance(c.b), c.manhattan);
    EXPECT_EQ(c.b.Distance(c.a), c.manhattan);
    EXPECT_EQ(c.a.SquaredEuclideanDistance(c.b), c.squared_euclidean);
    EXPECT_EQ(c.b.SquaredEuclideanDistance(c.a), c.squared_euclidean);
    EXPECT_DOUBLE_EQ(c.a.EuclideanDistance(c.b), c.euclidean);
    EXPECT_DOUBLE_EQ(c.b.EuclideanDistance(c.a), c.euclidean);
  }
}

// Distance is the same as ManhattanDistance.
TYPED_TEST(Point2Distance, DistanceIsManhattanDistance) {
  const Point2<TypeParam> p(1, 2);
  const Point2<TypeParam> q(4, -2);

  EXPECT_EQ(p.ManhattanDistance(q), 7);
  EXPECT_EQ(p.Distance(q), p.ManhattanDistance(q));
}

TEST(Point2DistanceTypes, ManhattanDistanceFromEachPointToEachPoint) {
  const Point2_i p1(0, 0);
  const Point2_i p2(10, 0);
  const Point2_i p3(-1, -2);

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

TEST(Point2DistanceTypes, SquaredEuclideanDistanceInt) {
  EXPECT_EQ(Point2_i(1, 2).SquaredEuclideanDistance(Point2_i(4, -2)), 25);
  EXPECT_EQ(Point2_i(4, -2).SquaredEuclideanDistance(Point2_i(1, 2)), 25);
  EXPECT_EQ(Point2_i(3, 3).SquaredEuclideanDistance(Point2_i(3, 3)), 0);
}

// For an int32 point, dx is computed in int64_t, so a gap that does not fit
// in an int (2e9) and its square (4e18) are exact.
TEST(Point2DistanceTypes, SquaredEuclideanDistanceInt32NoOverflow) {
  EXPECT_EQ(Point2_i(-1000000000, 0)
                .SquaredEuclideanDistance(Point2_i(1000000000, 0)),
            int64_t{4000000000000000000});
  EXPECT_EQ(Point2_i(0, 1000000000)
                .SquaredEuclideanDistance(Point2_i(0, -1000000000)),
            int64_t{4000000000000000000});
  EXPECT_DOUBLE_EQ(
      Point2_i(-1000000000, 0).EuclideanDistance(Point2_i(1000000000, 0)), 2e9);
}

TEST(Point2DistanceTypes, EuclideanDistance) {
  EXPECT_DOUBLE_EQ(Point2_i(1, 2).EuclideanDistance(Point2_i(4, -2)), 5.0);
  EXPECT_DOUBLE_EQ(Point2<double>(0, 0).EuclideanDistance(Point2<double>(1, 1)),
                   std::sqrt(2.0));
}

TEST(Point2DistanceTypes, FloatingPointFractions) {
  const Point2<double> a(0.5, 0.25);
  const Point2<double> b(-1, 2.25);

  EXPECT_DOUBLE_EQ(a.ManhattanDistance(b), 3.5);
  EXPECT_DOUBLE_EQ(a.SquaredEuclideanDistance(b), 6.25);
  EXPECT_DOUBLE_EQ(a.EuclideanDistance(b), 2.5);
}

// =============================================================
// String conversion
// =============================================================

// ToString, AbslStringify (%v) and operator<< all print "(x y)".
TYPED_TEST(Point2StringConversion, AllFormsPrintXSpaceY) {
  using P = Point2<TypeParam>;
  struct Case {
    std::string name;
    P p;
    std::string expected;
  };
  const std::vector<Case> cases = {
      {"positive", P(1, 2), "(1 2)"},
      {"negative", P(-3, -40), "(-3 -40)"},
      {"origin", P(), "(0 0)"},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    std::ostringstream os;
    os << c.p;

    EXPECT_THAT(c.p.ToString(), StrEq(c.expected));
    EXPECT_THAT(absl::StrFormat("%v", c.p), StrEq(c.expected));
    EXPECT_THAT(os.str(), StrEq(c.expected));
  }
}

TEST(Point2StringConversionTypes, ToStringInt) {
  const Point2_i p(1, 2);

  EXPECT_THAT(p.ToString(), StrEq("(1 2)"));
}

TEST(Point2StringConversionTypes, AbslStringifyInt) {
  const Point2_i p(1, 2);
  const std::string s = absl::StrFormat("%v", p);

  EXPECT_THAT(s, StrEq("(1 2)"));
}

TEST(Point2StringConversionTypes, FloatingPoint) {
  EXPECT_THAT(Point2<double>(1.5, -2.25).ToString(), StrEq("(1.5 -2.25)"));
  EXPECT_THAT(Point2<float>(0.5f, 3.0f).ToString(), StrEq("(0.5 3)"));
  EXPECT_THAT(absl::StrFormat("%v", Point2<double>(0.125, 4)),
              StrEq("(0.125 4)"));
  EXPECT_THAT(absl::StrFormat("%v", Point2<float>(-0.75f, 2.5f)),
              StrEq("(-0.75 2.5)"));

  std::ostringstream os;
  os << Point2<double>(0.5, -8) << " " << Point2<float>(1.25f, 0);
  EXPECT_THAT(os.str(), StrEq("(0.5 -8) (1.25 0)"));
}

TEST(Point2StringConversionTypes, Int64BeyondInt32) {
  EXPECT_THAT(Point2_i64(int64_t{1} << 40, -1).ToString(),
              StrEq("(1099511627776 -1)"));
  EXPECT_THAT(absl::StrFormat("%v", Point2_i64(-(int64_t{1} << 40), 7)),
              StrEq("(-1099511627776 7)"));

  std::ostringstream os;
  os << Point2_i64(int64_t{1} << 40, 0);
  EXPECT_THAT(os.str(), StrEq("(1099511627776 0)"));
}

// =============================================================
// Hash
// =============================================================

TYPED_TEST(Point2Hash, SupportsAbslHash) {
  using P = Point2<TypeParam>;
  EXPECT_TRUE(absl::VerifyTypeImplementsAbslHashCorrectly({
      P(),
      P(1, 2),
      P(0, 0),
      P(2, 3),
      P(0, -1),
      P(2, 1),
  }));
}

// =============================================================
// Protobuf
// =============================================================

// ToProto writes the variant of the point's own coordinate type.
TYPED_TEST(Point2Protobuf, ToProtoWritesTheVariantOfT) {
  using P = Point2<TypeParam>;
  struct Case {
    std::string name;
    P p;
    std::pair<double, double> expected;
  };
  const std::vector<Case> cases = {
      {"positive", P(1, 2), {1, 2}},
      {"negative", P(-3, -4), {-3, -4}},
      {"origin", P(), {0, 0}},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    const Point2Proto proto = c.p.ToProto();

    EXPECT_EQ(proto.type_variant_case(), VariantOf<TypeParam>());
    EXPECT_EQ(ProtoCoordinates(proto), c.expected);
  }
}

// FromProto reads every proto variant into every coordinate type.
TYPED_TEST(Point2Protobuf, FromProtoOfEachVariant) {
  using P = Point2<TypeParam>;
  for (const ProtoVariant& v : ProtoVariants()) {
    SCOPED_TRACE(v.name);

    EXPECT_THAT(P::FromProto(MakeProto(v.variant, 1, -2)),
                IsOkAndHolds(P(1, -2)));
  }
}

TYPED_TEST(Point2Protobuf, FromProtoRoundTrip) {
  using P = Point2<TypeParam>;

  EXPECT_THAT(P::FromProto(P(3, -4).ToProto()), IsOkAndHolds(P(3, -4)));
  EXPECT_THAT(P::FromProto(P().ToProto()), IsOkAndHolds(P()));
}

TYPED_TEST(Point2Protobuf, FromProtoNotSet) {
  EXPECT_THAT(Point2<TypeParam>::FromProto(Point2Proto()),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("[Point2] The proto has no coordinate type "
                                 "set.")));
}

TYPED_TEST(Point2Protobuf, SetFromProtoOfEachVariant) {
  using P = Point2<TypeParam>;
  for (const ProtoVariant& v : ProtoVariants()) {
    SCOPED_TRACE(v.name);
    P p(7, 8);

    ASSERT_THAT(p.SetFromProto(MakeProto(v.variant, 1, 2)), IsOk());
    EXPECT_EQ(p.x(), 1);
    EXPECT_EQ(p.y(), 2);
  }
}

TYPED_TEST(Point2Protobuf, SetFromProtoNotSetLeavesPointUnchanged) {
  using P = Point2<TypeParam>;
  P p(7, 8);

  EXPECT_THAT(p.SetFromProto(Point2Proto()),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("no coordinate type set")));
  EXPECT_EQ(p, P(7, 8));
}

TEST(Point2ProtobufTypes, ToProtoInt32) {
  const Point2_i p(1, 2);
  const Point2Proto proto = p.ToProto();

  EXPECT_TRUE(proto.has_point_int32());
  EXPECT_EQ(proto.point_int32().x(), 1);
  EXPECT_EQ(proto.point_int32().y(), 2);
}

TEST(Point2ProtobufTypes, SetFromProtoInt32) {
  Point2Proto proto;
  proto.mutable_point_int32()->set_x(1);
  proto.mutable_point_int32()->set_y(2);

  Point2_i p;
  ASSERT_THAT(p.SetFromProto(proto), IsOk());

  EXPECT_EQ(p.x(), 1);
  EXPECT_EQ(p.y(), 2);
}

TEST(Point2ProtobufTypes, FromProtoInt32) {
  Point2Proto proto;
  proto.mutable_point_int32()->set_x(1);
  proto.mutable_point_int32()->set_y(-2);

  EXPECT_THAT(Point2_i::FromProto(proto), IsOkAndHolds(Point2_i(1, -2)));
}

// Values that only the point's own type holds exactly survive a round trip.
TEST(Point2ProtobufTypes, FromProtoRoundTripKeepsTypeSpecificValues) {
  EXPECT_THAT(Point2_i::FromProto(Point2_i(3, -4).ToProto()),
              IsOkAndHolds(Point2_i(3, -4)));
  EXPECT_THAT(Point2_i::FromProto(Point2_i(std::numeric_limits<int32_t>::min(),
                                           std::numeric_limits<int32_t>::max())
                                      .ToProto()),
              IsOkAndHolds(Point2_i(std::numeric_limits<int32_t>::min(),
                                    std::numeric_limits<int32_t>::max())));
  EXPECT_THAT(Point2_i64::FromProto(Point2_i64(int64_t{1} << 40, 5).ToProto()),
              IsOkAndHolds(Point2_i64(int64_t{1} << 40, 5)));
  EXPECT_THAT(Point2<float>::FromProto(Point2<float>(0.5f, -1.25f).ToProto()),
              IsOkAndHolds(Point2<float>(0.5f, -1.25f)));
  EXPECT_THAT(Point2<double>::FromProto(Point2<double>(0.1, 2.5).ToProto()),
              IsOkAndHolds(Point2<double>(0.1, 2.5)));
}

TEST(Point2ProtobufTypes, FromProtoConvertsType) {
  Point2Proto proto;
  proto.mutable_point_double()->set_x(2.0);
  proto.mutable_point_double()->set_y(-3.0);

  EXPECT_THAT(Point2_i::FromProto(proto), IsOkAndHolds(Point2_i(2, -3)));
  EXPECT_THAT(Point2<double>::FromProto(Point2_i(1, 2).ToProto()),
              IsOkAndHolds(Point2<double>(1, 2)));
}

TEST(Point2ProtobufTypes, FromProtoFractionsIntoFloatingPoint) {
  Point2Proto proto;
  proto.mutable_point_double()->set_x(0.5);
  proto.mutable_point_double()->set_y(-1.25);

  EXPECT_THAT(Point2<float>::FromProto(proto),
              IsOkAndHolds(Point2<float>(0.5f, -1.25f)));
  EXPECT_THAT(Point2<double>::FromProto(Point2<float>(0.5f, 2).ToProto()),
              IsOkAndHolds(Point2<double>(0.5, 2)));
}

TEST(Point2ProtobufErrors, FromProtoOutOfRange) {
  Point2Proto proto;
  proto.mutable_point_int64()->set_x(0);
  proto.mutable_point_int64()->set_y(int64_t{1} << 40);

  EXPECT_THAT(Point2_i::FromProto(proto),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("[Point2] y: coordinate 1099511627776 does "
                                 "not fit in int32.")));
}

TEST(Point2ProtobufErrors, FromProtoNotWholeNumber) {
  Point2Proto proto;
  proto.mutable_point_double()->set_x(1.5);
  proto.mutable_point_double()->set_y(0);

  EXPECT_THAT(Point2_i::FromProto(proto),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("[Point2] x: coordinate 1.5 is not a whole "
                                 "number")));
}

// A coordinate that is not finite is rejected by every coordinate type, and
// the error names the coordinate.
TEST(Point2ProtobufErrors, FromProtoNotFinite) {
  const auto doubles = [](double x, double y) {
    Point2Proto proto;
    proto.mutable_point_double()->set_x(x);
    proto.mutable_point_double()->set_y(y);
    return proto;
  };
  const auto floats = [](float x, float y) {
    Point2Proto proto;
    proto.mutable_point_float()->set_x(x);
    proto.mutable_point_float()->set_y(y);
    return proto;
  };
  struct Case {
    std::string name;
    Point2Proto proto;
    std::string message;
  };
  const std::vector<Case> cases = {
      {"double x infinite", doubles(std::numeric_limits<double>::infinity(), 0),
       "[Point2] x: coordinate inf is not finite."},
      {"double y NaN", doubles(0, std::numeric_limits<double>::quiet_NaN()),
       "[Point2] y: coordinate nan is not finite."},
      {"float x minus infinite",
       floats(-std::numeric_limits<float>::infinity(), 0),
       "[Point2] x: coordinate -inf is not finite."},
      {"float y NaN", floats(0, std::numeric_limits<float>::quiet_NaN()),
       "[Point2] y: coordinate nan is not finite."},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    const auto is_error =
        StatusIs(absl::StatusCode::kInvalidArgument, HasSubstr(c.message));

    EXPECT_THAT(Point2_i::FromProto(c.proto), is_error);
    EXPECT_THAT(Point2_i64::FromProto(c.proto), is_error);
    EXPECT_THAT(Point2<float>::FromProto(c.proto), is_error);
    EXPECT_THAT(Point2<double>::FromProto(c.proto), is_error);
  }
}

// A coordinate outside T's range, or a fraction for an integral T, is
// rejected, and the error names the coordinate.
TEST(Point2ProtobufErrors, FromProtoDoesNotFitInT) {
  struct Case {
    std::string name;
    absl::Status status;
    std::string message;
  };
  Point2Proto int64_x;
  int64_x.mutable_point_int64()->set_x(-(int64_t{1} << 40));
  int64_x.mutable_point_int64()->set_y(0);
  Point2Proto double_to_float;
  double_to_float.mutable_point_double()->set_x(0);
  double_to_float.mutable_point_double()->set_y(1e300);
  Point2Proto double_to_int64;
  double_to_int64.mutable_point_double()->set_x(1e19);
  double_to_int64.mutable_point_double()->set_y(0);
  Point2Proto float_to_int32;
  float_to_int32.mutable_point_float()->set_x(0);
  float_to_int32.mutable_point_float()->set_y(-0.5f);
  const std::vector<Case> cases = {
      {"int64 x below int32", Point2_i::FromProto(int64_x).status(),
       "[Point2] x: coordinate -1099511627776 does not fit in int32."},
      {"double y beyond float",
       Point2<float>::FromProto(double_to_float).status(),
       "[Point2] y: coordinate 1e+300 does not fit in float."},
      {"double x beyond int64", Point2_i64::FromProto(double_to_int64).status(),
       "[Point2] x: coordinate 1e+19 does not fit in int64."},
      {"float y fraction into int32",
       Point2_i::FromProto(float_to_int32).status(),
       "[Point2] y: coordinate -0.5 is not a whole number, so it does not "
       "fit in int32."},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);

    EXPECT_THAT(c.status, StatusIs(absl::StatusCode::kInvalidArgument,
                                   HasSubstr(c.message)));
  }
}

TEST(Point2ProtobufErrors, SetFromProtoNotSetLeavesValueUnchanged) {
  Point2_i v = Point2_i(7, 8);

  EXPECT_THAT(v.SetFromProto(Point2Proto()),
              StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_EQ(v, Point2_i(7, 8));
}

// x converts but y does not: the point keeps both old coordinates.
TEST(Point2ProtobufErrors, SetFromProtoConversionErrorLeavesValueUnchanged) {
  Point2Proto proto;
  proto.mutable_point_int64()->set_x(1);
  proto.mutable_point_int64()->set_y(int64_t{1} << 40);
  Point2_i v(7, 8);

  EXPECT_THAT(v.SetFromProto(proto),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("[Point2] y: coordinate 1099511627776 does "
                                 "not fit in int32.")));
  EXPECT_EQ(v, Point2_i(7, 8));
}

// =============================================================
// Aliases
// =============================================================

TEST(Point2Aliases, AliasTypes) {
  static_assert(std::is_same_v<Point2_i, Point2<int>>);
  static_assert(std::is_same_v<Point2_i32, Point2<int32_t>>);
  static_assert(std::is_same_v<Point2_i64, Point2<int64_t>>);
  static_assert(std::is_same_v<Point2_i, Point2_i32>);
}

// =============================================================
// Boost geometry traits
// =============================================================

TYPED_TEST(Point2BoostGeometry, Traits) {
  using T = TypeParam;
  using P = Point2<T>;
  static_assert(std::is_same_v<typename bg::tag<P>::type, bg::point_tag>);
  static_assert(bg::dimension<P>::value == 2);
  static_assert(std::is_same_v<typename bg::coordinate_type<P>::type, T>);
  static_assert(std::is_same_v<typename bg::coordinate_system<P>::type,
                               bg::cs::cartesian>);
}

TYPED_TEST(Point2BoostGeometry, Get) {
  const Point2<TypeParam> p(1, -2);

  EXPECT_EQ(bg::get<0>(p), 1);
  EXPECT_EQ(bg::get<1>(p), -2);
}

TYPED_TEST(Point2BoostGeometry, Set) {
  Point2<TypeParam> p(1, 2);
  bg::set<0>(p, 3);

  EXPECT_EQ(p, Point2<TypeParam>(3, 2));

  bg::set<1>(p, -4);

  EXPECT_EQ(p, Point2<TypeParam>(3, -4));
}

// =============================================================
// Boost polygon traits
// =============================================================

TYPED_TEST(Point2BoostPolygon, Traits) {
  using T = TypeParam;
  using P = Point2<T>;
  static_assert(std::is_same_v<typename gtl::geometry_concept<P>::type,
                               gtl::point_concept>);
  static_assert(
      std::is_same_v<typename gtl::point_traits<P>::coordinate_type, T>);
}

TYPED_TEST(Point2BoostPolygon, Get) {
  const Point2<TypeParam> p(1, -2);

  EXPECT_EQ(gtl::x(p), 1);
  EXPECT_EQ(gtl::y(p), -2);
  EXPECT_EQ(gtl::get(p, gtl::HORIZONTAL), 1);
  EXPECT_EQ(gtl::get(p, gtl::VERTICAL), -2);
}

TYPED_TEST(Point2BoostPolygon, Set) {
  using P = Point2<TypeParam>;
  P p(1, 2);
  gtl::x(p, 3);

  EXPECT_EQ(p, P(3, 2));

  gtl::y(p, -4);

  EXPECT_EQ(p, P(3, -4));

  gtl::set(p, gtl::HORIZONTAL, 5);
  gtl::set(p, gtl::VERTICAL, 6);

  EXPECT_EQ(p, P(5, 6));
}

TYPED_TEST(Point2BoostPolygon, Construct) {
  using P = Point2<TypeParam>;

  EXPECT_EQ(gtl::construct<P>(1, -2), P(1, -2));
}

}  // namespace
}  // namespace moab
