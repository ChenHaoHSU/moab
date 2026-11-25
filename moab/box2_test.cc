#include "box2.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>
#include <list>
#include <optional>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "absl/hash/hash_testing.h"
#include "absl/status/status.h"
#include "absl/status/status_matchers.h"
#include "absl/strings/str_format.h"
#include "boost/geometry.hpp"
#include "boost/polygon/polygon.hpp"
#include "moab/box2.pb.h"
#include "moab/point2.h"

// The tests follow the order of box2.h: constructors, assignment, accessors,
// distance, mutators, operations, operators, utilities, string conversion,
// hash, protobuf and the Boost traits.

namespace moab {
namespace {

namespace bg = boost::geometry;

using ::absl_testing::IsOk;
using ::absl_testing::IsOkAndHolds;
using ::absl_testing::StatusIs;
using ::testing::ElementsAre;
using ::testing::FieldsAre;
using ::testing::HasSubstr;
using ::testing::StrEq;

// The behavior common to all coordinate types runs as typed tests over these
// types, with small integer values that are exact in all of them.
using CoordinateTypes = ::testing::Types<int32_t, int64_t, float, double>;

// =============================================================
// Helpers for the protobuf tests
// =============================================================

// The proto variants, in the order Box2Proto declares them.
constexpr std::array<Box2Proto::TypeVariantCase, 4> kProtoVariants = {
    Box2Proto::kBoxInt32, Box2Proto::kBoxInt64, Box2Proto::kBoxFloat,
    Box2Proto::kBoxDouble};

// Returns the proto variant that Box2<T>::ToProto writes.
template <typename T>
constexpr Box2Proto::TypeVariantCase ProtoVariantOf() {
  if constexpr (std::is_same_v<T, int32_t>) {
    return Box2Proto::kBoxInt32;
  } else if constexpr (std::is_same_v<T, int64_t>) {
    return Box2Proto::kBoxInt64;
  } else if constexpr (std::is_same_v<T, float>) {
    return Box2Proto::kBoxFloat;
  } else {
    return Box2Proto::kBoxDouble;
  }
}

// Returns a proto of the given variant with the corners (xl yl) and (xh yh)
// written as given (not sorted).
Box2Proto MakeProto(Box2Proto::TypeVariantCase variant, int xl, int yl, int xh,
                    int yh) {
  Box2Proto proto;
  const auto set = [&](auto* box) {
    box->mutable_min_corner()->set_x(xl);
    box->mutable_min_corner()->set_y(yl);
    box->mutable_max_corner()->set_x(xh);
    box->mutable_max_corner()->set_y(yh);
  };
  switch (variant) {
    case Box2Proto::kBoxInt32:
      set(proto.mutable_box_int32());
      break;
    case Box2Proto::kBoxInt64:
      set(proto.mutable_box_int64());
      break;
    case Box2Proto::kBoxFloat:
      set(proto.mutable_box_float());
      break;
    case Box2Proto::kBoxDouble:
      set(proto.mutable_box_double());
      break;
    case Box2Proto::TYPE_VARIANT_NOT_SET:
      break;
  }
  return proto;
}

// Returns the corners stored in 'proto' as {xl, yl, xh, yh}, whatever its
// variant.
std::array<double, 4> ProtoCorners(const Box2Proto& proto) {
  const auto get = [](const auto& box) {
    return std::array<double, 4>{static_cast<double>(box.min_corner().x()),
                                 static_cast<double>(box.min_corner().y()),
                                 static_cast<double>(box.max_corner().x()),
                                 static_cast<double>(box.max_corner().y())};
  };
  switch (proto.type_variant_case()) {
    case Box2Proto::kBoxInt32:
      return get(proto.box_int32());
    case Box2Proto::kBoxInt64:
      return get(proto.box_int64());
    case Box2Proto::kBoxFloat:
      return get(proto.box_float());
    case Box2Proto::kBoxDouble:
      return get(proto.box_double());
    case Box2Proto::TYPE_VARIANT_NOT_SET:
      break;
  }
  return {};
}

std::string VariantName(Box2Proto::TypeVariantCase variant) {
  switch (variant) {
    case Box2Proto::kBoxInt32:
      return "box_int32";
    case Box2Proto::kBoxInt64:
      return "box_int64";
    case Box2Proto::kBoxFloat:
      return "box_float";
    case Box2Proto::kBoxDouble:
      return "box_double";
    case Box2Proto::TYPE_VARIANT_NOT_SET:
      break;
  }
  return "not set";
}

// =============================================================
// Constructors
// =============================================================

template <typename T>
class Box2Constructors : public ::testing::Test {};
TYPED_TEST_SUITE(Box2Constructors, CoordinateTypes);

TYPED_TEST(Box2Constructors, DefaultIsAZeroBoxAtTheOrigin) {
  using P = Point2<TypeParam>;
  Box2<TypeParam> b;

  EXPECT_EQ(b.ll(), P(0, 0));
  EXPECT_EQ(b.ur(), P(0, 0));
}

TYPED_TEST(Box2Constructors, FromTwoPoints) {
  using P = Point2<TypeParam>;
  Box2<TypeParam> b(P(1, 2), P(3, 4));

  EXPECT_EQ(b.ll(), P(1, 2));
  EXPECT_EQ(b.ur(), P(3, 4));
}

TYPED_TEST(Box2Constructors, FromFourCoordinates) {
  using P = Point2<TypeParam>;
  Box2<TypeParam> b(1, 2, 3, 4);

  EXPECT_EQ(b.ll(), P(1, 2));
  EXPECT_EQ(b.ur(), P(3, 4));
}

TYPED_TEST(Box2Constructors, SortTheEndsOfEachAxis) {
  using B = Box2<TypeParam>;
  using P = Point2<TypeParam>;
  struct Case {
    std::string name;
    B box;
  };
  const std::vector<Case> cases = {
      {"points, x swapped", B(P(3, 2), P(1, 4))},
      {"points, y swapped", B(P(1, 4), P(3, 2))},
      {"points, both swapped", B(P(3, 4), P(1, 2))},
      {"coordinates, x swapped", B(3, 2, 1, 4)},
      {"coordinates, y swapped", B(1, 4, 3, 2)},
      {"coordinates, both swapped", B(3, 4, 1, 2)},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    EXPECT_EQ(c.box.ll(), P(1, 2));
    EXPECT_EQ(c.box.ur(), P(3, 4));
  }
}

TYPED_TEST(Box2Constructors, FromProto) {
  using P = Point2<TypeParam>;
  Box2Proto proto;
  proto.mutable_box_int32()->mutable_min_corner()->set_x(1);
  proto.mutable_box_int32()->mutable_min_corner()->set_y(2);
  proto.mutable_box_int32()->mutable_max_corner()->set_x(3);
  proto.mutable_box_int32()->mutable_max_corner()->set_y(4);

  Box2<TypeParam> b(proto);

  EXPECT_EQ(b.ll(), P(1, 2));
  EXPECT_EQ(b.ur(), P(3, 4));
}

TYPED_TEST(Box2Constructors, Copy) {
  using P = Point2<TypeParam>;
  Box2<TypeParam> b1(P(1, 2), P(3, 4));
  Box2<TypeParam> b2(b1);

  EXPECT_EQ(b2.ll(), P(1, 2));
  EXPECT_EQ(b2.ur(), P(3, 4));
}

TYPED_TEST(Box2Constructors, Move) {
  using P = Point2<TypeParam>;
  Box2<TypeParam> b1(P(1, 2), P(3, 4));
  Box2<TypeParam> b2(std::move(b1));

  EXPECT_EQ(b2.ll(), P(1, 2));
  EXPECT_EQ(b2.ur(), P(3, 4));
}

// The proto constructor checks the proto with CHECK_OK, so it aborts in every
// build.
TEST(Box2ConstructorsDeathTest, ProtoWithoutVariantAborts) {
  EXPECT_DEATH(Box2_i{Box2Proto()},
               "\\[Box2\\] The proto has no coordinate type set.");
}

// =============================================================
// Assignment operators
// =============================================================

template <typename T>
class Box2AssignmentOperators : public ::testing::Test {};
TYPED_TEST_SUITE(Box2AssignmentOperators, CoordinateTypes);

TYPED_TEST(Box2AssignmentOperators, Copy) {
  using P = Point2<TypeParam>;
  Box2<TypeParam> b1(P(1, 2), P(3, 4));
  Box2<TypeParam> b2;

  b2 = b1;

  EXPECT_EQ(b2.ll(), P(1, 2));
  EXPECT_EQ(b2.ur(), P(3, 4));
}

TYPED_TEST(Box2AssignmentOperators, Move) {
  using P = Point2<TypeParam>;
  Box2<TypeParam> b1(P(1, 2), P(3, 4));
  Box2<TypeParam> b2;

  b2 = std::move(b1);

  EXPECT_EQ(b2.ll(), P(1, 2));
  EXPECT_EQ(b2.ur(), P(3, 4));
}

// =============================================================
// Accessors
// =============================================================

template <typename T>
class Box2Accessors : public ::testing::Test {};
TYPED_TEST_SUITE(Box2Accessors, CoordinateTypes);

TYPED_TEST(Box2Accessors, LlAndUrAreTheCorners) {
  using P = Point2<TypeParam>;
  Box2<TypeParam> b(P(1, 2), P(3, 4));

  EXPECT_EQ(b.ll(), P(1, 2));
  EXPECT_EQ(b.ur(), P(3, 4));
}

TYPED_TEST(Box2Accessors, MinCornerAndMaxCornerAreTheCorners) {
  using P = Point2<TypeParam>;
  Box2<TypeParam> b(P(1, 2), P(3, 4));

  EXPECT_EQ(b.MinCorner(), P(1, 2));
  EXPECT_EQ(b.MaxCorner(), P(3, 4));
}

TYPED_TEST(Box2Accessors, XlYlXhYhAreTheCoordinates) {
  using P = Point2<TypeParam>;
  Box2<TypeParam> b(P(1, 2), P(3, 4));

  EXPECT_EQ(b.xl(), 1);
  EXPECT_EQ(b.yl(), 2);
  EXPECT_EQ(b.xh(), 3);
  EXPECT_EQ(b.yh(), 4);
}

TYPED_TEST(Box2Accessors, MinXMinYMaxXMaxYAreTheCoordinates) {
  using P = Point2<TypeParam>;
  Box2<TypeParam> b(P(1, 2), P(3, 4));

  EXPECT_EQ(b.MinX(), 1);
  EXPECT_EQ(b.MinY(), 2);
  EXPECT_EQ(b.MaxX(), 3);
  EXPECT_EQ(b.MaxY(), 4);
}

TYPED_TEST(Box2Accessors, WidthAndHeight) {
  using P = Point2<TypeParam>;
  Box2<TypeParam> b(P(1, 2), P(3, 5));

  EXPECT_EQ(b.Width(), 2);
  EXPECT_EQ(b.Height(), 3);
}

TYPED_TEST(Box2Accessors, ToTupleIsXlYlXhYh) {
  using P = Point2<TypeParam>;
  Box2<TypeParam> b(P(1, 2), P(3, 4));

  EXPECT_THAT(b.ToTuple(), FieldsAre(1, 2, 3, 4));
}

TYPED_TEST(Box2Accessors, CenterOfEvenSizedBox) {
  using P = Point2<TypeParam>;
  Box2<TypeParam> b(P(1, 2), P(3, 6));

  EXPECT_EQ(b.CenterX(), 2);
  EXPECT_EQ(b.CenterY(), 4);
  EXPECT_EQ(b.Center(), P(2, 4));
}

TYPED_TEST(Box2Accessors, AreaAndPerimeters) {
  using P = Point2<TypeParam>;
  Box2<TypeParam> b(P(1, 2), P(3, 5));

  EXPECT_EQ(b.Area(), 6);
  EXPECT_EQ(b.HalfPerimeter(), 5);
  EXPECT_EQ(b.Perimeter(), 10);
}

TEST(Box2AccessorsTypeSpecific, AreaTypeIsInt64ForIntegers) {
  static_assert(std::is_same_v<Box2_i::area_type, int64_t>);
  static_assert(std::is_same_v<Box2_i64::area_type, int64_t>);
  static_assert(std::is_same_v<Box2<float>::area_type, float>);
  static_assert(std::is_same_v<Box2<double>::area_type, double>);
}

TEST(Box2AccessorsTypeSpecific, AreaDoesNotOverflowInt) {
  // 100000 x 100000 does not fit in an int.
  EXPECT_EQ(Box2_i(0, 0, 100000, 100000).Area(), int64_t{10000000000});
  EXPECT_EQ(Box2_i(-100000, -100000, 100000, 100000).Area(),
            int64_t{40000000000});
}

TEST(Box2AccessorsTypeSpecific, AreaOfFractionalBox) {
  EXPECT_DOUBLE_EQ(Box2<double>(0, 0, 1.5, 2.5).Area(), 3.75);
  EXPECT_FLOAT_EQ(Box2<float>(0, 0, 1.5f, 2.5f).Area(), 3.75f);
}

TEST(Box2AccessorsTypeSpecific, IntegerCenterRoundsTowardZero) {
  struct Case {
    std::string name;
    Box2_i box;
    Point2_i center;
  };
  const std::vector<Case> cases = {
      {"positive odd sums round down", Box2_i(10, 20, 15, 25),
       Point2_i(12, 22)},
      {"negative odd sums round up", Box2_i(-3, -3, 0, 0), Point2_i(-1, -1)},
      {"mixed signs", Box2_i(-5, -4, 0, 1), Point2_i(-2, -1)},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    EXPECT_EQ(c.box.CenterX(), c.center.x());
    EXPECT_EQ(c.box.CenterY(), c.center.y());
    EXPECT_EQ(c.box.Center(), c.center);
  }

  EXPECT_EQ(Box2_i64(10, 20, 15, 25).Center(), Point2_i64(12, 22));
  EXPECT_EQ(Box2_i64(-3, -3, 0, 0).Center(), Point2_i64(-1, -1));
}

// The sum lo + hi is computed in a wider type, so the center of a box near
// the type's limits does not overflow.
TEST(Box2AccessorsTypeSpecific, IntegerCenterDoesNotOverflow) {
  constexpr int32_t kMax32 = std::numeric_limits<int32_t>::max();
  constexpr int32_t kMin32 = std::numeric_limits<int32_t>::min();
  EXPECT_EQ(Box2_i32(kMax32 - 2, kMax32 - 4, kMax32, kMax32).Center(),
            Point2_i32(kMax32 - 1, kMax32 - 2));
  EXPECT_EQ(Box2_i32(kMin32, kMin32, kMin32 + 2, kMin32 + 4).Center(),
            Point2_i32(kMin32 + 1, kMin32 + 2));
  EXPECT_EQ(Box2_i32(kMin32, kMin32, kMax32, kMax32).Center(),
            Point2_i32(0, 0));

  constexpr int64_t kMax64 = std::numeric_limits<int64_t>::max();
  constexpr int64_t kMin64 = std::numeric_limits<int64_t>::min();
  EXPECT_EQ(Box2_i64(kMax64 - 2, kMax64 - 4, kMax64, kMax64).Center(),
            Point2_i64(kMax64 - 1, kMax64 - 2));
  EXPECT_EQ(Box2_i64(kMin64, kMin64, kMax64, kMax64).Center(),
            Point2_i64(0, 0));
}

TEST(Box2AccessorsTypeSpecific, FloatingPointCenterIsExact) {
  EXPECT_EQ(Box2<double>(10, 20, 15, 25).Center(), Point2<double>(12.5, 22.5));
  EXPECT_EQ(Box2<float>(10, 20, 15, 25).Center(), Point2<float>(12.5f, 22.5f));
  EXPECT_EQ(Box2<double>(-3, -3, 0, 0).Center(), Point2<double>(-1.5, -1.5));
}

// Width and Height check the corner order in debug builds. Boost's raw writes
// are the only way to produce an inverted box.
#ifndef NDEBUG
TEST(Box2AccessorsDeathTest, InvertedBoxSizeFailsTheOrderCheck) {
  Box2_i b;
  boost::geometry::assign_inverse(b);

  EXPECT_DEATH((void)b.Width(), "Box is inverted in x");
  EXPECT_DEATH((void)b.Height(), "Box is inverted in y");
  EXPECT_DEATH((void)b.Area(), "Box is inverted");
}
#endif

// =============================================================
// Distance
// =============================================================

template <typename T>
class Box2Distance : public ::testing::Test {};
TYPED_TEST_SUITE(Box2Distance, CoordinateTypes);

TYPED_TEST(Box2Distance, ZeroWhenOverlappingOrTouching) {
  using B = Box2<TypeParam>;
  struct Case {
    std::string name;
    B other;
  };
  const std::vector<Case> cases = {
      {"overlap", B(5, 5, 15, 15)},
      {"inside", B(2, 2, 3, 3)},
      {"edge", B(10, 0, 20, 10)},
      {"corner", B(10, 10, 20, 20)},
  };
  const B b(0, 0, 10, 10);
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    EXPECT_EQ(b.SquaredEuclideanDistance(c.other), 0);
    EXPECT_EQ(c.other.SquaredEuclideanDistance(b), 0);
    EXPECT_EQ(b.EuclideanDistance(c.other), 0.0);
  }
}

TYPED_TEST(Box2Distance, GapsAlongEachAxis) {
  using B = Box2<TypeParam>;
  struct Case {
    std::string name;
    B other;
    typename B::area_type squared;
    double distance;
  };
  const std::vector<Case> cases = {
      // The y projections overlap.
      {"gap along x only", B(15, 2, 20, 8), 25, 5.0},
      // The x projections overlap.
      {"gap along y only", B(-5, -7, 3, -3), 9, 3.0},
      // Corner to corner, dx = 3 and dy = 4.
      {"diagonal", B(13, 14, 20, 20), 25, 5.0},
      // Corner to corner on the low side, dx = 1 and dy = 2.
      {"diagonal, low side", B(-4, -5, -1, -2), 5, std::sqrt(5.0)},
  };
  const B b(0, 0, 10, 10);
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    EXPECT_EQ(b.SquaredEuclideanDistance(c.other), c.squared);
    // Symmetric.
    EXPECT_EQ(c.other.SquaredEuclideanDistance(b), c.squared);
    EXPECT_DOUBLE_EQ(b.EuclideanDistance(c.other), c.distance);
    EXPECT_DOUBLE_EQ(c.other.EuclideanDistance(b), c.distance);
  }
}

TEST(Box2DistanceTypeSpecific, SquaredEuclideanDistanceDoesNotOverflowInt) {
  const Box2_i a(-1000000000, 0, -999999999, 1);
  const Box2_i b(1000000000, 0, 1000000001, 1);

  // dx = 1999999999, which does not fit in a product of ints.
  EXPECT_EQ(a.SquaredEuclideanDistance(b), int64_t{1999999999} * 1999999999);
}

TEST(Box2DistanceTypeSpecific, EuclideanDistanceOfFractionalGaps) {
  EXPECT_DOUBLE_EQ(
      Box2<double>(0, 0, 1, 1).EuclideanDistance(Box2<double>(2.5, 3, 4, 4)),
      2.5);
}

// The closest points of two integer boxes have integer coordinates, so the
// minimum over all integer point pairs is the exact squared distance.
TEST(Box2DistanceTypeSpecific, SquaredEuclideanDistanceMatchesPointScan) {
  std::mt19937 rng(11);
  std::uniform_int_distribution<int> pos(0, 20);
  std::uniform_int_distribution<int> size(0, 5);
  for (int iter = 0; iter < 300; ++iter) {
    const int ax = pos(rng), ay = pos(rng), bx = pos(rng), by = pos(rng);
    const Box2_i a(ax, ay, ax + size(rng), ay + size(rng));
    const Box2_i b(bx, by, bx + size(rng), by + size(rng));
    int64_t best = std::numeric_limits<int64_t>::max();
    for (int x0 = a.xl(); x0 <= a.xh(); ++x0) {
      for (int y0 = a.yl(); y0 <= a.yh(); ++y0) {
        for (int x1 = b.xl(); x1 <= b.xh(); ++x1) {
          for (int y1 = b.yl(); y1 <= b.yh(); ++y1) {
            best = std::min(best, Point2_i(x0, y0).SquaredEuclideanDistance(
                                      Point2_i(x1, y1)));
          }
        }
      }
    }
    EXPECT_EQ(a.SquaredEuclideanDistance(b), best) << a << " " << b;
  }
}

// =============================================================
// Mutators
// =============================================================

template <typename T>
class Box2Mutators : public ::testing::Test {};
TYPED_TEST_SUITE(Box2Mutators, CoordinateTypes);

TYPED_TEST(Box2Mutators, SetSortsTheEnds) {
  using B = Box2<TypeParam>;
  using P = Point2<TypeParam>;
  struct Case {
    std::string name;
    B start;
    std::function<void(B&)> set;
  };
  const std::vector<Case> cases = {
      {"four coordinates", B(P(1, 2), P(3, 4)),
       [](B& b) { b.Set(5, 6, 7, 8); }},
      {"four coordinates, swapped", B(P(3, 4), P(1, 2)),
       [](B& b) { b.Set(7, 8, 5, 6); }},
      {"four coordinates, x swapped", B(P(1, 2), P(3, 4)),
       [](B& b) { b.Set(7, 6, 5, 8); }},
      {"two points", B(P(1, 2), P(3, 4)),
       [](B& b) { b.Set(P(5, 6), P(7, 8)); }},
      {"two points, swapped", B(P(1, 2), P(3, 4)),
       [](B& b) { b.Set(P(7, 8), P(5, 6)); }},
      {"two points, y swapped", B(P(1, 2), P(3, 4)),
       [](B& b) { b.Set(P(5, 8), P(7, 6)); }},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    B b = c.start;
    c.set(b);
    EXPECT_EQ(b.ll(), P(5, 6));
    EXPECT_EQ(b.ur(), P(7, 8));
  }
}

// A setter that moves a side past the opposite side swaps the two sides.
TYPED_TEST(Box2Mutators, SettersKeepTheBoxOrdered) {
  using B = Box2<TypeParam>;
  using P = Point2<TypeParam>;
  struct Case {
    std::string name;
    B start;
    std::function<void(B&)> set;
    P ll;
    P ur;
  };
  const B b1234(P(1, 2), P(3, 4));
  const B square(0, 0, 10, 10);
  const std::vector<Case> cases = {
      {"set_xl", b1234, [](B& b) { b.set_xl(0); }, P(0, 2), P(3, 4)},
      {"set_yl", b1234, [](B& b) { b.set_yl(0); }, P(1, 0), P(3, 4)},
      {"set_xh", b1234, [](B& b) { b.set_xh(5); }, P(1, 2), P(5, 4)},
      {"set_yh", b1234, [](B& b) { b.set_yh(5); }, P(1, 2), P(3, 5)},
      {"set_ll", b1234, [](B& b) { b.set_ll(P(0, 1)); }, P(0, 1), P(3, 4)},
      {"set_ur", b1234, [](B& b) { b.set_ur(P(5, 6)); }, P(1, 2), P(5, 6)},
      {"SetMinCorner", b1234, [](B& b) { b.SetMinCorner(P(0, 1)); }, P(0, 1),
       P(3, 4)},
      {"SetMaxCorner", b1234, [](B& b) { b.SetMaxCorner(P(5, 6)); }, P(1, 2),
       P(5, 6)},
      {"set_xl past xh", square, [](B& b) { b.set_xl(15); }, P(10, 0),
       P(15, 10)},
      {"set_yl past yh", square, [](B& b) { b.set_yl(15); }, P(0, 10),
       P(10, 15)},
      {"set_xh past xl", square, [](B& b) { b.set_xh(-5); }, P(-5, 0),
       P(0, 10)},
      {"set_yh past yl", square, [](B& b) { b.set_yh(-5); }, P(0, -5),
       P(10, 0)},
      {"set_ll past xh", square, [](B& b) { b.set_ll(P(15, -5)); }, P(10, -5),
       P(15, 10)},
      {"set_ur past yl", square, [](B& b) { b.set_ur(P(-5, 15)); }, P(-5, 0),
       P(0, 15)},
      {"SetMinCorner past both", square,
       [](B& b) { b.SetMinCorner(P(15, 20)); }, P(10, 10), P(15, 20)},
      {"SetMaxCorner past both", square,
       [](B& b) { b.SetMaxCorner(P(-5, -6)); }, P(-5, -6), P(0, 0)},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    B b = c.start;
    c.set(b);
    EXPECT_EQ(b.ll(), c.ll);
    EXPECT_EQ(b.ur(), c.ur);
  }
}

// =============================================================
// Operations
// =============================================================

template <typename T>
class Box2Operations : public ::testing::Test {};
TYPED_TEST_SUITE(Box2Operations, CoordinateTypes);

TYPED_TEST(Box2Operations, ShiftMovesBothCorners) {
  using B = Box2<TypeParam>;
  using P = Point2<TypeParam>;
  struct Case {
    std::string name;
    std::function<void(B&)> shift;
    P ll;
    P ur;
  };
  const std::vector<Case> cases = {
      {"Shift", [](B& b) { b.Shift(1, 2); }, P(2, 4), P(4, 6)},
      {"Shift negative", [](B& b) { b.Shift(-1, -2); }, P(0, 0), P(2, 2)},
      {"ShiftX", [](B& b) { b.ShiftX(1); }, P(2, 2), P(4, 4)},
      {"ShiftY", [](B& b) { b.ShiftY(1); }, P(1, 3), P(3, 5)},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    B b(P(1, 2), P(3, 4));
    c.shift(b);
    EXPECT_EQ(b.ll(), c.ll);
    EXPECT_EQ(b.ur(), c.ur);
  }
}

// Each step starts from the box the step before it ends with, so the second
// step of each pair undoes or reverses the first.
TYPED_TEST(Box2Operations, BloatAndShrinkMoveBothSides) {
  using B = Box2<TypeParam>;
  using P = Point2<TypeParam>;
  struct Case {
    std::string name;
    B start;
    std::function<B&(B&)> op;
    P ll;
    P ur;
  };
  const std::vector<Case> cases = {
      {"Bloat(5)", B(1, 2, 3, 4), [](B& b) -> B& { return b.Bloat(5); },
       P(-4, -3), P(8, 9)},
      {"then Bloat(-3)", B(-4, -3, 8, 9),
       [](B& b) -> B& { return b.Bloat(-3); }, P(-1, 0), P(5, 6)},
      {"Bloat(5, 6)", B(1, 2, 3, 4), [](B& b) -> B& { return b.Bloat(5, 6); },
       P(-4, -4), P(8, 10)},
      {"then Bloat(-3, -2)", B(-4, -4, 8, 10),
       [](B& b) -> B& { return b.Bloat(-3, -2); }, P(-1, -2), P(5, 8)},
      {"BloatX(5)", B(1, 2, 3, 4), [](B& b) -> B& { return b.BloatX(5); },
       P(-4, 2), P(8, 4)},
      {"then BloatX(-3)", B(-4, 2, 8, 4),
       [](B& b) -> B& { return b.BloatX(-3); }, P(-1, 2), P(5, 4)},
      {"BloatY(5)", B(1, 2, 3, 4), [](B& b) -> B& { return b.BloatY(5); },
       P(1, -3), P(3, 9)},
      {"then BloatY(-3)", B(1, -3, 3, 9),
       [](B& b) -> B& { return b.BloatY(-3); }, P(1, 0), P(3, 6)},
      {"Shrink(1)", B(0, 1, 10, 20), [](B& b) -> B& { return b.Shrink(1); },
       P(1, 2), P(9, 19)},
      {"then Shrink(-1)", B(1, 2, 9, 19),
       [](B& b) -> B& { return b.Shrink(-1); }, P(0, 1), P(10, 20)},
      {"Shrink(1, 2)", B(0, 1, 10, 20),
       [](B& b) -> B& { return b.Shrink(1, 2); }, P(1, 3), P(9, 18)},
      {"then Shrink(-1, -2)", B(1, 3, 9, 18),
       [](B& b) -> B& { return b.Shrink(-1, -2); }, P(0, 1), P(10, 20)},
      {"ShrinkX(1)", B(0, 1, 10, 20), [](B& b) -> B& { return b.ShrinkX(1); },
       P(1, 1), P(9, 20)},
      {"then ShrinkX(-1)", B(1, 1, 9, 20),
       [](B& b) -> B& { return b.ShrinkX(-1); }, P(0, 1), P(10, 20)},
      {"ShrinkY(1)", B(0, 1, 10, 20), [](B& b) -> B& { return b.ShrinkY(1); },
       P(0, 2), P(10, 19)},
      {"then ShrinkY(-1)", B(0, 2, 10, 19),
       [](B& b) -> B& { return b.ShrinkY(-1); }, P(0, 1), P(10, 20)},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    B b = c.start;
    // Bloat and Shrink return the box itself.
    EXPECT_EQ(&c.op(b), &b);
    EXPECT_EQ(b.ll(), c.ll);
    EXPECT_EQ(b.ur(), c.ur);
  }
}

TYPED_TEST(Box2Operations, ShrinkPastZeroCollapsesToCenter) {
  using B = Box2<TypeParam>;
  B b(0, 0, 10, 20);

  b.Shrink(6);  // x is 10 wide and collapses to 5; y shrinks to [6, 14].

  EXPECT_EQ(b, B(5, 6, 5, 14));
  EXPECT_EQ(b.Width(), 0);
  EXPECT_EQ(b.Area(), 0);
}

TYPED_TEST(Box2Operations, AxisShrunkToOrPastZeroCollapses) {
  using B = Box2<TypeParam>;
  struct Case {
    std::string name;
    B start;
    std::function<void(B&)> op;
    B expected;
  };
  const std::vector<Case> cases = {
      {"Shrink exactly to zero", B(0, 0, 10, 10), [](B& b) { b.Shrink(5); },
       B(5, 5, 5, 5)},
      {"Bloat negative past zero", B(0, 0, 10, 10),
       [](B& b) { b.Bloat(-6, 0); }, B(5, 0, 5, 10)},
      {"ShrinkY past zero", B(0, 2, 10, 6), [](B& b) { b.ShrinkY(3); },
       B(0, 4, 10, 4)},
      {"zero-width box shrunk", B(3, 0, 3, 10), [](B& b) { b.ShrinkX(1); },
       B(3, 0, 3, 10)},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    B b = c.start;
    c.op(b);
    EXPECT_EQ(b, c.expected);
  }
}

// Encompass returns true only if the box grew.
TYPED_TEST(Box2Operations, EncompassPoint) {
  using B = Box2<TypeParam>;
  using P = Point2<TypeParam>;
  struct Case {
    std::string name;
    P point;
    bool grew;
    P ll;
    P ur;
  };
  const std::vector<Case> cases = {
      {"below and left", P(0, 1), true, P(0, 1), P(3, 4)},
      {"the min corner", P(1, 2), false, P(1, 2), P(3, 4)},
      {"inside", P(2, 3), false, P(1, 2), P(3, 4)},
      {"right and below", P(100, -100), true, P(1, -100), P(100, 4)},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    B b(P(1, 2), P(3, 4));
    EXPECT_EQ(b.Encompass(c.point), c.grew);
    EXPECT_EQ(b.ll(), c.ll);
    EXPECT_EQ(b.ur(), c.ur);
  }
}

TYPED_TEST(Box2Operations, EncompassBox) {
  using B = Box2<TypeParam>;
  using P = Point2<TypeParam>;
  struct Case {
    std::string name;
    B other;
    bool grew;
    P ll;
    P ur;
  };
  const std::vector<Case> cases = {
      {"containing box", B(P(0, 1), P(5, 6)), true, P(0, 1), P(5, 6)},
      {"equal box", B(P(1, 2), P(3, 4)), false, P(1, 2), P(3, 4)},
      {"box inside", B(P(2, 2), P(3, 3)), false, P(1, 2), P(3, 4)},
      {"distant box", B(P(100, -100), P(200, -50)), true, P(1, -100),
       P(200, 4)},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    B b(P(1, 2), P(3, 4));
    EXPECT_EQ(b.Encompass(c.other), c.grew);
    EXPECT_EQ(b.ll(), c.ll);
    EXPECT_EQ(b.ur(), c.ur);
  }
}

// The collapse point lo + (hi - lo) / 2 rounds toward lo for integers.
TEST(Box2OperationsTypeSpecific, IntegerShrinkPastZeroRoundsTowardLo) {
  Box2_i b(0, 0, 9, 9);
  b.ShrinkX(5);
  EXPECT_EQ(b, Box2_i(4, 0, 4, 9));

  Box2_i64 b64(0, 0, 9, 9);
  b64.ShrinkY(5);
  EXPECT_EQ(b64, Box2_i64(0, 4, 9, 4));
}

TEST(Box2OperationsTypeSpecific, IntegerShrinkPastZeroNegativeCoordinates) {
  Box2_i b(-10, -10, -1, -1);

  b.Shrink(5);  // -10 + 9 / 2 = -6.

  EXPECT_EQ(b, Box2_i(-6, -6, -6, -6));
}

TEST(Box2OperationsTypeSpecific, FloatingPointShrinkPastZeroIsExactCenter) {
  Box2<double> b(0, 0, 3, 1);
  b.ShrinkX(2);
  EXPECT_EQ(b, Box2<double>(1.5, 0, 1.5, 1));

  Box2<float> f(-10, -10, -1, -1);
  f.Shrink(5);
  EXPECT_EQ(f, Box2<float>(-5.5f, -5.5f, -5.5f, -5.5f));
}

// =============================================================
// Operators
// =============================================================

template <typename T>
class Box2Operators : public ::testing::Test {};
TYPED_TEST_SUITE(Box2Operators, CoordinateTypes);

TYPED_TEST(Box2Operators, SubscriptReturnsTheCorners) {
  using P = Point2<TypeParam>;
  Box2<TypeParam> b(P(1, 2), P(3, 4));

  EXPECT_EQ(b[0], P(1, 2));
  EXPECT_EQ(b[1], P(3, 4));
}

// The corners are read-only outside the setters, so a box can't be inverted
// through them.
TYPED_TEST(Box2Operators, CornerAccessorsAreReadOnly) {
  using B = Box2<TypeParam>;
  using P = Point2<TypeParam>;
  static_assert(std::is_same_v<decltype(std::declval<B&>()[0]), const P&>);
  static_assert(std::is_same_v<decltype(std::declval<B&>().ll()), const P&>);
  static_assert(std::is_same_v<decltype(std::declval<B&>().ur()), const P&>);
  static_assert(
      std::is_same_v<decltype(std::declval<B&>().MinCorner()), const P&>);
  static_assert(
      std::is_same_v<decltype(std::declval<B&>().MaxCorner()), const P&>);
}

TYPED_TEST(Box2Operators, EqualityComparesBothCorners) {
  using B = Box2<TypeParam>;
  using P = Point2<TypeParam>;
  struct Case {
    std::string name;
    B other;
    bool equal;
  };
  const std::vector<Case> cases = {
      {"same corners", B(P(1, 2), P(3, 4)), true},
      {"same corners, given swapped", B(P(3, 4), P(1, 2)), true},
      {"different corners", B(P(5, 6), P(7, 8)), false},
      {"xl differs", B(P(0, 2), P(3, 4)), false},
      {"yl differs", B(P(1, 1), P(3, 4)), false},
      {"xh differs", B(P(1, 2), P(4, 4)), false},
      {"yh differs", B(P(1, 2), P(3, 5)), false},
  };
  const B b(P(1, 2), P(3, 4));
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    EXPECT_EQ(b == c.other, c.equal);
    EXPECT_EQ(b != c.other, !c.equal);
    EXPECT_EQ(c.other == b, c.equal);
    EXPECT_EQ(c.other != b, !c.equal);
    if (c.equal) {
      EXPECT_EQ(b, c.other);
    } else {
      EXPECT_NE(b, c.other);
    }
  }
}

// operator< orders by the min corner, then by the max corner; points compare
// by x, then by y.
TYPED_TEST(Box2Operators, OrderIsLexicographic) {
  using B = Box2<TypeParam>;
  using P = Point2<TypeParam>;
  struct Case {
    std::string name;
    B a;
    B b;
    bool a_less;
    bool b_less;
  };
  const std::vector<Case> cases = {
      {"both corners smaller", B(P(1, 2), P(3, 4)), B(P(3, 4), P(5, 6)), true,
       false},
      {"equal", B(P(1, 2), P(3, 4)), B(P(1, 2), P(3, 4)), false, false},
      {"min corner decides over max corner", B(0, 0, 9, 9), B(1, 0, 2, 2), true,
       false},
      {"min corner y decides", B(0, 1, 2, 2), B(0, 0, 5, 5), false, true},
      {"equal min corners, max corner x decides", B(0, 0, 1, 5), B(0, 0, 2, 1),
       true, false},
      {"equal min corners, max corner y decides", B(0, 0, 2, 2), B(0, 0, 2, 1),
       false, true},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    EXPECT_EQ(c.a < c.b, c.a_less);
    EXPECT_EQ(c.b > c.a, c.a_less);
    EXPECT_EQ(c.b < c.a, c.b_less);
    EXPECT_EQ(c.a > c.b, c.b_less);
    EXPECT_EQ(c.a <= c.b, !c.b_less);
    EXPECT_EQ(c.b >= c.a, !c.b_less);
    EXPECT_EQ(c.a >= c.b, !c.a_less);
    EXPECT_EQ(c.b <= c.a, !c.a_less);
  }
}

// operator[] checks the index with DCHECK, so only debug builds abort.
#ifndef NDEBUG
TEST(Box2OperatorsDeathTest, SubscriptOutOfRange) {
  Box2_i v(0, 0, 1, 1);
  const Box2_i& const_v = v;

  EXPECT_DEATH((void)v[2], "Check failed: i < d_.size");
  EXPECT_DEATH((void)const_v[2], "Check failed: i < d_.size");
}
#endif

// =============================================================
// Utilities
// =============================================================

template <typename T>
class Box2BoundingBox : public ::testing::Test {};
TYPED_TEST_SUITE(Box2BoundingBox, CoordinateTypes);

TYPED_TEST(Box2BoundingBox, Vector) {
  using B = Box2<TypeParam>;
  using P = Point2<TypeParam>;
  struct Case {
    std::string name;
    std::vector<P> points;
    P ll;
    P ur;
  };
  const std::vector<Case> cases = {
      {"one point", {P(1, 2)}, P(1, 2), P(1, 2)},
      {"four points", {P(1, 2), P(3, 4), P(-1, 5), P(0, 0)}, P(-1, 0), P(3, 5)},
      {"repeated point",
       {P(1, 2), P(1, 2), P(1, 2), P(1, 2)},
       P(1, 2),
       P(1, 2)},
      {"points around the origin",
       {P(-10, -20), P(10, 20), P(-5, 5), P(5, -5), P(0, 0)},
       P(-10, -20),
       P(10, 20)},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    std::optional<B> box = B::BoundingBox(c.points);
    ASSERT_TRUE(box.has_value());
    EXPECT_EQ(box->ll(), c.ll);
    EXPECT_EQ(box->ur(), c.ur);
  }
}

TYPED_TEST(Box2BoundingBox, OtherContainers) {
  using B = Box2<TypeParam>;
  using P = Point2<TypeParam>;
  const std::array<P, 3> array = {P(1, 2), P(3, 4), P(-1, 5)};
  const std::list<P> list = {P(1, 2), P(3, 4), P(-1, 5)};
  const std::set<P> set = {P(1, 2), P(3, 4), P(-1, 5)};
  struct Case {
    std::string name;
    std::optional<B> box;
  };
  const std::vector<Case> cases = {
      {"array", B::BoundingBox(array)},
      {"initializer_list", B::BoundingBox({P(1, 2), P(3, 4), P(-1, 5)})},
      {"list", B::BoundingBox(list)},
      {"set", B::BoundingBox(set)},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    ASSERT_TRUE(c.box.has_value());
    EXPECT_EQ(c.box->ll(), P(-1, 2));
    EXPECT_EQ(c.box->ur(), P(3, 5));
  }
}

TYPED_TEST(Box2BoundingBox, EmptyContainerHasNone) {
  using B = Box2<TypeParam>;
  using P = Point2<TypeParam>;

  EXPECT_EQ(B::BoundingBox(std::vector<P>()), std::nullopt);
  EXPECT_EQ(B::BoundingBox(std::array<P, 0>{}), std::nullopt);
  EXPECT_EQ(B::BoundingBox({}), std::nullopt);
  EXPECT_EQ(B::BoundingBox(std::list<P>()), std::nullopt);
  EXPECT_EQ(B::BoundingBox(std::set<P>()), std::nullopt);
}

// =============================================================
// String conversion
// =============================================================

template <typename T>
class Box2StringConversion : public ::testing::Test {};
TYPED_TEST_SUITE(Box2StringConversion, CoordinateTypes);

TYPED_TEST(Box2StringConversion, ToString) {
  using P = Point2<TypeParam>;

  EXPECT_THAT(Box2<TypeParam>(P(1, 2), P(3, 4)).ToString(),
              StrEq("((1 2) (3 4))"));
  EXPECT_THAT(Box2<TypeParam>(-1, -2, 3, 4).ToString(),
              StrEq("((-1 -2) (3 4))"));
}

TYPED_TEST(Box2StringConversion, SupportsAbslStringify) {
  using P = Point2<TypeParam>;
  Box2<TypeParam> b(P(1, 2), P(3, 4));
  std::string s = absl::StrFormat("%v", b);

  EXPECT_THAT(s, StrEq("((1 2) (3 4))"));
}

TYPED_TEST(Box2StringConversion, OstreamOperator) {
  std::ostringstream os;

  os << Box2<TypeParam>(1, 2, 3, 4) << ";";

  EXPECT_THAT(os.str(), StrEq("((1 2) (3 4));"));
}

TEST(Box2StringConversionTypeSpecific, FractionalCoordinates) {
  EXPECT_THAT(Box2<double>(0.5, 1, 2, 3.5).ToString(),
              StrEq("((0.5 1) (2 3.5))"));
  EXPECT_THAT(Box2<float>(0.5f, 1, 2, 3.5f).ToString(),
              StrEq("((0.5 1) (2 3.5))"));
  EXPECT_THAT(absl::StrFormat("%v", Box2<float>(-0.25f, 0, 0, 0.75f)),
              StrEq("((-0.25 0) (0 0.75))"));
}

TEST(Box2StringConversionTypeSpecific, Int64BeyondInt32) {
  const int64_t big = int64_t{1} << 40;  // 1099511627776

  EXPECT_THAT(Box2_i64(-big, 0, big, 1).ToString(),
              StrEq("((-1099511627776 0) (1099511627776 1))"));
}

// =============================================================
// Hash
// =============================================================

template <typename T>
class Box2Hash : public ::testing::Test {};
TYPED_TEST_SUITE(Box2Hash, CoordinateTypes);

TYPED_TEST(Box2Hash, SupportsAbslHash) {
  using B = Box2<TypeParam>;
  using P = Point2<TypeParam>;

  EXPECT_TRUE(absl::VerifyTypeImplementsAbslHashCorrectly({
      B(),
      B(1, 2, 3, 4),
      B(1, 2, 3, 4),
      B(P(1, 2), P(3, 4)),
      B(P(4, 5), P(7, 6)),
      B(1, 2, 4, 3),
  }));
}

// =============================================================
// Protobuf
// =============================================================

template <typename T>
class Box2Protobuf : public ::testing::Test {};
TYPED_TEST_SUITE(Box2Protobuf, CoordinateTypes);

TYPED_TEST(Box2Protobuf, ToProtoWritesTheVariantOfT) {
  using P = Point2<TypeParam>;
  Box2<TypeParam> b(P(1, 2), P(3, 4));

  Box2Proto proto = b.ToProto();

  EXPECT_EQ(proto.type_variant_case(), ProtoVariantOf<TypeParam>());
  EXPECT_THAT(ProtoCorners(proto), ElementsAre(1, 2, 3, 4));
  EXPECT_THAT(ProtoCorners(Box2<TypeParam>(-1, -2, 0, 0).ToProto()),
              ElementsAre(-1, -2, 0, 0));
}

// Every T reads every proto variant whose values fit in T.
TYPED_TEST(Box2Protobuf, FromProtoReadsEveryVariant) {
  using B = Box2<TypeParam>;
  for (Box2Proto::TypeVariantCase variant : kProtoVariants) {
    SCOPED_TRACE(VariantName(variant));
    EXPECT_THAT(B::FromProto(MakeProto(variant, 1, 2, 3, 4)),
                IsOkAndHolds(B(1, 2, 3, 4)));
    EXPECT_THAT(B::FromProto(MakeProto(variant, -1, -2, 0, 0)),
                IsOkAndHolds(B(-1, -2, 0, 0)));
  }
}

TYPED_TEST(Box2Protobuf, SetFromProtoReadsEveryVariant) {
  using P = Point2<TypeParam>;
  for (Box2Proto::TypeVariantCase variant : kProtoVariants) {
    SCOPED_TRACE(VariantName(variant));
    Box2<TypeParam> b;
    ASSERT_THAT(b.SetFromProto(MakeProto(variant, 1, 2, 3, 4)), IsOk());

    EXPECT_EQ(b.ll(), P(1, 2));
    EXPECT_EQ(b.ur(), P(3, 4));
  }
}

TYPED_TEST(Box2Protobuf, ProtoConstructorReadsEveryVariant) {
  using B = Box2<TypeParam>;
  for (Box2Proto::TypeVariantCase variant : kProtoVariants) {
    SCOPED_TRACE(VariantName(variant));
    EXPECT_EQ(B(MakeProto(variant, 1, 2, 3, 4)), B(1, 2, 3, 4));
  }
}

TYPED_TEST(Box2Protobuf, FromProtoRoundTrip) {
  using B = Box2<TypeParam>;

  EXPECT_THAT(B::FromProto(B(-1, 2, 3, 4).ToProto()),
              IsOkAndHolds(B(-1, 2, 3, 4)));
}

// A proto may hold its corners in any order; FromProto sorts them.
TYPED_TEST(Box2Protobuf, FromProtoNormalizesCorners) {
  using B = Box2<TypeParam>;
  for (Box2Proto::TypeVariantCase variant : kProtoVariants) {
    SCOPED_TRACE(VariantName(variant));
    EXPECT_THAT(B::FromProto(MakeProto(variant, 10, 20, 0, 5)),
                IsOkAndHolds(B(0, 5, 10, 20)));
  }
}

TYPED_TEST(Box2Protobuf, FromProtoWithoutVariantFails) {
  EXPECT_THAT(Box2<TypeParam>::FromProto(Box2Proto()),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("[Box2] The proto has no coordinate type "
                                 "set.")));
}

TYPED_TEST(Box2Protobuf, SetFromProtoErrorLeavesValueUnchanged) {
  using B = Box2<TypeParam>;
  B v = B(1, 2, 3, 4);

  EXPECT_THAT(v.SetFromProto(Box2Proto()),
              StatusIs(absl::StatusCode::kInvalidArgument));
  EXPECT_EQ(v, B(1, 2, 3, 4));
}

TEST(Box2ProtobufTypeSpecific, RoundTripInt64BeyondInt32) {
  const int64_t big = int64_t{1} << 40;
  Box2_i64 b(-big, 0, big, big + 1);

  Box2Proto proto = b.ToProto();

  ASSERT_TRUE(proto.has_box_int64());
  EXPECT_EQ(proto.box_int64().min_corner().x(), -big);
  EXPECT_EQ(proto.box_int64().min_corner().y(), 0);
  EXPECT_EQ(proto.box_int64().max_corner().x(), big);
  EXPECT_EQ(proto.box_int64().max_corner().y(), big + 1);
  EXPECT_EQ(Box2_i64(proto), b);
  EXPECT_THAT(Box2_i64::FromProto(
                  Box2_i64(0, 0, int64_t{1} << 40, int64_t{1} << 41).ToProto()),
              IsOkAndHolds(Box2_i64(0, 0, int64_t{1} << 40, int64_t{1} << 41)));
}

TEST(Box2ProtobufTypeSpecific, RoundTripFractionalFloat) {
  Box2<float> b(0.5f, 1.25f, 2.0f, 3.0f);

  Box2Proto proto = b.ToProto();

  ASSERT_TRUE(proto.has_box_float());
  EXPECT_FLOAT_EQ(proto.box_float().min_corner().x(), 0.5f);
  EXPECT_FLOAT_EQ(proto.box_float().max_corner().y(), 3.0f);
  EXPECT_EQ(Box2<float>(proto), b);
}

TEST(Box2ProtobufTypeSpecific, RoundTripFractionalDouble) {
  Box2<double> b(-0.001, 0.25, 12.5, 7.75);

  Box2Proto proto = b.ToProto();

  ASSERT_TRUE(proto.has_box_double());
  EXPECT_DOUBLE_EQ(proto.box_double().min_corner().x(), -0.001);
  EXPECT_DOUBLE_EQ(proto.box_double().min_corner().y(), 0.25);
  EXPECT_DOUBLE_EQ(proto.box_double().max_corner().x(), 12.5);
  EXPECT_DOUBLE_EQ(proto.box_double().max_corner().y(), 7.75);
  EXPECT_EQ(Box2<double>(proto), b);
  EXPECT_THAT(Box2<double>::FromProto(Box2<double>(0.5, 1, 2, 3.5).ToProto()),
              IsOkAndHolds(Box2<double>(0.5, 1, 2, 3.5)));
}

TEST(Box2ProtobufErrors, OutOfRangeForInt32) {
  Box2Proto proto;
  proto.mutable_box_double()->mutable_max_corner()->set_y(3e9);

  EXPECT_THAT(Box2_i::FromProto(proto),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("[Box2] max_corner.y: coordinate")));
}

TEST(Box2ProtobufErrors, MessageNamesTheBadCoordinate) {
  struct Case {
    std::string name;
    std::function<void(Box2ProtoDouble*)> set;
    std::string message;
  };
  const std::vector<Case> cases = {
      {"min_corner.x",
       [](Box2ProtoDouble* p) { p->mutable_min_corner()->set_x(0.5); },
       "[Box2] min_corner.x: coordinate 0.5 is not a whole number"},
      {"min_corner.y",
       [](Box2ProtoDouble* p) { p->mutable_min_corner()->set_y(0.5); },
       "[Box2] min_corner.y: coordinate 0.5 is not a whole number"},
      {"max_corner.x",
       [](Box2ProtoDouble* p) { p->mutable_max_corner()->set_x(0.5); },
       "[Box2] max_corner.x: coordinate 0.5 is not a whole number"},
      {"max_corner.y",
       [](Box2ProtoDouble* p) { p->mutable_max_corner()->set_y(0.5); },
       "[Box2] max_corner.y: coordinate 0.5 is not a whole number"},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    Box2Proto proto;
    c.set(proto.mutable_box_double());
    EXPECT_THAT(
        Box2_i::FromProto(proto),
        StatusIs(absl::StatusCode::kInvalidArgument, HasSubstr(c.message)));
    EXPECT_THAT(
        Box2_i64::FromProto(proto),
        StatusIs(absl::StatusCode::kInvalidArgument, HasSubstr(c.message)));
  }
}

TEST(Box2ProtobufErrors, NotFinite) {
  Box2Proto proto;
  proto.mutable_box_double()->mutable_min_corner()->set_x(
      std::numeric_limits<double>::infinity());
  Box2Proto nan_proto;
  nan_proto.mutable_box_float()->mutable_max_corner()->set_x(
      std::numeric_limits<float>::quiet_NaN());

  EXPECT_THAT(Box2<double>::FromProto(proto),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("[Box2] min_corner.x: coordinate inf is not "
                                 "finite.")));
  EXPECT_THAT(Box2<float>::FromProto(nan_proto),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("[Box2] max_corner.x: coordinate nan is not "
                                 "finite.")));
}

TEST(Box2ProtobufErrors, Int64DoesNotFitInInt32) {
  Box2Proto proto;
  proto.mutable_box_int64()->mutable_min_corner()->set_y(int64_t{1} << 40);

  EXPECT_THAT(Box2_i::FromProto(proto),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("[Box2] min_corner.y: coordinate "
                                 "1099511627776 does not fit in int32.")));
  // int64 and the floating-point types hold it.
  EXPECT_THAT(Box2_i64::FromProto(proto),
              IsOkAndHolds(Box2_i64(0, 0, 0, int64_t{1} << 40)));
  EXPECT_THAT(Box2<double>::FromProto(proto),
              IsOkAndHolds(Box2<double>(0, 0, 0, 1099511627776.0)));
}

TEST(Box2ProtobufErrors, DoubleDoesNotFitInFloat) {
  Box2Proto proto;
  proto.mutable_box_double()->mutable_max_corner()->set_x(1e300);

  EXPECT_THAT(Box2<float>::FromProto(proto),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("[Box2] max_corner.x: coordinate 1e+300 does "
                                 "not fit in float.")));
}

TEST(Box2ProtobufErrors, SetFromProtoErrorLeavesValueUnchanged) {
  Box2Proto proto;
  proto.mutable_box_float()->mutable_max_corner()->set_x(0.5f);
  Box2_i v(1, 2, 3, 4);

  EXPECT_THAT(v.SetFromProto(proto),
              StatusIs(absl::StatusCode::kInvalidArgument,
                       HasSubstr("[Box2] max_corner.x")));
  EXPECT_EQ(v, Box2_i(1, 2, 3, 4));
}

// =============================================================
// Boost.Geometry traits
// =============================================================

// Boost writes box coordinates one at a time, and Boost.Geometry starts some
// algorithms from an inside-out box. These cases fail if a write ever reorders
// the ends of an axis.
template <typename T>
class Box2BoostGeometry : public ::testing::Test {};
TYPED_TEST_SUITE(Box2BoostGeometry, CoordinateTypes);

TYPED_TEST(Box2BoostGeometry, IndexedAccessGetsAndSetsCoordinates) {
  using B = Box2<TypeParam>;
  B b(1, 2, 3, 4);

  EXPECT_EQ((bg::get<bg::min_corner, 0>(b)), 1);
  EXPECT_EQ((bg::get<bg::min_corner, 1>(b)), 2);
  EXPECT_EQ((bg::get<bg::max_corner, 0>(b)), 3);
  EXPECT_EQ((bg::get<bg::max_corner, 1>(b)), 4);

  bg::set<bg::min_corner, 0>(b, 0);
  bg::set<bg::min_corner, 1>(b, 1);
  bg::set<bg::max_corner, 0>(b, 5);
  bg::set<bg::max_corner, 1>(b, 6);

  EXPECT_EQ(b, B(0, 1, 5, 6));
}

TYPED_TEST(Box2BoostGeometry, AssignInverseThenExpand) {
  using B = Box2<TypeParam>;
  using P = Point2<TypeParam>;
  B b;

  bg::assign_inverse(b);
  bg::expand(b, P(50, 60));
  bg::expand(b, P(70, 90));

  EXPECT_EQ(b, B(50, 60, 70, 90));
}

TYPED_TEST(Box2BoostGeometry, Convert) {
  using B = Box2<TypeParam>;
  B b;

  bg::convert(B(100, 100, 200, 300), b);

  EXPECT_EQ(b, B(100, 100, 200, 300));
}

// =============================================================
// Boost.Polygon traits
// =============================================================

template <typename T>
class Box2BoostPolygon : public ::testing::Test {};
TYPED_TEST_SUITE(Box2BoostPolygon, CoordinateTypes);

TYPED_TEST(Box2BoostPolygon, GettersReadTheIntervals) {
  Box2<TypeParam> b(1, 2, 3, 4);

  EXPECT_EQ(gtl::xl(b), 1);
  EXPECT_EQ(gtl::yl(b), 2);
  EXPECT_EQ(gtl::xh(b), 3);
  EXPECT_EQ(gtl::yh(b), 4);
  EXPECT_EQ(gtl::low(gtl::horizontal(b)), 1);
  EXPECT_EQ(gtl::high(gtl::horizontal(b)), 3);
  EXPECT_EQ(gtl::low(gtl::vertical(b)), 2);
  EXPECT_EQ(gtl::high(gtl::vertical(b)), 4);
}

TYPED_TEST(Box2BoostPolygon, SettersWriteTheIntervals) {
  using B = Box2<TypeParam>;
  B b(1, 2, 3, 4);

  gtl::set(b, gtl::HORIZONTAL, gtl::interval_data<TypeParam>(5, 7));
  EXPECT_EQ(b, B(5, 2, 7, 4));

  gtl::set(b, gtl::VERTICAL, gtl::interval_data<TypeParam>(-1, 0));
  EXPECT_EQ(b, B(5, -1, 7, 0));
}

TYPED_TEST(Box2BoostPolygon, Construct) {
  using B = Box2<TypeParam>;

  EXPECT_EQ(gtl::construct<B>(gtl::interval_data<TypeParam>(1, 3),
                              gtl::interval_data<TypeParam>(2, 4)),
            B(1, 2, 3, 4));
  EXPECT_EQ(
      gtl::construct<B>(TypeParam{1}, TypeParam{2}, TypeParam{3}, TypeParam{4}),
      B(1, 2, 3, 4));
}

TYPED_TEST(Box2BoostPolygon, Move) {
  using B = Box2<TypeParam>;
  B b(0, 0, 10, 10);

  gtl::move(b, gtl::HORIZONTAL, 20);

  EXPECT_EQ(b, B(20, 0, 30, 10));
}

TYPED_TEST(Box2BoostPolygon, Convolve) {
  using B = Box2<TypeParam>;
  B b(0, 0, 10, 10);

  gtl::convolve(b, Point2<TypeParam>(100, 0));

  EXPECT_EQ(b, B(100, 0, 110, 10));
}

// The value-type operations are constexpr, so tech constants can be built at
// compile time.
constexpr Box2<int> BloatedAndShifted() {
  Box2<int> b(10, 10, 0, 0);
  b.Bloat(2, 1);
  b.Shift(1, 1);
  b.set_xh(20);
  return b;
}

TEST(Box2Constexpr, OperationsRunAtCompileTime) {
  static_assert(Box2<int>(10, 10, 0, 0).ll() == Point2<int>(0, 0));
  static_assert(BloatedAndShifted() == Box2<int>(-1, 0, 20, 12));
  static_assert(Box2<int>(0, 0, 4, 3).Area() == 12);
  static_assert(Box2<int>(0, 0, 4, 3).Perimeter() == 14);
  static_assert(Box2<int>(-3, -3, 0, 0).Center() == Point2<int>(-1, -1));
  static_assert(Box2<int64_t>(0, 0, 4, 2).Center() == Point2<int64_t>(2, 1));
  static_assert(Box2<int>(0, 0, 2, 2).Shrink(5) == Box2<int>(1, 1, 1, 1));
  static_assert(
      Box2<int>(0, 0, 1, 1).SquaredEuclideanDistance(Box2<int>(4, 5, 6, 6)) ==
      25);
  static_assert(std::get<3>(Box2<int>(0, 0, 4, 3).ToTuple()) == 3);
}

}  // namespace
}  // namespace moab
