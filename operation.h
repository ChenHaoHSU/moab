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

// Please refer to
//   https://www.boost.org/doc/libs/1_85_0/libs/polygon/doc/gtl_polygon_set_concept.htm
// for more information.

// ==================================
// Boost polygon set operators
// ==================================
// Polygon set union.
// lhs |= rhs; lhs += rhs;
constexpr auto UnionSet = [](auto& lhs, const auto& rhs) constexpr {
  return boost::polygon::operators::operator|=(lhs, rhs);
};

// Polygon set intersection.
// lhs &= rhs; lhs *= rhs;
constexpr auto IntersectionSet = [](auto& lhs, const auto& rhs) constexpr {
  return boost::polygon::operators::operator&=(lhs, rhs);
};

// Polygon set disjoint union (xor).
// lhs ^= rhs;
constexpr auto DisjointUnionSet = [](auto& lhs, const auto& rhs) constexpr {
  return boost::polygon::operators::operator^=(lhs, rhs);
};

// Polygon set subtraction.
// lhs -= rhs;
constexpr auto SubtractSet = [](auto& lhs, const auto& rhs) constexpr {
  return boost::polygon::operators::operator-=(lhs, rhs);
};

// ==================================
// Boost polygon set functions
// ==================================
// Polygon set assignment (refinement).
// Removes overlaps in geometry of rhs and copies rhs to lhs.
constexpr auto Assign = [](auto& lhs, const auto& rhs) constexpr {
  return boost::polygon::assign(lhs, rhs);
};

// Polygon set equivalence.
// Returns true if lhs and rhs are equivalent.
constexpr auto Equivalence = [](const auto& lhs,
                                const auto& rhs) constexpr -> bool {
  return boost::polygon::equivalence(lhs, rhs);
};

// Polygon set area.
// Returns the area of the polygon set.
constexpr auto Area = [](const auto& set) constexpr {
  return boost::polygon::area(set);
};

// ==================================
// Boost geometry algorithms functions
// ==================================

// Checks if the first geometry is inside or on border the second geometry.
constexpr auto IsCoveredBy = [](const auto& g1,
                                const auto& g2) constexpr -> bool {
  return boost::geometry::covered_by(g1, g2);
};

// Checks if two geometries have at least one intersection.
// Point/edge touch is considered as intersection.
// Point-Box: Point must be inside or on the border of the box.
// Box-Box
constexpr auto IsIntersect = [](const auto& g1,
                                const auto& g2) constexpr -> bool {
  return boost::geometry::intersects(g1, g2);
};

// Checks if the first geometry is completely inside the second geometry.
// Point-Box: Point must be inside the box. Border touch is not considered as
// inside.
// Box-Box: Box must be completely inside the box. Border touch is still
// considered as inside.
constexpr auto IsWithin = [](const auto& g1, const auto& g2) constexpr -> bool {
  return boost::geometry::within(g1, g2);
};

// Checks if two geometries overlap.
// Box-Box: the intersection of two boxes is not empty and not equal to either
// of the boxes.
constexpr auto IsOverlap = [](const auto& g1,
                              const auto& g2) constexpr -> bool {
  return boost::geometry::overlaps(g1, g2);
};

// Checks if two geometries have at least one touching point (tangent - non
// overlapping).
// Point-Box
// Box-Box
constexpr auto IsTouch = [](const auto& g1, const auto& g2) constexpr -> bool {
  return boost::geometry::touches(g1, g2);
};

// Checks if two geometries are disjoint.
constexpr auto IsDisjoint = [](const auto& g1,
                               const auto& g2) constexpr -> bool {
  return boost::geometry::disjoint(g1, g2);
};

// Checks if a geometry are spatially equal.
// Point-Point
// Box-Box
constexpr auto IsEqual = [](const auto& g1, const auto& g2) constexpr -> bool {
  return boost::geometry::equals(g1, g2);
};

}  // namespace moab

#endif  // MOAB_OPERATION_H_