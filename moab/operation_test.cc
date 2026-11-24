#include "operation.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <string>
#include <type_traits>
#include <vector>

#include "absl/strings/str_cat.h"
#include "moab/box2.h"
#include "moab/point2.h"
#include "moab/ring2.h"
#include "moab/segment2.h"

// The tests follow the order of operation.h: set operators, set functions and
// predicates.

namespace moab {
namespace {

using ::testing::ElementsAre;

// =============================================================
// Helpers for the set operations and set functions
// =============================================================

std::vector<Ring2_i> ToRings(const std::vector<Box2_i>& boxes) {
  std::vector<Ring2_i> rings;
  for (const Box2_i& b : boxes) rings.push_back(Ring2_i(b));
  return rings;
}

// Checks that 'shapes' cover exactly the area of 'expected' and do not overlap
// each other: their areas add up to the area they cover. How Boost.Polygon cuts
// an area into shapes is not checked, except that an empty result must be
// empty and a single-rectangle result must be that one rectangle.
template <typename Shapes>
void ExpectRegion(const Shapes& shapes, const std::vector<Box2_i>& expected) {
  EXPECT_TRUE(Equivalence(shapes, expected));
  const int64_t area = Area(expected);
  EXPECT_EQ(Area(shapes), area);
  int64_t sum = 0;
  for (const auto& shape : shapes) sum += shape.Area();
  EXPECT_EQ(sum, area) << "the shapes overlap";
  if (expected.empty()) {
    EXPECT_TRUE(shapes.empty());
  } else if (expected.size() == 1) {
    ASSERT_EQ(shapes.size(), 1u);
    EXPECT_TRUE(IsEqual(shapes[0], expected[0]));
  }
}

// A set operation on lhs and rhs, with the area expected in lhs afterwards.
struct SetCase {
  std::string name;
  std::vector<Box2_i> lhs;
  std::vector<Box2_i> rhs;
  std::vector<Box2_i> expected;
};

// Runs 'op' on each case twice: with std::vector<Box2_i> containers and with
// std::vector<Ring2_i> containers. rhs is taken by const reference, so it is
// never changed.
template <typename Op>
void ExpectSetOperation(Op op, const std::vector<SetCase>& cases) {
  for (const SetCase& c : cases) {
    SCOPED_TRACE(c.name);
    {
      SCOPED_TRACE("std::vector<Box2_i>");
      std::vector<Box2_i> lhs = c.lhs;
      op(lhs, c.rhs);
      ExpectRegion(lhs, c.expected);
    }
    {
      SCOPED_TRACE("std::vector<Ring2_i>");
      std::vector<Ring2_i> lhs = ToRings(c.lhs);
      op(lhs, ToRings(c.rhs));
      ExpectRegion(lhs, c.expected);
    }
  }
}

// Runs 'op' with lhs = the square (0 0)-(10 10) and rhs = the square
// (5 5)-(15 15), for every lhs container and rhs type 'op' accepts: a Box2_i
// container takes a Box2_i or a Box2_i container, and a Ring2_i container
// takes all four.
template <typename Op>
void ExpectAllTypeCombinations(Op op, const std::vector<Box2_i>& expected) {
  const Box2_i lhs_box(0, 0, 10, 10);
  const Box2_i rhs_box(5, 5, 15, 15);
  const auto run = [&](const char* types, auto lhs, const auto& rhs) {
    SCOPED_TRACE(types);
    op(lhs, rhs);
    ExpectRegion(lhs, expected);
  };
  const std::vector<Box2_i> lhs_boxes = {lhs_box};
  const std::vector<Box2_i> rhs_boxes = {rhs_box};
  run("lhs vector<Box2_i>, rhs Box2_i", lhs_boxes, rhs_box);
  run("lhs vector<Box2_i>, rhs vector<Box2_i>", lhs_boxes, rhs_boxes);
  run("lhs vector<Ring2_i>, rhs Box2_i", ToRings(lhs_boxes), rhs_box);
  run("lhs vector<Ring2_i>, rhs Ring2_i", ToRings(lhs_boxes), Ring2_i(rhs_box));
  run("lhs vector<Ring2_i>, rhs vector<Box2_i>", ToRings(lhs_boxes), rhs_boxes);
  run("lhs vector<Ring2_i>, rhs vector<Ring2_i>", ToRings(lhs_boxes),
      ToRings(rhs_boxes));
}

// =============================================================
// Set operators
// =============================================================

TEST(UnionSet, Cases) {
  ExpectSetOperation(
      UnionSet,
      {
          {"adjacent boxes merge into one",
           {Box2_i(0, 0, 1, 1)},
           {Box2_i(1, 0, 2, 1)},
           {Box2_i(0, 0, 2, 1)}},
          {"an L-shape",
           {Box2_i(0, 0, 1, 1)},
           {Box2_i(1, 0, 2, 1), Box2_i(0, 1, 1, 2)},
           {Box2_i(0, 0, 2, 1), Box2_i(0, 1, 1, 2)}},
          {"a distant box stays separate",
           {Box2_i(0, 0, 1, 1)},
           {Box2_i(1, 0, 2, 1), Box2_i(10, 10, 20, 20)},
           {Box2_i(0, 0, 2, 1), Box2_i(10, 10, 20, 20)}},
          {"duplicate boxes merge into one",
           {Box2_i(0, 0, 1, 1)},
           {Box2_i(0, 0, 1, 1), Box2_i(0, 0, 1, 1), Box2_i(0, 0, 1, 1),
            Box2_i(0, 0, 1, 1)},
           {Box2_i(0, 0, 1, 1)}},
          {"lhs inside rhs",
           {Box2_i(0, 0, 1, 1)},
           {Box2_i(0, 0, 2, 2)},
           {Box2_i(0, 0, 2, 2)}},
          {"overlapping squares",
           {Box2_i(0, 0, 10, 10)},
           {Box2_i(5, 5, 15, 15)},
           {Box2_i(0, 0, 10, 10), Box2_i(5, 5, 15, 15)}},
          {"boxes touching at a corner stay separate",
           {Box2_i(0, 0, 1, 1)},
           {Box2_i(1, 1, 2, 2)},
           {Box2_i(0, 0, 1, 1), Box2_i(1, 1, 2, 2)}},
          {"empty lhs", {}, {Box2_i(0, 0, 1, 1)}, {Box2_i(0, 0, 1, 1)}},
          {"empty rhs", {Box2_i(0, 0, 1, 1)}, {}, {Box2_i(0, 0, 1, 1)}},
      });
}

TEST(UnionSet, AllTypeCombinations) {
  ExpectAllTypeCombinations(UnionSet,
                            {Box2_i(0, 0, 10, 10), Box2_i(5, 5, 15, 15)});
}

TEST(IntersectionSet, Cases) {
  ExpectSetOperation(
      IntersectionSet,
      {
          {"touching boxes have no intersection",
           {Box2_i(0, 0, 1, 1)},
           {Box2_i(1, 0, 2, 1)},
           {}},
          {"a box touching several boxes has no intersection",
           {Box2_i(0, 0, 1, 1)},
           {Box2_i(1, 0, 2, 1), Box2_i(0, 1, 1, 2)},
           {}},
          {"rhs inside lhs",
           {Box2_i(0, 0, 2, 2)},
           {Box2_i(0, 0, 1, 1)},
           {Box2_i(0, 0, 1, 1)}},
          {"partial overlap",
           {Box2_i(0, 0, 10, 10)},
           {Box2_i(5, 5, 15, 15)},
           {Box2_i(5, 5, 10, 10)}},
          {"two separate pieces",
           {Box2_i(0, 0, 10, 10)},
           {Box2_i(-5, 2, 3, 4), Box2_i(8, 8, 20, 20)},
           {Box2_i(0, 2, 3, 4), Box2_i(8, 8, 10, 10)}},
          {"apart", {Box2_i(0, 0, 1, 1)}, {Box2_i(5, 5, 6, 6)}, {}},
          {"empty rhs", {Box2_i(0, 0, 1, 1)}, {}, {}},
      });
}

TEST(IntersectionSet, AllTypeCombinations) {
  ExpectAllTypeCombinations(IntersectionSet, {Box2_i(5, 5, 10, 10)});
}

TEST(DisjointUnionSet, Cases) {
  ExpectSetOperation(
      DisjointUnionSet,
      {
          {"lhs inside rhs leaves the rest of rhs",
           {Box2_i(0, 0, 1, 1)},
           {Box2_i(0, 0, 2, 2)},
           {Box2_i(1, 0, 2, 1), Box2_i(0, 1, 2, 2)}},
          {"lhs inside rhs, with a distant box",
           {Box2_i(0, 0, 1, 1)},
           {Box2_i(0, 0, 2, 2), Box2_i(10, 10, 20, 20)},
           {Box2_i(1, 0, 2, 1), Box2_i(0, 1, 2, 2), Box2_i(10, 10, 20, 20)}},
          {"adjacent boxes merge into one",
           {Box2_i(0, 0, 1, 1)},
           {Box2_i(1, 0, 2, 1)},
           {Box2_i(0, 0, 2, 1)}},
          {"equal boxes cancel",
           {Box2_i(0, 0, 2, 2)},
           {Box2_i(0, 0, 2, 2)},
           {}},
          {"overlapping squares",
           {Box2_i(0, 0, 10, 10)},
           {Box2_i(5, 5, 15, 15)},
           {Box2_i(0, 0, 10, 5), Box2_i(0, 5, 5, 10), Box2_i(10, 5, 15, 10),
            Box2_i(5, 10, 15, 15)}},
      });
}

TEST(DisjointUnionSet, AllTypeCombinations) {
  ExpectAllTypeCombinations(DisjointUnionSet,
                            {Box2_i(0, 0, 10, 5), Box2_i(0, 5, 5, 10),
                             Box2_i(10, 5, 15, 10), Box2_i(5, 10, 15, 15)});
}

TEST(SubtractSet, Cases) {
  ExpectSetOperation(
      SubtractSet,
      {
          {"a corner leaves an L-shape",
           {Box2_i(0, 0, 2, 2)},
           {Box2_i(0, 0, 1, 1)},
           {Box2_i(1, 0, 2, 1), Box2_i(0, 1, 2, 2)}},
          {"an equal box leaves nothing",
           {Box2_i(0, 0, 2, 2)},
           {Box2_i(0, 0, 2, 2)},
           {}},
          {"touching and distant boxes remove nothing",
           {Box2_i(0, 0, 2, 2)},
           {Box2_i(2, 0, 4, 2), Box2_i(10, 10, 20, 20)},
           {Box2_i(0, 0, 2, 2)}},
          {"the middle of several boxes",
           {Box2_i(0, 0, 2, 2), Box2_i(2, 0, 5, 2)},
           {Box2_i(2, 0, 4, 2), Box2_i(10, 10, 20, 20)},
           {Box2_i(0, 0, 2, 2), Box2_i(4, 0, 5, 2)}},
          {"a band splits a square in two",
           {Box2_i(0, 0, 10, 10)},
           {Box2_i(-1, 4, 11, 6)},
           {Box2_i(0, 0, 10, 4), Box2_i(0, 6, 10, 10)}},
          {"a larger box leaves nothing",
           {Box2_i(0, 0, 10, 10)},
           {Box2_i(-1, -1, 11, 11)},
           {}},
          // A Ring2_i has no holes, so Boost.Polygon returns one polygon slit
          // open to the hole (a keyhole); a Box2_i container gets rectangles.
          {"punching a hole",
           {Box2_i(0, 0, 10, 10)},
           {Box2_i(4, 4, 6, 6)},
           {Box2_i(0, 0, 10, 4), Box2_i(0, 6, 10, 10), Box2_i(0, 4, 4, 6),
            Box2_i(6, 4, 10, 6)}},
          {"empty rhs removes nothing",
           {Box2_i(0, 0, 1, 1)},
           {},
           {Box2_i(0, 0, 1, 1)}},
      });
}

TEST(SubtractSet, AllTypeCombinations) {
  ExpectAllTypeCombinations(SubtractSet,
                            {Box2_i(0, 0, 10, 5), Box2_i(0, 5, 5, 10)});
}

// =============================================================
// Set functions
// =============================================================

TEST(Assign, Cases) {
  ExpectSetOperation(
      Assign,
      {
          {"into an empty lhs", {}, {Box2_i(0, 0, 1, 1)}, {Box2_i(0, 0, 1, 1)}},
          {"replaces lhs",
           {Box2_i(0, 0, 2, 2)},
           {Box2_i(0, 0, 1, 1)},
           {Box2_i(0, 0, 1, 1)}},
          {"merges nested boxes",
           {},
           {Box2_i(0, 0, 1, 1), Box2_i(0, 0, 2, 2)},
           {Box2_i(0, 0, 2, 2)}},
          {"merges nested boxes and keeps a distant box",
           {},
           {Box2_i(0, 0, 1, 1), Box2_i(0, 0, 2, 2), Box2_i(0, 0, 4, 4),
            Box2_i(10, 10, 20, 20)},
           {Box2_i(0, 0, 4, 4), Box2_i(10, 10, 20, 20)}},
          {"merges overlapping boxes",
           {},
           {Box2_i(0, 0, 10, 10), Box2_i(5, 5, 15, 15)},
           {Box2_i(0, 0, 10, 10), Box2_i(5, 5, 15, 15)}},
          {"an empty rhs empties lhs", {Box2_i(0, 0, 1, 1)}, {}, {}},
      });
}

TEST(Assign, AllTypeCombinations) {
  ExpectAllTypeCombinations(Assign, {Box2_i(5, 5, 15, 15)});
}

TEST(Equivalence, Cases) {
  struct Case {
    std::string name;
    std::vector<Box2_i> lhs;
    std::vector<Box2_i> rhs;
    bool expected;
  };
  const std::vector<Case> cases = {
      {"same box", {Box2_i(0, 0, 1, 1)}, {Box2_i(0, 0, 1, 1)}, true},
      {"different boxes", {Box2_i(0, 0, 1, 1)}, {Box2_i(0, 0, 2, 1)}, false},
      {"split and whole box",
       {Box2_i(0, 0, 1, 1), Box2_i(1, 0, 2, 1)},
       {Box2_i(0, 0, 2, 1)},
       true},
      {"duplicates do not matter",
       {Box2_i(0, 0, 1, 1), Box2_i(0, 0, 1, 1), Box2_i(0, 0, 1, 1),
        Box2_i(0, 0, 1, 1), Box2_i(1, 0, 2, 1)},
       {Box2_i(0, 0, 2, 1), Box2_i(0, 0, 2, 1), Box2_i(0, 0, 2, 1)},
       true},
      {"order does not matter",
       {Box2_i(1, 0, 2, 1), Box2_i(0, 0, 1, 1)},
       {Box2_i(0, 0, 1, 1), Box2_i(1, 0, 2, 1)},
       true},
      {"extra area",
       {Box2_i(0, 0, 1, 1)},
       {Box2_i(0, 0, 1, 1), Box2_i(0, 0, 1, 2)},
       false},
      {"both empty", {}, {}, true},
      {"empty and a box", {}, {Box2_i(0, 0, 1, 1)}, false},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    EXPECT_EQ(Equivalence(c.lhs, c.rhs), c.expected);
    EXPECT_EQ(Equivalence(ToRings(c.lhs), ToRings(c.rhs)), c.expected);
    EXPECT_EQ(Equivalence(c.rhs, c.lhs), c.expected);
  }
}

// The square (0 0)-(10 10) written in each of the four types, as one shape or
// split into halves, against every type, and against a shifted square.
TEST(Equivalence, AllTypeCombinations) {
  const Box2_i square(0, 0, 10, 10);
  const Box2_i left(0, 0, 5, 10);
  const Box2_i right(5, 0, 10, 10);
  const Box2_i shifted(1, 0, 11, 10);
  const auto check_against_all = [&](const char* type, const auto& x) {
    SCOPED_TRACE(type);
    EXPECT_TRUE(Equivalence(x, square));
    EXPECT_TRUE(Equivalence(x, Ring2_i(square)));
    EXPECT_TRUE(Equivalence(x, std::vector<Box2_i>{left, right}));
    EXPECT_TRUE(Equivalence(x, ToRings({left, right})));
    EXPECT_FALSE(Equivalence(x, shifted));
    EXPECT_FALSE(Equivalence(x, Ring2_i(shifted)));
    EXPECT_FALSE(Equivalence(x, std::vector<Box2_i>{shifted}));
    EXPECT_FALSE(Equivalence(x, ToRings({shifted})));
  };
  check_against_all("Box2_i", square);
  check_against_all("Ring2_i", Ring2_i(square));
  check_against_all("vector<Box2_i>", std::vector<Box2_i>{left, right});
  check_against_all("vector<Ring2_i>", ToRings({left, right}));
}

TEST(Area, Cases) {
  struct Case {
    std::string name;
    std::vector<Box2_i> set;
    int64_t expected;
  };
  const std::vector<Case> cases = {
      {"one box", {Box2_i(0, 0, 1, 1)}, 1},
      {"adjacent boxes", {Box2_i(0, 0, 1, 1), Box2_i(1, 0, 2, 1)}, 2},
      {"nested boxes count once", {Box2_i(0, 0, 1, 1), Box2_i(0, 0, 2, 2)}, 4},
      {"nested and distant boxes",
       {Box2_i(0, 0, 1, 1), Box2_i(0, 0, 2, 2), Box2_i(0, 0, 4, 4),
        Box2_i(10, 10, 20, 20)},
       116},
      {"a zero-size box", {Box2_i(0, 0, 0, 0)}, 0},
      {"one square", {Box2_i(0, 0, 10, 10)}, 100},
      {"duplicate and nested squares count once",
       {Box2_i(0, 0, 10, 10), Box2_i(0, 0, 10, 10), Box2_i(4, 4, 6, 6)},
       100},
      {"overlapping squares count once",
       {Box2_i(0, 0, 10, 10), Box2_i(5, 0, 15, 10)},
       150},
      {"empty", {}, 0},
  };
  for (const Case& c : cases) {
    SCOPED_TRACE(c.name);
    EXPECT_EQ(Area(c.set), c.expected);
    EXPECT_EQ(Area(ToRings(c.set)), c.expected);
  }
}

TEST(Area, AllTypes) {
  const Box2_i a(0, 0, 10, 10);
  const Box2_i b(5, 5, 15, 15);
  EXPECT_EQ(Area(a), 100);
  EXPECT_EQ(Area(Ring2_i{Point2_i(0, 0), Point2_i(20, 0), Point2_i(20, 10),
                         Point2_i(10, 10), Point2_i(10, 20), Point2_i(0, 20),
                         Point2_i(0, 0)}),
            300);
  EXPECT_EQ(Area(std::vector<Box2_i>{a, b}), 175);
  EXPECT_EQ(Area(ToRings({a, b})), 175);
}

// =============================================================
// Predicates
// =============================================================
// The Predicates tests cover every ordered pair of Point2, Segment2, Box2 and
// Ring2 that each predicate compiles for, in both argument orders. Their
// expected results come from a model independent of Boost: each geometry
// classifies sample points on a half-unit grid as interior, boundary or
// exterior, and the predicates follow the OGC definitions. All geometries have
// integer coordinates and axis-aligned or 45-degree edges, so every
// intersection lies on that grid.

enum Kind { kPoint, kSegment, kBox, kRing };

template <typename T>
constexpr Kind KindOf() {
  if constexpr (std::is_same_v<T, Point2_i>) {
    return kPoint;
  } else if constexpr (std::is_same_v<T, Segment2_i>) {
    return kSegment;
  } else if constexpr (std::is_same_v<T, Box2_i>) {
    return kBox;
  } else {
    static_assert(std::is_same_v<T, Ring2_i>);
    return kRing;
  }
}

// The ordered pairs (g1, g2) that IsTouch and IsStrictlyIntersect, IsOverlap,
// IsWithin and IsCoveredBy compile for. IsIntersect, IsDisjoint and IsEqual
// compile for all pairs.
template <typename A, typename B>
constexpr bool TouchSupported() {
  constexpr Kind a = KindOf<A>(), b = KindOf<B>();
  return (a == kPoint && b != kBox) ||
         (b == kPoint && (a == kSegment || a == kRing)) ||
         (a == b && (a == kBox || a == kRing));
}
template <typename A, typename B>
constexpr bool OverlapSupported() {
  constexpr Kind a = KindOf<A>(), b = KindOf<B>();
  return ((a == kPoint || a == kRing) && (b == kPoint || b == kRing)) ||
         (a == kBox && (b == kBox || b == kRing));
}
template <typename A, typename B>
constexpr bool WithinSupported() {
  constexpr Kind a = KindOf<A>(), b = KindOf<B>();
  return a == kPoint || (a == b && (a == kBox || a == kRing));
}
template <typename A, typename B>
constexpr bool CoveredBySupported() {
  constexpr Kind a = KindOf<A>(), b = KindOf<B>();
  return a == kPoint ||
         ((a == kBox || a == kRing) && (b == kBox || b == kRing));
}

// Expected predicate results for a pair (a, b); 1 is true and 0 false.
struct Rel {
  int intersect, strictly, overlap, touch, within_ab, within_ba, covered_ab,
      covered_ba, disjoint, equal;
};

template <typename A, typename B>
void ExpectOrdered(const A& a, const B& b, const Rel& r, bool within,
                   bool covered) {
  SCOPED_TRACE(absl::StrCat(a, " vs ", b));
  EXPECT_EQ(IsIntersect(a, b), r.intersect == 1);
  EXPECT_EQ(IsDisjoint(a, b), r.disjoint == 1);
  EXPECT_EQ(IsEqual(a, b), r.equal == 1);
  if constexpr (TouchSupported<A, B>()) {
    EXPECT_EQ(IsTouch(a, b), r.touch == 1);
    EXPECT_EQ(IsStrictlyIntersect(a, b), r.strictly == 1);
  }
  if constexpr (OverlapSupported<A, B>()) {
    EXPECT_EQ(IsOverlap(a, b), r.overlap == 1);
  }
  if constexpr (WithinSupported<A, B>()) {
    EXPECT_EQ(IsWithin(a, b), within);
  }
  if constexpr (CoveredBySupported<A, B>()) {
    EXPECT_EQ(IsCoveredBy(a, b), covered);
  }
}

// Checks 'r' for (a, b), and for (b, a) with within and covered-by swapped.
template <typename A, typename B>
void ExpectRelations(const A& a, const B& b, const Rel& r) {
  ExpectOrdered(a, b, r, r.within_ab == 1, r.covered_ab == 1);
  ExpectOrdered(b, a, r, r.within_ba == 1, r.covered_ba == 1);
}

// Columns of Rel: intersect, strictly, overlap, touch, within(a, b),
// within(b, a), covered(a, b), covered(b, a), disjoint, equal.

TEST(Predicates, PointPoint) {
  // same point
  ExpectRelations(Point2_i(3, 3), Point2_i(3, 3),
                  Rel{1, 1, 0, 0, 1, 1, 1, 1, 0, 1});
  // different points
  ExpectRelations(Point2_i(3, 3), Point2_i(4, 3),
                  Rel{0, 0, 0, 0, 0, 0, 0, 0, 1, 0});
  // same point, negative coordinates
  ExpectRelations(Point2_i(-7, -2), Point2_i(-7, -2),
                  Rel{1, 1, 0, 0, 1, 1, 1, 1, 0, 1});
}

TEST(Predicates, PointSegment) {
  // point inside the segment
  ExpectRelations(Point2_i(5, 0), Segment2_i(0, 0, 10, 0),
                  Rel{1, 1, 0, 0, 1, 0, 1, 0, 0, 0});
  // point at the end
  ExpectRelations(Point2_i(10, 0), Segment2_i(0, 0, 10, 0),
                  Rel{1, 0, 0, 1, 0, 0, 1, 0, 0, 0});
  // point at the start
  ExpectRelations(Point2_i(0, 0), Segment2_i(0, 0, 10, 0),
                  Rel{1, 0, 0, 1, 0, 0, 1, 0, 0, 0});
  // point on the line, past the end
  ExpectRelations(Point2_i(15, 0), Segment2_i(0, 0, 10, 0),
                  Rel{0, 0, 0, 0, 0, 0, 0, 0, 1, 0});
  // point off the line
  ExpectRelations(Point2_i(5, 1), Segment2_i(0, 0, 10, 0),
                  Rel{0, 0, 0, 0, 0, 0, 0, 0, 1, 0});
  // point inside a diagonal segment
  ExpectRelations(Point2_i(2, 2), Segment2_i(0, 0, 4, 4),
                  Rel{1, 1, 0, 0, 1, 0, 1, 0, 0, 0});
  // point beside a diagonal segment
  ExpectRelations(Point2_i(2, 3), Segment2_i(0, 0, 4, 4),
                  Rel{0, 0, 0, 0, 0, 0, 0, 0, 1, 0});
}

TEST(Predicates, PointBox) {
  // point at the lower-left corner
  ExpectRelations(Point2_i(0, 0), Box2_i(0, 0, 10, 10),
                  Rel{1, 0, 0, 1, 0, 0, 1, 0, 0, 0});
  // point at the upper-right corner
  ExpectRelations(Point2_i(10, 10), Box2_i(0, 0, 10, 10),
                  Rel{1, 0, 0, 1, 0, 0, 1, 0, 0, 0});
  // point at the upper-left corner
  ExpectRelations(Point2_i(0, 10), Box2_i(0, 0, 10, 10),
                  Rel{1, 0, 0, 1, 0, 0, 1, 0, 0, 0});
  // point at the lower-right corner
  ExpectRelations(Point2_i(10, 0), Box2_i(0, 0, 10, 10),
                  Rel{1, 0, 0, 1, 0, 0, 1, 0, 0, 0});
  // point inside, at the center
  ExpectRelations(Point2_i(5, 5), Box2_i(0, 0, 10, 10),
                  Rel{1, 1, 0, 0, 1, 0, 1, 0, 0, 0});
  // point inside
  ExpectRelations(Point2_i(3, 4), Box2_i(0, 0, 10, 10),
                  Rel{1, 1, 0, 0, 1, 0, 1, 0, 0, 0});
  // point on the top edge
  ExpectRelations(Point2_i(3, 10), Box2_i(0, 0, 10, 10),
                  Rel{1, 0, 0, 1, 0, 0, 1, 0, 0, 0});
  // point on the left edge
  ExpectRelations(Point2_i(0, 5), Box2_i(0, 0, 10, 10),
                  Rel{1, 0, 0, 1, 0, 0, 1, 0, 0, 0});
  // point on the right edge
  ExpectRelations(Point2_i(10, 5), Box2_i(0, 0, 10, 10),
                  Rel{1, 0, 0, 1, 0, 0, 1, 0, 0, 0});
  // point on the bottom edge
  ExpectRelations(Point2_i(5, 0), Box2_i(0, 0, 10, 10),
                  Rel{1, 0, 0, 1, 0, 0, 1, 0, 0, 0});
  // point just right of the lower-right corner
  ExpectRelations(Point2_i(11, 0), Box2_i(0, 0, 10, 10),
                  Rel{0, 0, 0, 0, 0, 0, 0, 0, 1, 0});
  // point just above the upper-left corner
  ExpectRelations(Point2_i(0, 11), Box2_i(0, 0, 10, 10),
                  Rel{0, 0, 0, 0, 0, 0, 0, 0, 1, 0});
  // point just left of the lower-left corner
  ExpectRelations(Point2_i(-1, 0), Box2_i(0, 0, 10, 10),
                  Rel{0, 0, 0, 0, 0, 0, 0, 0, 1, 0});
  // point just below the lower-left corner
  ExpectRelations(Point2_i(0, -1), Box2_i(0, 0, 10, 10),
                  Rel{0, 0, 0, 0, 0, 0, 0, 0, 1, 0});
  // point far away
  ExpectRelations(Point2_i(100, -1), Box2_i(0, 0, 10, 10),
                  Rel{0, 0, 0, 0, 0, 0, 0, 0, 1, 0});
  // point above, in line with the left edge
  ExpectRelations(Point2_i(0, 20), Box2_i(0, 0, 10, 10),
                  Rel{0, 0, 0, 0, 0, 0, 0, 0, 1, 0});
}

TEST(Predicates, PointRing) {
  // point inside the square
  ExpectRelations(Point2_i(5, 5),
                  Ring2_i{Point2_i(0, 0), Point2_i(10, 0), Point2_i(10, 10),
                          Point2_i(0, 10), Point2_i(0, 0)},
                  Rel{1, 1, 0, 0, 1, 0, 1, 0, 0, 0});
  // point on an edge of the square
  ExpectRelations(Point2_i(10, 5),
                  Ring2_i{Point2_i(0, 0), Point2_i(10, 0), Point2_i(10, 10),
                          Point2_i(0, 10), Point2_i(0, 0)},
                  Rel{1, 0, 0, 1, 0, 0, 1, 0, 0, 0});
  // point on a corner of the square
  ExpectRelations(Point2_i(10, 10),
                  Ring2_i{Point2_i(0, 0), Point2_i(10, 0), Point2_i(10, 10),
                          Point2_i(0, 10), Point2_i(0, 0)},
                  Rel{1, 0, 0, 1, 0, 0, 1, 0, 0, 0});
  // point outside the square
  ExpectRelations(Point2_i(11, 5),
                  Ring2_i{Point2_i(0, 0), Point2_i(10, 0), Point2_i(10, 10),
                          Point2_i(0, 10), Point2_i(0, 0)},
                  Rel{0, 0, 0, 0, 0, 0, 0, 0, 1, 0});
  // point inside an arm of the L
  ExpectRelations(Point2_i(5, 15),
                  Ring2_i{Point2_i(0, 0), Point2_i(20, 0), Point2_i(20, 10),
                          Point2_i(10, 10), Point2_i(10, 20), Point2_i(0, 20),
                          Point2_i(0, 0)},
                  Rel{1, 1, 0, 0, 1, 0, 1, 0, 0, 0});
  // point in the notch of the L
  ExpectRelations(Point2_i(15, 15),
                  Ring2_i{Point2_i(0, 0), Point2_i(20, 0), Point2_i(20, 10),
                          Point2_i(10, 10), Point2_i(10, 20), Point2_i(0, 20),
                          Point2_i(0, 0)},
                  Rel{0, 0, 0, 0, 0, 0, 0, 0, 1, 0});
  // point on the inner corner of the L
  ExpectRelations(Point2_i(10, 10),
                  Ring2_i{Point2_i(0, 0), Point2_i(20, 0), Point2_i(20, 10),
                          Point2_i(10, 10), Point2_i(10, 20), Point2_i(0, 20),
                          Point2_i(0, 0)},
                  Rel{1, 0, 0, 1, 0, 0, 1, 0, 0, 0});
  // point on an inner edge of the L
  ExpectRelations(Point2_i(15, 10),
                  Ring2_i{Point2_i(0, 0), Point2_i(20, 0), Point2_i(20, 10),
                          Point2_i(10, 10), Point2_i(10, 20), Point2_i(0, 20),
                          Point2_i(0, 0)},
                  Rel{1, 0, 0, 1, 0, 0, 1, 0, 0, 0});
  // point on an outer edge of the L
  ExpectRelations(Point2_i(0, 15),
                  Ring2_i{Point2_i(0, 0), Point2_i(20, 0), Point2_i(20, 10),
                          Point2_i(10, 10), Point2_i(10, 20), Point2_i(0, 20),
                          Point2_i(0, 0)},
                  Rel{1, 0, 0, 1, 0, 0, 1, 0, 0, 0});
}

TEST(Predicates, SegmentSegment) {
  // crossing at both interiors
  ExpectRelations(Segment2_i(5, -5, 5, 5), Segment2_i(0, 0, 10, 0),
                  Rel{1, 1, 0, 0, 0, 0, 0, 0, 0, 0});
  // one ends inside the other
  ExpectRelations(Segment2_i(5, 0, 5, 5), Segment2_i(0, 0, 10, 0),
                  Rel{1, 0, 0, 1, 0, 0, 0, 0, 0, 0});
  // end to end, collinear
  ExpectRelations(Segment2_i(10, 0, 20, 0), Segment2_i(0, 0, 10, 0),
                  Rel{1, 0, 0, 1, 0, 0, 0, 0, 0, 0});
  // end to end, at an angle
  ExpectRelations(Segment2_i(10, 0, 15, 5), Segment2_i(0, 0, 10, 0),
                  Rel{1, 0, 0, 1, 0, 0, 0, 0, 0, 0});
  // collinear, partly overlapping
  ExpectRelations(Segment2_i(5, 0, 15, 0), Segment2_i(0, 0, 10, 0),
                  Rel{1, 1, 1, 0, 0, 0, 0, 0, 0, 0});
  // collinear, one inside the other
  ExpectRelations(Segment2_i(2, 0, 8, 0), Segment2_i(0, 0, 10, 0),
                  Rel{1, 1, 0, 0, 1, 0, 1, 0, 0, 0});
  // equal
  ExpectRelations(Segment2_i(0, 0, 10, 0), Segment2_i(0, 0, 10, 0),
                  Rel{1, 1, 0, 0, 1, 1, 1, 1, 0, 1});
  // equal, reversed
  ExpectRelations(Segment2_i(10, 0, 0, 0), Segment2_i(0, 0, 10, 0),
                  Rel{1, 1, 0, 0, 1, 1, 1, 1, 0, 1});
  // parallel, apart
  ExpectRelations(Segment2_i(0, 5, 10, 5), Segment2_i(0, 0, 10, 0),
                  Rel{0, 0, 0, 0, 0, 0, 0, 0, 1, 0});
  // diagonals crossing between grid points
  ExpectRelations(Segment2_i(0, 0, 4, 4), Segment2_i(0, 3, 3, 0),
                  Rel{1, 1, 0, 0, 0, 0, 0, 0, 0, 0});
}

TEST(Predicates, SegmentBox) {
  // inside
  ExpectRelations(Segment2_i(2, 2, 8, 8), Box2_i(0, 0, 10, 10),
                  Rel{1, 1, 0, 0, 1, 0, 1, 0, 0, 0});
  // inside, one end on an edge
  ExpectRelations(Segment2_i(0, 5, 5, 5), Box2_i(0, 0, 10, 10),
                  Rel{1, 1, 0, 0, 1, 0, 1, 0, 0, 0});
  // crossing through
  ExpectRelations(Segment2_i(-5, 5, 15, 5), Box2_i(0, 0, 10, 10),
                  Rel{1, 1, 0, 0, 0, 0, 0, 0, 0, 0});
  // along part of an edge
  ExpectRelations(Segment2_i(2, 0, 8, 0), Box2_i(0, 0, 10, 10),
                  Rel{1, 0, 0, 1, 0, 0, 1, 0, 0, 0});
  // along a whole edge
  ExpectRelations(Segment2_i(0, 0, 10, 0), Box2_i(0, 0, 10, 10),
                  Rel{1, 0, 0, 1, 0, 0, 1, 0, 0, 0});
  // along an edge and past both corners
  ExpectRelations(Segment2_i(10, -5, 10, 15), Box2_i(0, 0, 10, 10),
                  Rel{1, 0, 0, 1, 0, 0, 0, 0, 0, 0});
  // outside, one end on an edge
  ExpectRelations(Segment2_i(10, 5, 20, 5), Box2_i(0, 0, 10, 10),
                  Rel{1, 0, 0, 1, 0, 0, 0, 0, 0, 0});
  // outside, one end on a corner
  ExpectRelations(Segment2_i(10, 10, 20, 20), Box2_i(0, 0, 10, 10),
                  Rel{1, 0, 0, 1, 0, 0, 0, 0, 0, 0});
  // diagonal entering through a corner
  ExpectRelations(Segment2_i(-5, -5, 5, 5), Box2_i(0, 0, 10, 10),
                  Rel{1, 1, 0, 0, 0, 0, 0, 0, 0, 0});
  // apart
  ExpectRelations(Segment2_i(20, 0, 30, 0), Box2_i(0, 0, 10, 10),
                  Rel{0, 0, 0, 0, 0, 0, 0, 0, 1, 0});
}

TEST(Predicates, SegmentRing) {
  // inside the square
  ExpectRelations(Segment2_i(2, 2, 8, 8),
                  Ring2_i{Point2_i(0, 0), Point2_i(10, 0), Point2_i(10, 10),
                          Point2_i(0, 10), Point2_i(0, 0)},
                  Rel{1, 1, 0, 0, 1, 0, 1, 0, 0, 0});
  // crossing the square
  ExpectRelations(Segment2_i(-5, 5, 15, 5),
                  Ring2_i{Point2_i(0, 0), Point2_i(10, 0), Point2_i(10, 10),
                          Point2_i(0, 10), Point2_i(0, 0)},
                  Rel{1, 1, 0, 0, 0, 0, 0, 0, 0, 0});
  // along an edge of the square
  ExpectRelations(Segment2_i(0, 0, 10, 0),
                  Ring2_i{Point2_i(0, 0), Point2_i(10, 0), Point2_i(10, 10),
                          Point2_i(0, 10), Point2_i(0, 0)},
                  Rel{1, 0, 0, 1, 0, 0, 1, 0, 0, 0});
  // outside the square, one end on an edge
  ExpectRelations(Segment2_i(10, 5, 20, 5),
                  Ring2_i{Point2_i(0, 0), Point2_i(10, 0), Point2_i(10, 10),
                          Point2_i(0, 10), Point2_i(0, 0)},
                  Rel{1, 0, 0, 1, 0, 0, 0, 0, 0, 0});
  // apart from the square
  ExpectRelations(Segment2_i(20, 0, 30, 0),
                  Ring2_i{Point2_i(0, 0), Point2_i(10, 0), Point2_i(10, 10),
                          Point2_i(0, 10), Point2_i(0, 0)},
                  Rel{0, 0, 0, 0, 0, 0, 0, 0, 1, 0});
  // across the notch of the L
  ExpectRelations(Segment2_i(5, 15, 15, 15),
                  Ring2_i{Point2_i(0, 0), Point2_i(20, 0), Point2_i(20, 10),
                          Point2_i(10, 10), Point2_i(10, 20), Point2_i(0, 20),
                          Point2_i(0, 0)},
                  Rel{1, 1, 0, 0, 0, 0, 0, 0, 0, 0});
  // in the notch, one end on the inner corner
  ExpectRelations(Segment2_i(15, 15, 10, 10),
                  Ring2_i{Point2_i(0, 0), Point2_i(20, 0), Point2_i(20, 10),
                          Point2_i(10, 10), Point2_i(10, 20), Point2_i(0, 20),
                          Point2_i(0, 0)},
                  Rel{1, 0, 0, 1, 0, 0, 0, 0, 0, 0});
  // along an inner edge of the L
  ExpectRelations(Segment2_i(10, 10, 10, 20),
                  Ring2_i{Point2_i(0, 0), Point2_i(20, 0), Point2_i(20, 10),
                          Point2_i(10, 10), Point2_i(10, 20), Point2_i(0, 20),
                          Point2_i(0, 0)},
                  Rel{1, 0, 0, 1, 0, 0, 1, 0, 0, 0});
  // inside the L, around its inner corner
  ExpectRelations(Segment2_i(5, 15, 15, 5),
                  Ring2_i{Point2_i(0, 0), Point2_i(20, 0), Point2_i(20, 10),
                          Point2_i(10, 10), Point2_i(10, 20), Point2_i(0, 20),
                          Point2_i(0, 0)},
                  Rel{1, 1, 0, 0, 1, 0, 1, 0, 0, 0});
}

TEST(Predicates, BoxBox) {
  // equal
  ExpectRelations(Box2_i(0, 0, 10, 10), Box2_i(0, 0, 10, 10),
                  Rel{1, 1, 0, 0, 1, 1, 1, 1, 0, 1});
  // partial overlap
  ExpectRelations(Box2_i(0, 0, 10, 10), Box2_i(5, 5, 15, 15),
                  Rel{1, 1, 1, 0, 0, 0, 0, 0, 0, 0});
  // inside at a corner
  ExpectRelations(Box2_i(0, 0, 1, 1), Box2_i(0, 0, 10, 10),
                  Rel{1, 1, 0, 0, 1, 0, 1, 0, 0, 0});
  // strictly inside
  ExpectRelations(Box2_i(1, 1, 2, 2), Box2_i(0, 0, 10, 10),
                  Rel{1, 1, 0, 0, 1, 0, 1, 0, 0, 0});
  // inside, sharing an edge
  ExpectRelations(Box2_i(0, 2, 5, 8), Box2_i(0, 0, 10, 10),
                  Rel{1, 1, 0, 0, 1, 0, 1, 0, 0, 0});
  // wider, same height
  ExpectRelations(Box2_i(-1, 0, 11, 10), Box2_i(0, 0, 10, 10),
                  Rel{1, 1, 0, 0, 0, 1, 0, 1, 0, 0});
  // larger, sharing a corner
  ExpectRelations(Box2_i(0, 0, 11, 11), Box2_i(0, 0, 10, 10),
                  Rel{1, 1, 0, 0, 0, 1, 0, 1, 0, 0});
  // strictly larger
  ExpectRelations(Box2_i(-1, -1, 11, 11), Box2_i(0, 0, 10, 10),
                  Rel{1, 1, 0, 0, 0, 1, 0, 1, 0, 0});
  // band across
  ExpectRelations(Box2_i(-1, 4, 11, 6), Box2_i(0, 0, 10, 10),
                  Rel{1, 1, 1, 0, 0, 0, 0, 0, 0, 0});
  // touching along an edge
  ExpectRelations(Box2_i(0, 0, 10, 10), Box2_i(10, 0, 20, 10),
                  Rel{1, 0, 0, 1, 0, 0, 0, 0, 0, 0});
  // touching along the top edge, from above
  ExpectRelations(Box2_i(0, 0, 10, 10), Box2_i(0, 10, 10, 11),
                  Rel{1, 0, 0, 1, 0, 0, 0, 0, 0, 0});
  // touching at a corner
  ExpectRelations(Box2_i(0, 0, 10, 10), Box2_i(10, 10, 20, 20),
                  Rel{1, 0, 0, 1, 0, 0, 0, 0, 0, 0});
  // zero width, from a corner
  ExpectRelations(Box2_i(0, 0, 10, 10), Box2_i(0, 10, 0, 20),
                  Rel{1, 0, 0, 1, 0, 0, 0, 0, 0, 0});
  // zero size, at a corner
  ExpectRelations(Box2_i(0, 0, 10, 10), Box2_i(0, 0, 0, 0),
                  Rel{1, 0, 0, 1, 0, 0, 0, 1, 0, 0});
  // zero height, along the top edge and beyond
  ExpectRelations(Box2_i(0, 0, 10, 10), Box2_i(0, 10, 200, 10),
                  Rel{1, 0, 0, 1, 0, 0, 0, 0, 0, 0});
  // apart
  ExpectRelations(Box2_i(0, 0, 10, 10), Box2_i(20, 20, 30, 30),
                  Rel{0, 0, 0, 0, 0, 0, 0, 0, 1, 0});
  // far away
  ExpectRelations(Box2_i(0, 0, 10, 10), Box2_i(100, 100, 200, 200),
                  Rel{0, 0, 0, 0, 0, 0, 0, 0, 1, 0});
}

TEST(Predicates, BoxRing) {
  // same square
  ExpectRelations(Box2_i(0, 0, 10, 10),
                  Ring2_i{Point2_i(0, 0), Point2_i(10, 0), Point2_i(10, 10),
                          Point2_i(0, 10), Point2_i(0, 0)},
                  Rel{1, 1, 0, 0, 1, 1, 1, 1, 0, 1});
  // inside the square
  ExpectRelations(Box2_i(2, 2, 8, 8),
                  Ring2_i{Point2_i(0, 0), Point2_i(10, 0), Point2_i(10, 10),
                          Point2_i(0, 10), Point2_i(0, 0)},
                  Rel{1, 1, 0, 0, 1, 0, 1, 0, 0, 0});
  // square with extra points on its edges
  ExpectRelations(
      Box2_i(0, 0, 10, 10),
      Ring2_i{Point2_i(0, 0), Point2_i(5, 0), Point2_i(10, 0), Point2_i(10, 10),
              Point2_i(0, 10), Point2_i(0, 5), Point2_i(0, 0)},
      Rel{1, 1, 0, 0, 1, 1, 1, 1, 0, 1});
  // inside an arm of the L
  ExpectRelations(Box2_i(2, 2, 8, 8),
                  Ring2_i{Point2_i(0, 0), Point2_i(20, 0), Point2_i(20, 10),
                          Point2_i(10, 10), Point2_i(10, 20), Point2_i(0, 20),
                          Point2_i(0, 0)},
                  Rel{1, 1, 0, 0, 1, 0, 1, 0, 0, 0});
  // exactly one arm of the L
  ExpectRelations(Box2_i(0, 0, 20, 10),
                  Ring2_i{Point2_i(0, 0), Point2_i(20, 0), Point2_i(20, 10),
                          Point2_i(10, 10), Point2_i(10, 20), Point2_i(0, 20),
                          Point2_i(0, 0)},
                  Rel{1, 1, 0, 0, 1, 0, 1, 0, 0, 0});
  // filling the notch of the L
  ExpectRelations(Box2_i(10, 10, 20, 20),
                  Ring2_i{Point2_i(0, 0), Point2_i(20, 0), Point2_i(20, 10),
                          Point2_i(10, 10), Point2_i(10, 20), Point2_i(0, 20),
                          Point2_i(0, 0)},
                  Rel{1, 0, 0, 1, 0, 0, 0, 0, 0, 0});
  // overlapping the notch of the L
  ExpectRelations(Box2_i(5, 5, 15, 15),
                  Ring2_i{Point2_i(0, 0), Point2_i(20, 0), Point2_i(20, 10),
                          Point2_i(10, 10), Point2_i(10, 20), Point2_i(0, 20),
                          Point2_i(0, 0)},
                  Rel{1, 1, 1, 0, 0, 0, 0, 0, 0, 0});
  // touching the L only at its outer corner
  ExpectRelations(Box2_i(20, 10, 25, 15),
                  Ring2_i{Point2_i(0, 0), Point2_i(20, 0), Point2_i(20, 10),
                          Point2_i(10, 10), Point2_i(10, 20), Point2_i(0, 20),
                          Point2_i(0, 0)},
                  Rel{1, 0, 0, 1, 0, 0, 0, 0, 0, 0});
  // enclosing the whole L
  ExpectRelations(Box2_i(-1, -1, 21, 21),
                  Ring2_i{Point2_i(0, 0), Point2_i(20, 0), Point2_i(20, 10),
                          Point2_i(10, 10), Point2_i(10, 20), Point2_i(0, 20),
                          Point2_i(0, 0)},
                  Rel{1, 1, 0, 0, 0, 1, 0, 1, 0, 0});
  // apart from the L
  ExpectRelations(Box2_i(30, 30, 40, 40),
                  Ring2_i{Point2_i(0, 0), Point2_i(20, 0), Point2_i(20, 10),
                          Point2_i(10, 10), Point2_i(10, 20), Point2_i(0, 20),
                          Point2_i(0, 0)},
                  Rel{0, 0, 0, 0, 0, 0, 0, 0, 1, 0});
}

TEST(Predicates, RingRing) {
  // same square
  ExpectRelations(Ring2_i{Point2_i(0, 0), Point2_i(10, 0), Point2_i(10, 10),
                          Point2_i(0, 10), Point2_i(0, 0)},
                  Ring2_i{Point2_i(0, 0), Point2_i(10, 0), Point2_i(10, 10),
                          Point2_i(0, 10), Point2_i(0, 0)},
                  Rel{1, 1, 0, 0, 1, 1, 1, 1, 0, 1});
  // same square, starting at another corner
  ExpectRelations(Ring2_i{Point2_i(0, 0), Point2_i(10, 0), Point2_i(10, 10),
                          Point2_i(0, 10), Point2_i(0, 0)},
                  Ring2_i{Point2_i(10, 0), Point2_i(10, 10), Point2_i(0, 10),
                          Point2_i(0, 0), Point2_i(10, 0)},
                  Rel{1, 1, 0, 0, 1, 1, 1, 1, 0, 1});
  // same square, with extra points on its edges
  ExpectRelations(
      Ring2_i{Point2_i(0, 0), Point2_i(10, 0), Point2_i(10, 10),
              Point2_i(0, 10), Point2_i(0, 0)},
      Ring2_i{Point2_i(0, 0), Point2_i(5, 0), Point2_i(10, 0), Point2_i(10, 10),
              Point2_i(0, 10), Point2_i(0, 5), Point2_i(0, 0)},
      Rel{1, 1, 0, 0, 1, 1, 1, 1, 0, 1});
  // square and a slightly larger square
  ExpectRelations(
      Ring2_i{Point2_i(0, 0), Point2_i(10, 0), Point2_i(10, 10),
              Point2_i(0, 10), Point2_i(0, 0)},
      Ring2_i{Point2_i(0, 0), Point2_i(5, 0), Point2_i(11, 0), Point2_i(11, 11),
              Point2_i(0, 11), Point2_i(0, 5), Point2_i(0, 0)},
      Rel{1, 1, 0, 0, 1, 0, 1, 0, 0, 0});
  // square strictly inside the L
  ExpectRelations(Ring2_i{Point2_i(2, 2), Point2_i(8, 2), Point2_i(8, 8),
                          Point2_i(2, 8), Point2_i(2, 2)},
                  Ring2_i{Point2_i(0, 0), Point2_i(20, 0), Point2_i(20, 10),
                          Point2_i(10, 10), Point2_i(10, 20), Point2_i(0, 20),
                          Point2_i(0, 0)},
                  Rel{1, 1, 0, 0, 1, 0, 1, 0, 0, 0});
  // square inside the L, sharing edges
  ExpectRelations(Ring2_i{Point2_i(0, 0), Point2_i(10, 0), Point2_i(10, 10),
                          Point2_i(0, 10), Point2_i(0, 0)},
                  Ring2_i{Point2_i(0, 0), Point2_i(20, 0), Point2_i(20, 10),
                          Point2_i(10, 10), Point2_i(10, 20), Point2_i(0, 20),
                          Point2_i(0, 0)},
                  Rel{1, 1, 0, 0, 1, 0, 1, 0, 0, 0});
  // L overlapping a shifted square
  ExpectRelations(Ring2_i{Point2_i(0, 0), Point2_i(20, 0), Point2_i(20, 10),
                          Point2_i(10, 10), Point2_i(10, 20), Point2_i(0, 20),
                          Point2_i(0, 0)},
                  Ring2_i{Point2_i(5, 5), Point2_i(15, 5), Point2_i(15, 15),
                          Point2_i(5, 15), Point2_i(5, 5)},
                  Rel{1, 1, 1, 0, 0, 0, 0, 0, 0, 0});
  // square touching the L along an edge
  ExpectRelations(Ring2_i{Point2_i(20, 0), Point2_i(30, 0), Point2_i(30, 10),
                          Point2_i(20, 10), Point2_i(20, 0)},
                  Ring2_i{Point2_i(0, 0), Point2_i(20, 0), Point2_i(20, 10),
                          Point2_i(10, 10), Point2_i(10, 20), Point2_i(0, 20),
                          Point2_i(0, 0)},
                  Rel{1, 0, 0, 1, 0, 0, 0, 0, 0, 0});
  // square touching the L at one corner
  ExpectRelations(Ring2_i{Point2_i(20, 10), Point2_i(25, 10), Point2_i(25, 15),
                          Point2_i(20, 15), Point2_i(20, 10)},
                  Ring2_i{Point2_i(0, 0), Point2_i(20, 0), Point2_i(20, 10),
                          Point2_i(10, 10), Point2_i(10, 20), Point2_i(0, 20),
                          Point2_i(0, 0)},
                  Rel{1, 0, 0, 1, 0, 0, 0, 0, 0, 0});
  // apart
  ExpectRelations(Ring2_i{Point2_i(0, 0), Point2_i(10, 0), Point2_i(10, 10),
                          Point2_i(0, 10), Point2_i(0, 0)},
                  Ring2_i{Point2_i(30, 30), Point2_i(40, 30), Point2_i(40, 40),
                          Point2_i(30, 40), Point2_i(30, 30)},
                  Rel{0, 0, 0, 0, 0, 0, 0, 0, 1, 0});
}

}  // namespace
}  // namespace moab
