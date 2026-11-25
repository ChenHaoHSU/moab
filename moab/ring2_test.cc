#include "ring2.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "absl/hash/hash.h"
#include "absl/hash/hash_testing.h"
#include "absl/strings/str_format.h"
#include "boost/geometry.hpp"
#include "boost/polygon/polygon.hpp"
#include "moab/box2.h"
#include "moab/operation.h"
#include "moab/point2.h"

// The tests follow the order of ring2.h.
//
// Everything common to all coordinate types is a typed test over int32_t,
// int64_t, float and double, with small integer values that are exact in every
// type. Area, MaxBoxes, BloatedRings and the Boost.Polygon traits go through
// Boost.Polygon, which compiles and gives the same results for all four types
// here. Integer rounding (Area, Centroid), int overflow and fractional printing
// are plain tests.

namespace moab {
namespace {

using ::testing::ElementsAre;
using ::testing::IsEmpty;
using ::testing::StrEq;
using ::testing::UnorderedElementsAre;
using ::testing::UnorderedElementsAreArray;

using CoordinateTypes = ::testing::Types<int32_t, int64_t, float, double>;

// Returns the ring with the given integer (x, y) points, in type T.
template <typename T>
Ring2<T> MakeRing(std::initializer_list<std::pair<int, int>> xy) {
  Ring2<T> r;
  for (const auto& [x, y] : xy) r.Append(Point2<T>(x, y));
  return r;
}

// Returns the points of the square (0 0)-(2 2), counterclockwise and closed.
template <typename T>
std::vector<Point2<T>> SquarePoints() {
  return {Point2<T>(0, 0), Point2<T>(2, 0), Point2<T>(2, 2), Point2<T>(0, 2),
          Point2<T>(0, 0)};
}

// The L-shape: the square (0 0)-(4 4) without its top left quarter. Its area
// is 2 * 2 + 2 * 4 = 12 and its centroid (7/3, 5/3).
template <typename T>
Ring2<T> LShape() {
  return MakeRing<T>({{0, 0}, {4, 0}, {4, 4}, {2, 4}, {2, 2}, {0, 2}, {0, 0}});
}

// The L-shape scaled by 10: area 20 * 20 + 20 * 40 = 1200, centroid
// (70/3, 50/3).
template <typename T>
Ring2<T> BigLShape() {
  return MakeRing<T>(
      {{0, 0}, {40, 0}, {40, 40}, {20, 40}, {20, 20}, {0, 20}, {0, 0}});
}

// A U-shape: the square (0 0)-(40 40) without the notch (10 10)-(30 40). Its
// centroid (20, 17) lies in the notch, outside the ring.
template <typename T>
Ring2<T> UShape() {
  return MakeRing<T>({{0, 0},
                      {40, 0},
                      {40, 40},
                      {30, 40},
                      {30, 10},
                      {10, 10},
                      {10, 40},
                      {0, 40},
                      {0, 0}});
}

template <typename T>
class Ring2Constructor : public ::testing::Test {};
TYPED_TEST_SUITE(Ring2Constructor, CoordinateTypes);

template <typename T>
class Ring2Accessor : public ::testing::Test {};
TYPED_TEST_SUITE(Ring2Accessor, CoordinateTypes);

template <typename T>
class Ring2Mutator : public ::testing::Test {};
TYPED_TEST_SUITE(Ring2Mutator, CoordinateTypes);

template <typename T>
class Ring2Operation : public ::testing::Test {};
TYPED_TEST_SUITE(Ring2Operation, CoordinateTypes);

template <typename T>
class Ring2Iterator : public ::testing::Test {};
TYPED_TEST_SUITE(Ring2Iterator, CoordinateTypes);

template <typename T>
class Ring2Operator : public ::testing::Test {};
TYPED_TEST_SUITE(Ring2Operator, CoordinateTypes);

template <typename T>
class Ring2StringConversion : public ::testing::Test {};
TYPED_TEST_SUITE(Ring2StringConversion, CoordinateTypes);

template <typename T>
class Ring2Hash : public ::testing::Test {};
TYPED_TEST_SUITE(Ring2Hash, CoordinateTypes);

template <typename T>
class Ring2BoostGeometry : public ::testing::Test {};
TYPED_TEST_SUITE(Ring2BoostGeometry, CoordinateTypes);

template <typename T>
class Ring2BoostRange : public ::testing::Test {};
TYPED_TEST_SUITE(Ring2BoostRange, CoordinateTypes);

template <typename T>
class Ring2BoostPolygon : public ::testing::Test {};
TYPED_TEST_SUITE(Ring2BoostPolygon, CoordinateTypes);

// =============================================================
// Type aliases
// =============================================================

TEST(Ring2TypeAliases, MemberTypes) {
  static_assert(std::is_same_v<Ring2_i::base_type, std::vector<Point2_i>>);
  static_assert(std::is_same_v<Ring2_i::coordinate_type, int>);
  static_assert(std::is_same_v<Ring2_i::point_type, Point2_i>);
  static_assert(std::is_same_v<Ring2_i::iterator_type,
                               std::vector<Point2_i>::const_iterator>);
  static_assert(std::is_same_v<Ring2_i::const_iterator_type,
                               std::vector<Point2_i>::const_iterator>);
  static_assert(std::is_same_v<Ring2_i::mutable_iterator_type,
                               std::vector<Point2_i>::iterator>);
  static_assert(std::is_same_v<Ring2_i::const_reverse_iterator_type,
                               std::vector<Point2_i>::const_reverse_iterator>);
  static_assert(std::is_same_v<Ring2_i::mutable_reverse_iterator_type,
                               std::vector<Point2_i>::reverse_iterator>);
}

TEST(Ring2TypeAliases, AreaTypeIsInt64ForIntegers) {
  static_assert(std::is_same_v<Ring2_i::area_type, int64_t>);
  static_assert(std::is_same_v<Ring2<int64_t>::area_type, int64_t>);
  static_assert(std::is_same_v<Ring2<float>::area_type, float>);
  static_assert(std::is_same_v<Ring2<double>::area_type, double>);
}

// =============================================================
// Constructors
// =============================================================

TYPED_TEST(Ring2Constructor, Default) {
  Ring2<TypeParam> r;

  EXPECT_EQ(r.Size(), 0u);
  EXPECT_THAT(r.Points(), IsEmpty());
}

TYPED_TEST(Ring2Constructor, VectorCopy) {
  std::vector<Point2<TypeParam>> points = SquarePoints<TypeParam>();
  Ring2<TypeParam> r(points);

  EXPECT_EQ(points.size(), 5u);
  EXPECT_EQ(r.Size(), 5u);
  EXPECT_EQ(r.Points(), SquarePoints<TypeParam>());
}

TYPED_TEST(Ring2Constructor, VectorMove) {
  std::vector<Point2<TypeParam>> points = SquarePoints<TypeParam>();
  Ring2<TypeParam> r(std::move(points));

  EXPECT_EQ(points.size(), 0u);  // NOLINT(bugprone-use-after-move)
  EXPECT_EQ(r.Size(), 5u);
  EXPECT_EQ(r.Points(), SquarePoints<TypeParam>());
}

TYPED_TEST(Ring2Constructor, BoxGivesClosedCounterclockwiseRing) {
  using P = Point2<TypeParam>;
  Ring2<TypeParam> r(Box2<TypeParam>(0, 0, 2, 2));

  EXPECT_EQ(r.Size(), 5u);
  EXPECT_EQ(r.Points(), SquarePoints<TypeParam>());

  // A box away from the origin and not square.
  Ring2<TypeParam> r2(Box2<TypeParam>(1, 2, 4, 3));
  EXPECT_THAT(r2.Points(),
              ElementsAre(P(1, 2), P(4, 2), P(4, 3), P(1, 3), P(1, 2)));
}

TYPED_TEST(Ring2Constructor, InitializerList) {
  using P = Point2<TypeParam>;
  Ring2<TypeParam> r = {P(0, 0), P(2, 0), P(2, 2), P(0, 2), P(0, 0)};

  EXPECT_EQ(r.Size(), 5u);
  EXPECT_EQ(r.Points(), SquarePoints<TypeParam>());
}

TYPED_TEST(Ring2Constructor, Copy) {
  Ring2<TypeParam> r1(SquarePoints<TypeParam>());
  Ring2<TypeParam> r2(r1);

  EXPECT_EQ(r1.Size(), 5u);
  EXPECT_EQ(r1.Points(), SquarePoints<TypeParam>());
  EXPECT_EQ(r2.Size(), 5u);
  EXPECT_EQ(r2.Points(), SquarePoints<TypeParam>());
}

TYPED_TEST(Ring2Constructor, Move) {
  Ring2<TypeParam> r1(SquarePoints<TypeParam>());
  Ring2<TypeParam> r2(std::move(r1));

  EXPECT_EQ(r1.Size(), 0u);  // NOLINT(bugprone-use-after-move)
  EXPECT_THAT(r1.Points(), ElementsAre());
  EXPECT_EQ(r2.Size(), 5u);
  EXPECT_EQ(r2.Points(), SquarePoints<TypeParam>());
}

// =============================================================
// Assignment operators
// =============================================================

TYPED_TEST(Ring2Constructor, CopyAssignment) {
  Ring2<TypeParam> r1(SquarePoints<TypeParam>());
  Ring2<TypeParam> r2 = MakeRing<TypeParam>({{5, 5}});
  r2 = r1;

  EXPECT_EQ(r1.Points(), SquarePoints<TypeParam>());
  EXPECT_EQ(r2.Points(), SquarePoints<TypeParam>());
}

TYPED_TEST(Ring2Constructor, MoveAssignment) {
  Ring2<TypeParam> r1(SquarePoints<TypeParam>());
  Ring2<TypeParam> r2 = MakeRing<TypeParam>({{5, 5}});
  r2 = std::move(r1);

  EXPECT_EQ(r2.Points(), SquarePoints<TypeParam>());
}

// =============================================================
// Accessors
// =============================================================

TYPED_TEST(Ring2Accessor, PointsIsMutable) {
  using P = Point2<TypeParam>;
  Ring2<TypeParam> r(SquarePoints<TypeParam>());

  EXPECT_EQ(r.Size(), 5u);
  EXPECT_EQ(r.Points(), SquarePoints<TypeParam>());

  r.Points()[1] = P(3, 0);
  EXPECT_EQ(r[1], P(3, 0));
}

TYPED_TEST(Ring2Accessor, PointsConst) {
  const Ring2<TypeParam> r(SquarePoints<TypeParam>());

  EXPECT_EQ(r.Size(), 5u);
  EXPECT_EQ(r.Points(), SquarePoints<TypeParam>());
}

TYPED_TEST(Ring2Accessor, SizeAndEmpty) {
  struct Case {
    std::string name;
    Ring2<TypeParam> ring;
    size_t expected_size;
    bool expected_empty;
  };
  const std::vector<Case> cases = {
      {"empty", Ring2<TypeParam>(), 0, true},
      {"one point", MakeRing<TypeParam>({{1, 1}}), 1, false},
      {"square", Ring2<TypeParam>(SquarePoints<TypeParam>()), 5, false},
      {"L-shape", LShape<TypeParam>(), 7, false},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    EXPECT_EQ(c.ring.Size(), c.expected_size);
    EXPECT_EQ(c.ring.size(), c.expected_size);
    EXPECT_EQ(c.ring.Empty(), c.expected_empty);
  }
}

// Area goes through Boost.Polygon (gtl::area), which has coordinate traits for
// all four types, so it is typed. Integer rounding is in Ring2AreaInteger.
TYPED_TEST(Ring2Accessor, Area) {
  struct Case {
    std::string name;
    Ring2<TypeParam> ring;
    double expected;
  };
  const std::vector<Case> cases = {
      {"empty", Ring2<TypeParam>(), 0},
      {"square", Ring2<TypeParam>(SquarePoints<TypeParam>()), 2 * 2},
      {"L-shape", LShape<TypeParam>(), 2 * 2 + 2 * 4},
      {"big L-shape", BigLShape<TypeParam>(), 20 * 20 + 20 * 40},
      {"U-shape", UShape<TypeParam>(), 40 * 40 - 20 * 30},
      {"triangle", MakeRing<TypeParam>({{0, 0}, {4, 0}, {0, 2}, {0, 0}}), 4},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    EXPECT_EQ(c.ring.Area(), c.expected);
    // The same for the reversed (clockwise) point list.
    Ring2<TypeParam> reversed(std::vector<Point2<TypeParam>>(
        c.ring.Points().rbegin(), c.ring.Points().rend()));
    EXPECT_EQ(reversed.Area(), c.expected);
  }
}

TYPED_TEST(Ring2Accessor, CentroidOfRectanglesIsExact) {
  using P = Point2<TypeParam>;
  struct Case {
    std::string name;
    Box2<TypeParam> box;
    P expected;
  };
  const std::vector<Case> cases = {
      {"square at the origin", Box2<TypeParam>(0, 0, 2, 2), P(1, 1)},
      {"rectangle", Box2<TypeParam>(2, 4, 8, 6), P(5, 5)},
      {"negative coordinates", Box2<TypeParam>(-6, -4, -2, 0), P(-4, -2)},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    EXPECT_EQ(Ring2<TypeParam>(c.box).Centroid(), c.expected);
  }
}

TYPED_TEST(Ring2Accessor, PointOnSurfaceIsInside) {
  struct Case {
    std::string name;
    Ring2<TypeParam> ring;
  };
  const std::vector<Case> cases = {
      {"square", Ring2<TypeParam>(SquarePoints<TypeParam>())},
      {"L-shape", LShape<TypeParam>()},
      {"big L-shape", BigLShape<TypeParam>()},
      {"U-shape", UShape<TypeParam>()},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    EXPECT_TRUE(IsWithin(c.ring.PointOnSurface(), c.ring));
  }
}

TYPED_TEST(Ring2Accessor, PointOnSurfaceInsideWhenCentroidIsNot) {
  // The centroid of the U-shape falls inside the notch (outside the ring),
  // but PointOnSurface still returns a point inside the ring.
  const Ring2<TypeParam> r = UShape<TypeParam>();

  EXPECT_FALSE(IsWithin(r.Centroid(), r));
  EXPECT_TRUE(IsWithin(r.PointOnSurface(), r));
}

TYPED_TEST(Ring2Accessor, BoundingBox) {
  struct Case {
    std::string name;
    Ring2<TypeParam> ring;
    Box2<TypeParam> expected;
  };
  const std::vector<Case> cases = {
      {"square", Ring2<TypeParam>(SquarePoints<TypeParam>()),
       Box2<TypeParam>(0, 0, 2, 2)},
      {"L-shape", LShape<TypeParam>(), Box2<TypeParam>(0, 0, 4, 4)},
      {"big L-shape", BigLShape<TypeParam>(), Box2<TypeParam>(0, 0, 40, 40)},
      // Boost.Geometry builds an envelope from an inside-out box. A ring away
      // from the origin fails if a write ever reorders the ends of an axis.
      {"away from the origin",
       Ring2<TypeParam>(Box2<TypeParam>(100, 100, 200, 300)),
       Box2<TypeParam>(100, 100, 200, 300)},
      {"negative coordinates",
       MakeRing<TypeParam>({{-3, -1}, {-1, -5}, {2, -2}, {-3, -1}}),
       Box2<TypeParam>(-3, -5, 2, -1)},
      // Boost.Geometry gives an empty range an inside-out box; Ring2 gives
      // the zero box instead, so the result is never inverted.
      {"empty ring gives the zero box", Ring2<TypeParam>(), Box2<TypeParam>()},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    EXPECT_EQ(c.ring.BoundingBox(), c.expected);
  }
}

TYPED_TEST(Ring2Accessor, IsManhattan) {
  struct Case {
    std::string name;
    Ring2<TypeParam> ring;
    bool expected;
  };
  const std::vector<Case> cases = {
      {"empty", Ring2<TypeParam>(), true},
      {"closed square", Ring2<TypeParam>(SquarePoints<TypeParam>()), true},
      {"L-shape", LShape<TypeParam>(), true},
      {"open rectangle", MakeRing<TypeParam>({{0, 0}, {4, 0}, {4, 2}, {0, 2}}),
       true},
      {"repeated point",
       MakeRing<TypeParam>({{0, 0}, {4, 0}, {4, 0}, {4, 2}, {0, 2}, {0, 0}}),
       true},
      {"closed triangle", MakeRing<TypeParam>({{0, 0}, {4, 0}, {4, 2}, {0, 0}}),
       false},
      // The edge from the last point back to the first is diagonal.
      {"open, diagonal closing edge",
       MakeRing<TypeParam>({{0, 0}, {4, 0}, {4, 2}}), false},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    EXPECT_EQ(c.ring.IsManhattan(), c.expected);
  }
}

// MaxBoxes goes through Boost.Polygon's get_max_rectangles. Boost.Polygon is
// meant for integer coordinates, but with these axis-parallel shapes and small
// integer values float and double give the same boxes.
TYPED_TEST(Ring2Accessor, MaxBoxes) {
  using B = Box2<TypeParam>;
  EXPECT_THAT(Ring2<TypeParam>(B(0, 0, 2, 2)).MaxBoxes(),
              ElementsAre(B(0, 0, 2, 2)));
  EXPECT_THAT(BigLShape<TypeParam>().MaxBoxes(),
              UnorderedElementsAre(B(20, 0, 40, 40), B(0, 0, 40, 20)));
  EXPECT_THAT(
      UShape<TypeParam>().MaxBoxes(),
      UnorderedElementsAre(B(0, 0, 40, 10), B(0, 0, 10, 40), B(30, 0, 40, 40)));
  EXPECT_THAT(Ring2<TypeParam>().MaxBoxes(), IsEmpty());
}

// Boost.Polygon reads a diagonal edge as axis-parallel, so MaxBoxes gives no
// boxes for a ring with one rather than wrong boxes.
TYPED_TEST(Ring2Accessor, MaxBoxesOfDiagonalRingIsEmpty) {
  EXPECT_THAT(MakeRing<TypeParam>({{0, 0}, {4, 0}, {4, 4}, {0, 0}}).MaxBoxes(),
              IsEmpty());
  EXPECT_THAT(MakeRing<TypeParam>({{2, 0},
                                   {6, 0},
                                   {8, 2},
                                   {8, 6},
                                   {6, 8},
                                   {2, 8},
                                   {0, 6},
                                   {0, 2},
                                   {2, 0}})
                  .MaxBoxes(),
              IsEmpty());
  EXPECT_THAT(
      MakeRing<TypeParam>({{0, 0}, {4, 0}, {4, 3}, {3, 4}, {0, 4}, {0, 0}})
          .MaxBoxes(),
      IsEmpty());
}

// MaxBoxes(step) turns a ring of any shape into a staircase of boxes inside
// it. These shapes cross every strip boundary at whole numbers, so all four
// coordinate types give the same boxes.
TYPED_TEST(Ring2Accessor, MaxBoxesWithStep) {
  using B = Box2<TypeParam>;
  struct Case {
    std::string name;
    Ring2<TypeParam> ring;
    TypeParam step;
    std::vector<B> expected;
  };
  const std::vector<Case> cases = {
      {"a Manhattan ring gives MaxBoxes() for any step", BigLShape<TypeParam>(),
       7, BigLShape<TypeParam>().MaxBoxes()},
      {"empty", Ring2<TypeParam>(), 1, {}},
      // Inside at height y: x from y to 4. The top strip only touches the
      // corner, so it gives no box.
      {"triangle, step 1",
       MakeRing<TypeParam>({{0, 0}, {4, 0}, {4, 4}, {0, 0}}),
       1,
       {B(1, 0, 4, 1), B(2, 0, 4, 2), B(3, 0, 4, 3)}},
      {"triangle, step 2",
       MakeRing<TypeParam>({{0, 0}, {4, 0}, {4, 4}, {0, 0}}),
       2,
       {B(2, 0, 4, 2)}},
      {"triangle, the top strip is lower than the step",
       MakeRing<TypeParam>({{0, 0}, {4, 0}, {4, 4}, {0, 0}}),
       3,
       {B(3, 0, 4, 3)}},
      {"open and clockwise triangle",
       MakeRing<TypeParam>({{0, 0}, {4, 4}, {4, 0}}),
       2,
       {B(2, 0, 4, 2)}},
      // The 45-degree corners are cut as a staircase.
      {"octagon, step 1",
       MakeRing<TypeParam>({{2, 0},
                            {6, 0},
                            {8, 2},
                            {8, 6},
                            {6, 8},
                            {2, 8},
                            {0, 6},
                            {0, 2},
                            {2, 0}}),
       1,
       {B(0, 2, 8, 6), B(1, 1, 7, 7), B(2, 0, 6, 8)}},
      // A vertex at y = 2 lies inside the strip [0 4]: the strip keeps only
      // the x range inside the ring both below and above it.
      {"octagon, step 4, vertices inside a strip",
       MakeRing<TypeParam>({{2, 0},
                            {6, 0},
                            {8, 2},
                            {8, 6},
                            {6, 8},
                            {2, 8},
                            {0, 6},
                            {0, 2},
                            {2, 0}}),
       4,
       {B(2, 0, 6, 8)}},
      // Both sides start at the apex (2 0); they are told apart by the top
      // of the strip.
      {"apex at the bottom",
       MakeRing<TypeParam>({{2, 0}, {4, 4}, {0, 4}, {2, 0}}),
       2,
       {B(1, 2, 3, 4)}},
      // A slanted band: the x range shifts with y, so a tall strip has no box
      // inside it, and one-unit strips give one-unit squares.
      {"slanted band, step 10",
       MakeRing<TypeParam>({{0, 0}, {2, 0}, {12, 10}, {10, 10}, {0, 0}}),
       10,
       {}},
      {"a zero-width diagonal ring",
       MakeRing<TypeParam>({{0, 0}, {4, 4}}),
       1,
       {}},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    EXPECT_THAT(c.ring.MaxBoxes(c.step), UnorderedElementsAreArray(c.expected));
  }
}

TYPED_TEST(Ring2Accessor, MaxBoxesWithStepStaysInsideTheRing) {
  const Ring2<TypeParam> band =
      MakeRing<TypeParam>({{0, 0}, {2, 0}, {12, 10}, {10, 10}, {0, 0}});
  const std::vector<Box2<TypeParam>> boxes = band.MaxBoxes(1);

  ASSERT_EQ(boxes.size(), 10u);
  for (const Box2<TypeParam>& b : boxes) {
    SCOPED_TRACE(b.ToString());
    EXPECT_TRUE(IsCoveredBy(b, band));
    EXPECT_EQ(b.Width(), 1);
    EXPECT_EQ(b.Height(), 1);
  }
}

// =============================================================
// Accessors: type-specific behavior
// =============================================================

// For integer coordinates the box sides are rounded inward, so the boxes
// stay inside the ring; floating-point coordinates keep the exact crossings.
TEST(Ring2MaxBoxesWithStep, IntegerSidesRoundInward) {
  // The right side x = 3 - 1.5 y runs up and to the left.
  EXPECT_THAT(
      Ring2_i({Point2_i(0, 0), Point2_i(3, 0), Point2_i(0, 2), Point2_i(0, 0)})
          .MaxBoxes(1),
      ElementsAre(Box2_i(0, 0, 1, 1)));
  EXPECT_THAT(Ring2<double>({Point2<double>(0, 0), Point2<double>(3, 0),
                             Point2<double>(0, 2), Point2<double>(0, 0)})
                  .MaxBoxes(1),
              ElementsAre(Box2<double>(0, 0, 1.5, 1)));
  // The left side x = 1.5 y runs up and to the right.
  EXPECT_THAT(Ring2_i64({Point2_i64(0, 0), Point2_i64(3, 0), Point2_i64(3, 2),
                         Point2_i64(0, 0)})
                  .MaxBoxes(1),
              ElementsAre(Box2_i64(2, 0, 3, 1)));
  EXPECT_THAT(Ring2<float>({Point2<float>(0, 0), Point2<float>(3, 0),
                            Point2<float>(3, 2), Point2<float>(0, 0)})
                  .MaxBoxes(1),
              ElementsAre(Box2<float>(1.5f, 0, 3, 1)));
}

TEST(Ring2MaxBoxesWithStep, LargeIntegerCoordinates) {
  // A right triangle near the int32 limits; the crossings need 128 bits.
  constexpr int32_t kMax = std::numeric_limits<int32_t>::max();
  constexpr int32_t kMin = std::numeric_limits<int32_t>::min();
  const Ring2_i32 r = {Point2_i32(kMin, kMin), Point2_i32(kMax, kMin),
                       Point2_i32(kMax, kMax), Point2_i32(kMin, kMin)};
  // Strips [kMin -1], [-1 kMax-1] and [kMax-1 kMax]. Inside the first, x
  // runs from the diagonal at its top (x = -1) to kMax; inside the second,
  // from kMax-1; the third has no width. The two pieces share the right edge.
  EXPECT_THAT(r.MaxBoxes(kMax),
              UnorderedElementsAre(Box2_i32(-1, kMin, kMax, -1),
                                   Box2_i32(kMax - 1, kMin, kMax, kMax - 1)));
}

#ifndef NDEBUG
TEST(Ring2MaxBoxesWithStepDeathTest, StepMustBePositive) {
  const Ring2_i r = {Point2_i(0, 0), Point2_i(4, 0), Point2_i(4, 4),
                     Point2_i(0, 0)};

  EXPECT_DEATH((void)r.MaxBoxes(0), "MaxBoxes step must be positive");
}
#endif

TEST(Ring2AreaInteger, FractionalAreaRoundsDown) {
  // Twice the area is 3 for each triangle, in either orientation.
  const Ring2_i ccw = MakeRing<int>({{0, 0}, {3, 0}, {0, 1}, {0, 0}});
  const Ring2_i cw = MakeRing<int>({{0, 0}, {0, 1}, {3, 0}, {0, 0}});
  const Ring2_i64 ccw64 = MakeRing<int64_t>({{0, 0}, {3, 0}, {0, 1}, {0, 0}});

  EXPECT_EQ(ccw.Area(), 1);
  EXPECT_EQ(cw.Area(), 1);
  EXPECT_EQ(ccw64.Area(), 1);
}

TEST(Ring2AreaInteger, FractionalAreaIsExactForFloatingPoint) {
  EXPECT_EQ(MakeRing<double>({{0, 0}, {3, 0}, {0, 1}, {0, 0}}).Area(), 1.5);
  EXPECT_EQ(MakeRing<float>({{0, 0}, {3, 0}, {0, 1}, {0, 0}}).Area(), 1.5f);
}

TEST(Ring2AreaInteger, AreaNoOverflow) {
  // 100000 x 100000 does not fit in an int.
  Ring2_i r(Box2_i(0, 0, 100000, 100000));

  EXPECT_EQ(r.Area(), int64_t{10000000000});
}

// For integer coordinates the centroid is approximate and not consistently
// rounded (see Centroid). These are the documented results.
TEST(Ring2CentroidInteger, IsApproximate) {
  struct Case {
    std::string name;
    Ring2_i ring;
    Point2_i expected;
  };
  const std::vector<Case> cases = {
      // Exact (7/3, 5/3).
      {"L-shape", LShape<int>(), Point2_i(2, 1)},
      // Exact (70/3, 50/3).
      {"big L-shape", BigLShape<int>(), Point2_i(23, 16)},
      // Exact (-3.5, 0.5): x goes down to -4.
      {"rectangle", Ring2_i(Box2_i(-7, 0, 0, 1)), Point2_i(-4, 0)},
      // Exact (-4/3, 1/3): x goes up to -1.
      {"triangle", MakeRing<int>({{0, 0}, {0, 1}, {-4, 0}, {0, 0}}),
       Point2_i(-1, 0)},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    EXPECT_EQ(c.ring.Centroid(), c.expected);
  }
}

TEST(Ring2CentroidFloatingPoint, IsExactForDouble) {
  struct Case {
    std::string name;
    Ring2<double> ring;
    double expected_x;
    double expected_y;
  };
  const std::vector<Case> cases = {
      {"L-shape", LShape<double>(), 7.0 / 3, 5.0 / 3},
      {"big L-shape", BigLShape<double>(), 70.0 / 3, 50.0 / 3},
      {"rectangle", Ring2<double>(Box2<double>(-7, 0, 0, 1)), -3.5, 0.5},
      {"triangle", MakeRing<double>({{0, 0}, {0, 1}, {-4, 0}, {0, 0}}),
       -4.0 / 3, 1.0 / 3},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    const Point2<double> p = c.ring.Centroid();
    EXPECT_NEAR(p.x(), c.expected_x, 1e-12);
    EXPECT_NEAR(p.y(), c.expected_y, 1e-12);
  }
}

// =============================================================
// Mutators
// =============================================================

TYPED_TEST(Ring2Mutator, Clear) {
  Ring2<TypeParam> r(SquarePoints<TypeParam>());
  r.Clear();

  EXPECT_EQ(r.Size(), 0u);
  EXPECT_TRUE(r.Empty());
}

TYPED_TEST(Ring2Mutator, Append) {
  using P = Point2<TypeParam>;
  Ring2<TypeParam> r;
  r.Append(P(0, 0));
  r.Append(P(2, 0));
  r.Append(P(2, 2));
  r.Append(P(0, 2));
  r.Append(P(0, 0));

  EXPECT_EQ(r.Size(), 5u);
  EXPECT_EQ(r.Points(), SquarePoints<TypeParam>());
}

TYPED_TEST(Ring2Mutator, Resize) {
  using P = Point2<TypeParam>;
  struct Case {
    std::string name;
    size_t n;
    std::vector<P> expected;
  };
  const std::vector<Case> cases = {
      {"to zero", 0, {}},
      {"shrink keeps the first points", 3, {P(0, 0), P(2, 0), P(2, 2)}},
      {"same size", 5, SquarePoints<TypeParam>()},
      {"grow appends (0 0) points",
       10,
       {P(0, 0), P(2, 0), P(2, 2), P(0, 2), P(0, 0), P(0, 0), P(0, 0), P(0, 0),
        P(0, 0), P(0, 0)}},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    Ring2<TypeParam> r(SquarePoints<TypeParam>());
    r.Resize(c.n);
    EXPECT_EQ(r.Size(), c.n);
    EXPECT_EQ(r.Empty(), c.n == 0);
    EXPECT_EQ(r.Points(), c.expected);
  }
}

TYPED_TEST(Ring2Mutator, AssignIterator) {
  using P = Point2<TypeParam>;
  Ring2<TypeParam> r = {P(0, 0), P(1, 0), P(1, 1), P(0, 1), P(0, 0)};
  std::vector<P> points = {P(0, 0), P(1, 0), P(2, 0),
                           P(2, 2), P(0, 2), P(0, 0)};
  r.Assign(points.begin(), points.end());

  EXPECT_EQ(r.Size(), 6u);
  EXPECT_THAT(r.Points(), ElementsAre(P(0, 0), P(1, 0), P(2, 0), P(2, 2),
                                      P(0, 2), P(0, 0)));
}

TYPED_TEST(Ring2Mutator, AssignInitializerList) {
  using P = Point2<TypeParam>;
  Ring2<TypeParam> r = MakeRing<TypeParam>({{7, 7}});
  r.Assign({P(0, 0), P(2, 0), P(2, 2), P(0, 2), P(0, 0)});

  EXPECT_EQ(r.Size(), 5u);
  EXPECT_EQ(r.Points(), SquarePoints<TypeParam>());
}

// =============================================================
// Operations - Bloat
// =============================================================
// BloatedRings is built on MaxBoxes; see the MaxBoxes test for float and
// double.

TYPED_TEST(Ring2Operation, BloatedRingsBloatsEachMaxBoxAndMerges) {
  // The max boxes (20 0)-(40 40) and (0 0)-(40 20) bloated by 5 are
  // (15 -5)-(45 45) and (-5 -5)-(45 25), which merge into one L-shape.
  std::vector<Ring2<TypeParam>> rings =
      BigLShape<TypeParam>().BloatedRings(5, 5);

  ASSERT_EQ(rings.size(), 1u);
  EXPECT_EQ(rings[0].Area(), 20 * 30 + 30 * 50);
  EXPECT_TRUE(IsEqual(rings[0], MakeRing<TypeParam>({{-5, -5},
                                                     {45, -5},
                                                     {45, 45},
                                                     {15, 45},
                                                     {15, 25},
                                                     {-5, 25},
                                                     {-5, -5}})));
}

TYPED_TEST(Ring2Operation, BloatedRingsCases) {
  using B = Box2<TypeParam>;
  using R = Ring2<TypeParam>;
  struct Case {
    std::string name;
    R ring;
    TypeParam x;
    TypeParam y;
    std::vector<R> expected;
  };
  const std::vector<Case> cases = {
      {"square by zero", R(B(0, 0, 2, 2)), 0, 0, {R(B(0, 0, 2, 2))}},
      {"L-shape by zero", LShape<TypeParam>(), 0, 0, {LShape<TypeParam>()}},
      {"x and y differ", R(B(0, 0, 2, 2)), 1, 3, {R(B(-1, -3, 3, 5))}},
      {"x only", R(B(0, 0, 2, 2)), 1, 0, {R(B(-1, 0, 3, 2))}},
      {"empty ring", R(), 1, 1, {}},
      {"diagonal ring",
       MakeRing<TypeParam>({{0, 0}, {4, 0}, {4, 4}, {0, 0}}),
       1,
       1,
       {}},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    const std::vector<R> rings = c.ring.BloatedRings(c.x, c.y);
    ASSERT_EQ(rings.size(), c.expected.size());
    for (size_t i = 0; i < rings.size(); ++i) {
      EXPECT_EQ(rings[i], c.expected[i]) << rings[i];
    }
  }
}

#ifndef NDEBUG
TEST(Ring2BloatedRingsDeathTest, NegativeAmount) {
  Ring2_i r(Box2_i(0, 0, 10, 10));

  EXPECT_DEATH((void)r.BloatedRings(-1, 0),
               "Bloat values must be non-negative");
  EXPECT_DEATH((void)r.BloatedRings(0, -1),
               "Bloat values must be non-negative");
}
#endif

// =============================================================
// Operations - Simplify
// =============================================================

// A ring as integer (x, y) points and the closed ring Simplify leaves.
struct SimplifyCase {
  std::string name;
  std::vector<std::pair<int, int>> input;
  std::vector<std::pair<int, int>> expected;
};

template <typename T>
Ring2<T> RingOf(const std::vector<std::pair<int, int>>& xy) {
  Ring2<T> r;
  for (const auto& [x, y] : xy) r.Append(Point2<T>(x, y));
  return r;
}

const std::vector<SimplifyCase>& SimplifyCases() {
  static const auto* const kCases = new std::vector<SimplifyCase>{
      {"a closed rectangle without redundant points stays the same",
       {{0, 0}, {4, 0}, {4, 2}, {0, 2}, {0, 0}},
       {{0, 0}, {4, 0}, {4, 2}, {0, 2}, {0, 0}}},
      {"an open rectangle gets closed",
       {{0, 0}, {4, 0}, {4, 2}, {0, 2}},
       {{0, 0}, {4, 0}, {4, 2}, {0, 2}, {0, 0}}},
      {"a point on an edge",
       {{0, 0}, {2, 0}, {4, 0}, {4, 2}, {0, 2}},
       {{0, 0}, {4, 0}, {4, 2}, {0, 2}, {0, 0}}},
      {"several points in a row on edges",
       {{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {4, 1}, {4, 2}, {0, 2}, {0, 0}},
       {{0, 0}, {4, 0}, {4, 2}, {0, 2}, {0, 0}}},
      {"a redundant start point",
       {{2, 0}, {4, 0}, {4, 2}, {0, 2}, {0, 0}},
       {{4, 0}, {4, 2}, {0, 2}, {0, 0}, {4, 0}}},
      {"a redundant start point, closed",
       {{2, 0}, {4, 0}, {4, 2}, {0, 2}, {0, 0}, {2, 0}},
       {{4, 0}, {4, 2}, {0, 2}, {0, 0}, {4, 0}}},
      {"the closing point and a last point on an edge",
       {{0, 0}, {4, 0}, {4, 2}, {0, 2}, {0, 1}, {0, 0}},
       {{0, 0}, {4, 0}, {4, 2}, {0, 2}, {0, 0}}},
      {"consecutive duplicates",
       {{0, 0}, {0, 0}, {4, 0}, {4, 2}, {4, 2}, {0, 2}, {0, 0}, {0, 0}},
       {{0, 0}, {4, 0}, {4, 2}, {0, 2}, {0, 0}}},
      {"a zero-width spike out of the top edge",
       {{0, 0}, {4, 0}, {4, 2}, {2, 2}, {2, 5}, {2, 2}, {0, 2}},
       {{0, 0}, {4, 0}, {4, 2}, {0, 2}, {0, 0}}},
      // The tip of the spike and the point under it both start the ring, so
      // the first vertex that remains is (0 2).
      {"a zero-width spike at the seam",
       {{2, 5}, {2, 2}, {0, 2}, {0, 0}, {4, 0}, {4, 2}, {2, 2}},
       {{0, 2}, {0, 0}, {4, 0}, {4, 2}, {0, 2}}},
      {"a point on a diagonal edge",
       {{0, 0}, {4, 0}, {2, 2}, {0, 4}},
       {{0, 0}, {4, 0}, {0, 4}, {0, 0}}},
      // An L-shape clipped at a row seam (y = 144) on its left edge.
      {"the corners of an L-shape",
       {{3444, 432},
        {0, 432},
        {0, 288},
        {0, 144},
        {3276, 144},
        {3276, 288},
        {3444, 288},
        {3444, 432}},
       {{3444, 432},
        {0, 432},
        {0, 144},
        {3276, 144},
        {3276, 288},
        {3444, 288},
        {3444, 432}}},
      {"an empty ring", {}, {}},
      {"one repeated point", {{3, 3}, {3, 3}, {3, 3}, {3, 3}, {3, 3}}, {}},
      {"a ring folded onto one segment", {{0, 0}, {4, 0}, {4, 0}, {0, 0}}, {}},
      {"collinear points only", {{0, 0}, {2, 0}, {4, 0}, {0, 0}}, {}},
  };
  return *kCases;
}

TYPED_TEST(Ring2Operation, SimplifyRemovesRedundantPoints) {
  using T = TypeParam;
  for (const SimplifyCase& c : SimplifyCases()) {
    SCOPED_TRACE(c.name);
    Ring2<T> r = RingOf<T>(c.input);
    const auto area = r.Area();

    EXPECT_EQ(&r.Simplify(), &r);

    EXPECT_EQ(r.Points(), RingOf<T>(c.expected).Points());
    EXPECT_EQ(r.Area(), area);
    // Simplifying again changes nothing.
    const std::vector<Point2<T>> once = r.Points();
    r.Simplify();
    EXPECT_EQ(r.Points(), once);
  }
}

TYPED_TEST(Ring2Operation, SimplifyGivesTheSameRingForOpenAndClosedInput) {
  using T = TypeParam;
  Ring2<T> open = RingOf<T>({{0, 0}, {2, 0}, {4, 0}, {4, 2}, {0, 2}});
  Ring2<T> closed = RingOf<T>({{0, 0}, {2, 0}, {4, 0}, {4, 2}, {0, 2}, {0, 0}});

  EXPECT_EQ(open.Simplify().Points(), closed.Simplify().Points());
}

// The corners of a square spanning all of int32 need the cross product of
// differences up to 2^32 - 1, whose products (about 1.8e19) are past int64_t.
TEST(Ring2SimplifyLargeCoordinates, Int32Extremes) {
  constexpr int32_t kMin = std::numeric_limits<int32_t>::min();
  constexpr int32_t kMax = std::numeric_limits<int32_t>::max();
  Ring2_i32 r{Point2_i32(kMin, kMin), Point2_i32(0, kMin),
              Point2_i32(kMax, kMin), Point2_i32(kMax, kMax),
              Point2_i32(-1, -1),     Point2_i32(kMin, kMin)};

  r.Simplify();

  // (0 kMin) is on the bottom edge and (-1 -1) on the diagonal y = x back to
  // the start, so only the three corners remain.
  EXPECT_THAT(r.Points(),
              ElementsAre(Point2_i32(kMin, kMin), Point2_i32(kMax, kMin),
                          Point2_i32(kMax, kMax), Point2_i32(kMin, kMin)));
}

// The cross product at (2^32 0) is exactly 2^64, which wraps to 0 in int64_t
// arithmetic and would make that corner look collinear; the triangle must keep
// all three corners.
TEST(Ring2SimplifyLargeCoordinates, Int64CrossProductOf2To64) {
  constexpr int64_t k2To32 = int64_t{1} << 32;
  Ring2_i64 r{Point2_i64(0, 0), Point2_i64(k2To32, 0),
              Point2_i64(k2To32, k2To32), Point2_i64(0, 0)};

  r.Simplify();

  EXPECT_THAT(r.Points(),
              ElementsAre(Point2_i64(0, 0), Point2_i64(k2To32, 0),
                          Point2_i64(k2To32, k2To32), Point2_i64(0, 0)));
}

TEST(Ring2SimplifyLargeCoordinates, Int64) {
  constexpr int64_t kBig = 1'000'000'000'000'000'000;  // 1e18
  Ring2_i64 r{Point2_i64(-kBig, -kBig), Point2_i64(0, -kBig),
              Point2_i64(kBig, -kBig),  Point2_i64(kBig, kBig),
              Point2_i64(-kBig, kBig),  Point2_i64(-kBig, 0),
              Point2_i64(-kBig, -kBig)};

  r.Simplify();

  EXPECT_THAT(r.Points(),
              ElementsAre(Point2_i64(-kBig, -kBig), Point2_i64(kBig, -kBig),
                          Point2_i64(kBig, kBig), Point2_i64(-kBig, kBig),
                          Point2_i64(-kBig, -kBig)));
}

// =============================================================
// Iterators
// =============================================================

TYPED_TEST(Ring2Iterator, Forward) {
  using P = Point2<TypeParam>;
  Ring2<TypeParam> r = MakeRing<TypeParam>({{0, 0}, {1, 0}, {1, 1}});

  EXPECT_THAT(std::vector<P>(r.begin(), r.end()),
              ElementsAre(P(0, 0), P(1, 0), P(1, 1)));
  *r.begin() = P(3, 3);
  EXPECT_EQ(r[0], P(3, 3));
}

TYPED_TEST(Ring2Iterator, ForwardConst) {
  using P = Point2<TypeParam>;
  const Ring2<TypeParam> r = MakeRing<TypeParam>({{0, 0}, {1, 0}, {1, 1}});

  EXPECT_THAT(std::vector<P>(r.begin(), r.end()),
              ElementsAre(P(0, 0), P(1, 0), P(1, 1)));
}

TYPED_TEST(Ring2Iterator, Reverse) {
  using P = Point2<TypeParam>;
  Ring2<TypeParam> r{P(0, 0), P(1, 0), P(1, 1)};

  std::vector<P> reversed(r.rbegin(), r.rend());
  EXPECT_THAT(reversed, ElementsAre(P(1, 1), P(1, 0), P(0, 0)));

  *r.rbegin() = P(2, 2);
  EXPECT_EQ(r[2], P(2, 2));
}

TYPED_TEST(Ring2Iterator, ReverseConst) {
  using P = Point2<TypeParam>;
  const Ring2<TypeParam> r{P(0, 0), P(1, 0), P(1, 1)};

  EXPECT_EQ(*r.rbegin(), P(1, 1));
  EXPECT_EQ(std::distance(r.rbegin(), r.rend()), 3);
  EXPECT_THAT(std::vector<P>(r.rbegin(), r.rend()),
              ElementsAre(P(1, 1), P(1, 0), P(0, 0)));
}

// =============================================================
// Operators
// =============================================================

TYPED_TEST(Ring2Operator, Subscript) {
  using P = Point2<TypeParam>;
  Ring2<TypeParam> r(SquarePoints<TypeParam>());

  EXPECT_EQ(r.Size(), 5u);
  EXPECT_EQ(r[0], P(0, 0));
  EXPECT_EQ(r[1], P(2, 0));
  EXPECT_EQ(r[2], P(2, 2));
  EXPECT_EQ(r[3], P(0, 2));
  EXPECT_EQ(r[4], P(0, 0));

  r[0] = P(1, 1);
  r[1] = P(2, 1);
  r[2] = P(2, 2);
  r[3] = P(1, 2);
  r[4] = P(1, 1);

  EXPECT_EQ(r[0], P(1, 1));
  EXPECT_EQ(r[1], P(2, 1));
  EXPECT_EQ(r[2], P(2, 2));
  EXPECT_EQ(r[3], P(1, 2));
  EXPECT_EQ(r[4], P(1, 1));
}

TYPED_TEST(Ring2Operator, SubscriptConst) {
  using P = Point2<TypeParam>;
  const Ring2<TypeParam> r(SquarePoints<TypeParam>());

  EXPECT_EQ(r.Size(), 5u);
  EXPECT_EQ(r[0], P(0, 0));
  EXPECT_EQ(r[1], P(2, 0));
  EXPECT_EQ(r[2], P(2, 2));
  EXPECT_EQ(r[3], P(0, 2));
  EXPECT_EQ(r[4], P(0, 0));
}

// operator[] checks the index with DCHECK, so only debug builds abort.
#ifndef NDEBUG
TEST(Ring2OperatorDeathTest, SubscriptOutOfRange) {
  Ring2_i v{Point2_i(0, 0), Point2_i(1, 0), Point2_i(1, 1)};
  const Ring2_i& const_v = v;

  EXPECT_DEATH((void)v[3], "Check failed: i < d_.size");
  EXPECT_DEATH((void)const_v[3], "Check failed: i < d_.size");
}
#endif

TYPED_TEST(Ring2Operator, EqualityComparesCoveredArea) {
  using P = Point2<TypeParam>;
  // The same square with extra collinear points on its edges.
  Ring2<TypeParam> r1 = {P(0, 0), P(2, 0), P(2, 2), P(0, 2), P(0, 0)};
  Ring2<TypeParam> r2 = {P(0, 0), P(1, 0), P(2, 0), P(2, 2), P(0, 2), P(0, 0)};
  Ring2<TypeParam> r3 = {P(0, 0), P(1, 0), P(2, 0), P(2, 2),
                         P(1, 2), P(0, 2), P(0, 0)};

  EXPECT_EQ(r1.Size(), 5u);
  EXPECT_EQ(r2.Size(), 6u);
  EXPECT_EQ(r3.Size(), 7u);
  EXPECT_TRUE(r1 == r2);
  EXPECT_TRUE(r1 == r3);
  EXPECT_TRUE(r2 == r3);
  EXPECT_EQ(r1, r2);
  EXPECT_EQ(r1, r3);
  EXPECT_EQ(r2, r3);
  EXPECT_FALSE(r1 != r2);
}

TYPED_TEST(Ring2Operator, EqualityCases) {
  using P = Point2<TypeParam>;
  const Ring2<TypeParam> square(SquarePoints<TypeParam>());
  struct Case {
    std::string name;
    Ring2<TypeParam> other;
    bool expected_equal;
  };
  const std::vector<Case> cases = {
      {"itself", square, true},
      {"different start point",
       Ring2<TypeParam>({P(2, 0), P(2, 2), P(0, 2), P(0, 0), P(2, 0)}), true},
      {"same area elsewhere", Ring2<TypeParam>(Box2<TypeParam>(1, 0, 3, 2)),
       false},
      {"larger", Ring2<TypeParam>(Box2<TypeParam>(0, 0, 3, 2)), false},
      {"contained", Ring2<TypeParam>(Box2<TypeParam>(0, 0, 1, 1)), false},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    EXPECT_EQ(square == c.other, c.expected_equal);
    EXPECT_EQ(c.other == square, c.expected_equal);
    EXPECT_EQ(square != c.other, !c.expected_equal);
  }
}

// =============================================================
// String conversion
// =============================================================

TYPED_TEST(Ring2StringConversion, ToString) {
  Ring2<TypeParam> r(SquarePoints<TypeParam>());

  EXPECT_THAT(r.ToString(), StrEq("((0 0) (2 0) (2 2) (0 2) (0 0))"));
  EXPECT_THAT(Ring2<TypeParam>().ToString(), StrEq("()"));
  EXPECT_THAT(MakeRing<TypeParam>({{-1, 3}}).ToString(), StrEq("((-1 3))"));
}

TYPED_TEST(Ring2StringConversion, SupportsAbslStringify) {
  Ring2<TypeParam> r(SquarePoints<TypeParam>());
  std::string s = absl::StrFormat("%v", r);

  EXPECT_THAT(s, StrEq("((0 0) (2 0) (2 2) (0 2) (0 0))"));
}

TYPED_TEST(Ring2StringConversion, StreamOperator) {
  std::ostringstream os;
  os << Ring2<TypeParam>(SquarePoints<TypeParam>()) << ";"
     << Ring2<TypeParam>();

  EXPECT_EQ(os.str(), "((0 0) (2 0) (2 2) (0 2) (0 0));()");
}

TEST(Ring2StringConversionFloatingPoint, PrintsFractions) {
  const Ring2<double> r = {Point2<double>(0.5, -1.25), Point2<double>(2, 0)};

  EXPECT_EQ(r.ToString(), "((0.5 -1.25) (2 0))");
}

// =============================================================
// Hash
// =============================================================

TYPED_TEST(Ring2Hash, SupportsAbslHash) {
  using P = Point2<TypeParam>;
  EXPECT_TRUE(absl::VerifyTypeImplementsAbslHashCorrectly({
      Ring2<TypeParam>(),
      Ring2<TypeParam>({P(0, 0), P(2, 0), P(2, 2), P(0, 2), P(0, 0)}),
      Ring2<TypeParam>({P(0, 0), P(2, 0), P(2, 2), P(0, 2), P(0, 0)}),
      // The same square, starting at another point.
      Ring2<TypeParam>({P(2, 0), P(2, 2), P(0, 2), P(0, 0), P(2, 0)}),
      Ring2<TypeParam>({P(0, 0), P(20, 0), P(20, 20), P(0, 20), P(0, 0)}),
  }));
}

// Rings equal under operator== hash equally, however their point lists are
// written. The hash covers the bounding box and the area.
TYPED_TEST(Ring2Hash, EqualRingsHashEqually) {
  using P = Point2<TypeParam>;
  const Ring2<TypeParam> square(SquarePoints<TypeParam>());
  struct Case {
    std::string name;
    Ring2<TypeParam> ring;
  };
  const std::vector<Case> cases = {
      {"a copy", Ring2<TypeParam>(square)},
      {"another start point", {P(2, 0), P(2, 2), P(0, 2), P(0, 0), P(2, 0)}},
      {"an extra collinear point",
       {P(0, 0), P(1, 0), P(2, 0), P(2, 2), P(0, 2), P(0, 0)}},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    ASSERT_EQ(c.ring, square);
    EXPECT_EQ(absl::HashOf(c.ring), absl::HashOf(square));
  }
  // Different rings with different boxes or areas hash differently.
  EXPECT_NE(absl::HashOf(square),
            absl::HashOf(Ring2<TypeParam>(Box2<TypeParam>(0, 0, 2, 3))));
  EXPECT_NE(absl::HashOf(square),
            absl::HashOf(Ring2<TypeParam>(Box2<TypeParam>(1, 0, 3, 2))));
}

// =============================================================
// Aliases
// =============================================================

TEST(Ring2Aliases, AreRing2OfIntegerTypes) {
  static_assert(std::is_same_v<Ring2_i, Ring2<int>>);
  static_assert(std::is_same_v<Ring2_i32, Ring2<int32_t>>);
  static_assert(std::is_same_v<Ring2_i64, Ring2<int64_t>>);
}

// =============================================================
// Boost geometry traits
// =============================================================

TYPED_TEST(Ring2BoostGeometry, Traits) {
  using R = Ring2<TypeParam>;
  static_assert(
      std::is_same_v<typename bg::traits::tag<R>::type, bg::ring_tag>);
  static_assert(bg::point_order<R>::value == bg::counterclockwise);
  static_assert(bg::closure<R>::value == bg::closed);
  static_assert(
      std::is_same_v<typename bg::point_type<R>::type, Point2<TypeParam>>);
}

TYPED_TEST(Ring2BoostGeometry, ClearAppendAndResize) {
  using P = Point2<TypeParam>;
  Ring2<TypeParam> r(SquarePoints<TypeParam>());

  bg::clear(r);
  EXPECT_TRUE(r.Empty());

  bg::append(r, P(1, 2));
  bg::append(r, P(3, 4));
  EXPECT_THAT(r.Points(), ElementsAre(P(1, 2), P(3, 4)));

  bg::traits::resize<Ring2<TypeParam>>::apply(r, 3);
  EXPECT_THAT(r.Points(), ElementsAre(P(1, 2), P(3, 4), P(0, 0)));
}

TYPED_TEST(Ring2BoostGeometry, CorrectMakesTheRingClosedAndCounterclockwise) {
  using P = Point2<TypeParam>;
  // An open clockwise square.
  Ring2<TypeParam> r = {P(0, 0), P(0, 2), P(2, 2), P(2, 0)};

  bg::correct(r);

  EXPECT_EQ(r.Size(), 5u);
  EXPECT_EQ(r.Area(), 4);
  EXPECT_TRUE(bg::is_valid(r));
  EXPECT_EQ(r, Ring2<TypeParam>(SquarePoints<TypeParam>()));
}

TYPED_TEST(Ring2BoostGeometry, ReturnEnvelope) {
  Ring2<TypeParam> r(Box2<TypeParam>(100, 100, 200, 300));

  EXPECT_EQ(bg::return_envelope<Box2<TypeParam>>(r),
            Box2<TypeParam>(100, 100, 200, 300));
}

// =============================================================
// Boost range traits
// =============================================================

TYPED_TEST(Ring2BoostRange, IteratorAndSizeTypes) {
  using R = Ring2<TypeParam>;
  static_assert(std::is_same_v<typename boost::range_mutable_iterator<R>::type,
                               typename R::mutable_iterator_type>);
  static_assert(std::is_same_v<typename boost::range_const_iterator<R>::type,
                               typename R::const_iterator_type>);
  static_assert(std::is_same_v<typename boost::range_size<R>::type, size_t>);
}

// Boost.Geometry walks the ring through the Boost.Range traits.
TYPED_TEST(Ring2BoostRange, UsedByBoostGeometry) {
  using P = Point2<TypeParam>;
  Ring2<TypeParam> r(SquarePoints<TypeParam>());

  EXPECT_EQ(bg::num_points(r), 5u);
  bg::for_each_point(r, [](P& p) { p.SetX(p.x() + 1); });
  EXPECT_THAT(r.Points(),
              ElementsAre(P(1, 0), P(3, 0), P(3, 2), P(1, 2), P(1, 0)));
}

// =============================================================
// Boost polygon traits
// =============================================================
// Boost.Polygon has coordinate traits for all four types.

TYPED_TEST(Ring2BoostPolygon, ConceptAndGetters) {
  using R = Ring2<TypeParam>;
  static_assert(std::is_same_v<typename gtl::geometry_concept<R>::type,
                               gtl::polygon_concept>);

  R r(Box2<TypeParam>(0, 0, 2, 3));
  EXPECT_EQ(gtl::size(r), 5u);
  EXPECT_EQ(*gtl::begin_points(r), Point2<TypeParam>(0, 0));
  EXPECT_EQ(std::distance(gtl::begin_points(r), gtl::end_points(r)), 5);
  EXPECT_EQ(gtl::area(r), 6);
  EXPECT_EQ(gtl::winding(r), gtl::COUNTERCLOCKWISE);
}

TYPED_TEST(Ring2BoostPolygon, SetPoints) {
  using P = Point2<TypeParam>;
  std::vector<P> points = {P(0, 0), P(4, 0), P(4, 2), P(0, 2)};
  Ring2<TypeParam> r = MakeRing<TypeParam>({{9, 9}});

  gtl::set_points(r, points.begin(), points.end());

  EXPECT_EQ(r.Size(), 4u);
  EXPECT_EQ(r.Points(), points);
  EXPECT_EQ(gtl::area(r), 8);
}

// Boost.Polygon writes the rings of a set operation closed (the first point
// repeats at the end) and counterclockwise, which matches the Boost.Geometry
// traits, so Boost.Geometry algorithms work on them unchanged.
template <typename T>
void ExpectSetOutputMatchesGeometryTraits() {
  std::vector<Ring2<T>> rings;
  Assign(rings,
         std::vector<Box2<T>>{Box2<T>(0, 0, 10, 10), Box2<T>(5, 5, 15, 15)});

  ASSERT_EQ(rings.size(), 1u);
  const Ring2<T>& r = rings[0];
  EXPECT_EQ(r.Size(), 9u);
  EXPECT_EQ(r.Points().front(), r.Points().back());
  EXPECT_EQ(bg::area(r), 175);  // Positive: counterclockwise.
  EXPECT_EQ(r.Area(), 175);
  EXPECT_TRUE(bg::is_valid(r));
  EXPECT_TRUE(IsWithin(Point2<T>(1, 1), r));
  EXPECT_FALSE(IsWithin(Point2<T>(1, 14), r));
}

TEST(Ring2BoostPolygonSetOutput, ClosedAndCounterclockwise) {
  ExpectSetOutputMatchesGeometryTraits<int32_t>();
  ExpectSetOutputMatchesGeometryTraits<int64_t>();
}

TYPED_TEST(Ring2BoostPolygon, SetPointsFromAnotherPointType) {
  std::vector<gtl::point_data<TypeParam>> points = {
      gtl::point_data<TypeParam>(0, 0), gtl::point_data<TypeParam>(4, 0),
      gtl::point_data<TypeParam>(4, 2)};
  Ring2<TypeParam> r;

  gtl::set_points(r, points.begin(), points.end());

  EXPECT_THAT(r.Points(),
              ElementsAre(Point2<TypeParam>(0, 0), Point2<TypeParam>(4, 0),
                          Point2<TypeParam>(4, 2)));
}

}  // namespace
}  // namespace moab
