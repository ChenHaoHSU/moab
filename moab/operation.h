#ifndef MOAB_OPERATION_H_
#define MOAB_OPERATION_H_

#include "boost/geometry.hpp"
#include "boost/polygon/polygon.hpp"

namespace moab {

template <typename T>
class Point2;

template <typename T>
class Interval;

template <typename T>
class Box2;

template <typename T>
class Ring2;

// Geometry operations on moab types, as thin wrappers over Boost.
//
// The set operations and set functions use Boost.Polygon and treat their
// arguments as sets of shapes: a std::vector<Box2_i> is a set of rectangles
// and a std::vector<Ring2_i> a set of polygons. See
//   https://www.boost.org/doc/libs/1_90_0/libs/polygon/doc/gtl_polygon_set_concept.htm
//
// The predicates use Boost.Geometry and compare two single geometries.

// ==================================
// Boost polygon set operators
// ==================================
// Each operator overwrites 'lhs' with its result. 'lhs' is a container such as
// std::vector<Box2_i> or std::vector<Ring2_i>, and its type decides the form
// of the result: non-overlapping rectangles for Box2_i, merged polygons for
// Ring2_i. 'rhs' is a single shape or a container of shapes. A Box2_i
// container accepts only rectangles; use a Ring2_i container for rings. How
// the result is cut into shapes, and their order, are up to Boost.Polygon.
//
// For std::vector<Box2_i> lhs = {(0 0)-(10 10)} and rhs = {(5 5)-(15 15)}:
//   UnionSet          {(0 0)-(10 5), (0 5)-(15 10), (5 10)-(15 15)}
//   IntersectionSet   {(5 5)-(10 10)}
//   SubtractSet       {(0 0)-(10 5), (0 5)-(5 10)}
//   DisjointUnionSet  {(0 0)-(10 5), (0 5)-(5 10), (10 5)-(15 10),
//                      (5 10)-(15 15)}

// Sets lhs to the union of lhs and rhs: the area in either. Same as lhs |= rhs
// (or lhs += rhs) in Boost.Polygon.
constexpr auto UnionSet = [](auto& lhs, const auto& rhs) constexpr {
  return boost::polygon::operators::operator|=(lhs, rhs);
};

// Sets lhs to the intersection of lhs and rhs: the area in both. Same as
// lhs &= rhs (or lhs *= rhs).
constexpr auto IntersectionSet = [](auto& lhs, const auto& rhs) constexpr {
  return boost::polygon::operators::operator&=(lhs, rhs);
};

// Sets lhs to the symmetric difference (XOR) of lhs and rhs: the area in
// exactly one of them. Same as lhs ^= rhs.
constexpr auto DisjointUnionSet = [](auto& lhs, const auto& rhs) constexpr {
  return boost::polygon::operators::operator^=(lhs, rhs);
};

// Sets lhs to lhs minus rhs: the area in lhs but not in rhs. Same as
// lhs -= rhs.
constexpr auto SubtractSet = [](auto& lhs, const auto& rhs) constexpr {
  return boost::polygon::operators::operator-=(lhs, rhs);
};

// ==================================
// Boost polygon set functions
// ==================================
// Sets lhs to the area covered by rhs, with overlaps merged, in lhs's form
// (see the set operators above). Assigning {(0 0)-(10 10), (5 5)-(15 15)}
// gives three non-overlapping rectangles in a std::vector<Box2_i>, and one
// 8-corner polygon in a std::vector<Ring2_i>.
constexpr auto Assign = [](auto& lhs, const auto& rhs) constexpr {
  return boost::polygon::assign(lhs, rhs);
};

// Returns true if lhs and rhs cover exactly the same area, however it is
// divided into shapes: {(0 0)-(10 10)} is equivalent to
// {(0 0)-(5 10), (5 0)-(10 10)}. To compare two single geometries, use
// IsEqual.
constexpr auto Equivalence = [](const auto& lhs,
                                const auto& rhs) constexpr -> bool {
  return boost::polygon::equivalence(lhs, rhs);
};

// Returns the area covered by 'set', counting overlaps once:
// {(0 0)-(10 10), (5 5)-(15 15)} has area 175, not 200.
constexpr auto Area = [](const auto& set) constexpr {
  return boost::polygon::area(set);
};

// ==================================
// Boost geometry algorithms functions
// ==================================
// The predicates take two single geometries (Point2, Segment2, Box2, Ring2)
// and differ in how they treat touching and containment. For a = (0 0)-(10 10)
// and boxes b:
//
//   b                          Intersect  Strictly   Overlap  Touch  Within
//                                         Intersect                  (b, a)
//   partial overlap                1          1         1       0      0
//   inside a                       1          1         0       0      1
//   inside a, sharing an edge      1          1         0       0      1
//   equal to a                     1          1         0       0      1
//   touching a along an edge       1          0         0       1      0
//   touching a at a corner         1          0         0       1      0
//   apart from a                   0          0         0       0      0
//
// IsDisjoint is the opposite of IsIntersect. Boost.Geometry implements touches,
// overlaps and within only for some pairs of types, so each predicate below
// lists the pairs it supports among Box2, Ring2 and Point2. Some other pairs
// compile too, such as two points, but notably Point2 or Segment2 with Box2
// does not compile for IsTouch or IsStrictlyIntersect.

// Returns true if no point of g1 is outside g2: g1 lies inside g2 or on its
// border. Unlike IsWithin, a point on g2's border is covered by g2.
// Supported: Point2 by any type; Box2 or Ring2 by Box2 or Ring2.
constexpr auto IsCoveredBy = [](const auto& g1,
                                const auto& g2) constexpr -> bool {
  return boost::geometry::covered_by(g1, g2);
};

// Returns true if g1 and g2 share at least one point, including touching at
// an edge or a corner. Supported: all pairs.
constexpr auto IsIntersect = [](const auto& g1,
                                const auto& g2) constexpr -> bool {
  return boost::geometry::intersects(g1, g2);
};

// Returns true if g1 lies inside g2: no point of g1 is outside g2, and their
// interiors meet. A box that shares an edge with g2, or equals it, is within
// g2, but a point on g2's border is not (see IsCoveredBy).
// Supported: Point2 in any type; Box2 in Box2; Ring2 in Ring2.
constexpr auto IsWithin = [](const auto& g1, const auto& g2) constexpr -> bool {
  return boost::geometry::within(g1, g2);
};

// Returns true if g1 and g2 overlap partially: the overlap has positive area
// and neither contains the other. Touching, containment and equal
// geometries do not overlap; IsStrictlyIntersect counts containment.
// Supported: Box2 with Box2 or Ring2 (in that order); Ring2 with Ring2.
constexpr auto IsOverlap = [](const auto& g1,
                              const auto& g2) constexpr -> bool {
  return boost::geometry::overlaps(g1, g2);
};

// Returns true if g1 and g2 meet only on their borders: they share at least
// one point, but their interiors do not overlap.
// Supported: Box2 with Box2; Ring2 with Ring2; Point2 with Segment2 or Ring2.
// Not Point2 or Segment2 with Box2.
constexpr auto IsTouch = [](const auto& g1, const auto& g2) constexpr -> bool {
  return boost::geometry::touches(g1, g2);
};

// Returns true if g1 and g2 share no point; the opposite of IsIntersect.
// Supported: all pairs.
constexpr auto IsDisjoint = [](const auto& g1,
                               const auto& g2) constexpr -> bool {
  return boost::geometry::disjoint(g1, g2);
};

// Returns true if g1 and g2 cover the same points, even if they are written
// differently, such as rings that start at different points. To compare
// sets of shapes, use Equivalence. Supported: all pairs.
constexpr auto IsEqual = [](const auto& g1, const auto& g2) constexpr -> bool {
  return boost::geometry::equals(g1, g2);
};

// Returns true if the interiors of g1 and g2 overlap, so their overlap has
// positive area. Unlike IsIntersect, touching at an edge or a corner does
// not count; unlike IsOverlap, containment and equal geometries do.
// Supported: Box2 with Box2; Ring2 with Ring2 (it uses IsTouch).
constexpr auto IsStrictlyIntersect = [](const auto& g1,
                                        const auto& g2) constexpr -> bool {
  return IsIntersect(g1, g2) && !IsTouch(g1, g2);
};

}  // namespace moab

#endif  // MOAB_OPERATION_H_