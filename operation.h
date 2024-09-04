#ifndef MOAB_OPERATION_H_
#define MOAB_OPERATION_H_

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

// [Boost polygon set operators]
// Polygon set union.
// lhs |= rhs; lhs += rhs;
constexpr auto Union = [](auto&& lhs, auto&& rhs) constexpr -> decltype(auto) {
  return boost::polygon::operators::operator|=(
      std::forward<decltype(lhs)>(lhs), std::forward<decltype(rhs)>(rhs));
};

// Polygon set intersection.
// lhs &= rhs; lhs *= rhs;
constexpr auto Intersection = [](auto&& lhs,
                                 auto&& rhs) constexpr -> decltype(auto) {
  return boost::polygon::operators::operator&=(
      std::forward<decltype(lhs)>(lhs), std::forward<decltype(rhs)>(rhs));
};

// Polygon set disjoint union (xor).
// lhs ^= rhs;
constexpr auto DisjointUnion = [](auto&& lhs,
                                  auto&& rhs) constexpr -> decltype(auto) {
  return boost::polygon::operators::operator^=(
      std::forward<decltype(lhs)>(lhs), std::forward<decltype(rhs)>(rhs));
};

// Polygon set subtraction.
// lhs -= rhs;
constexpr auto Subtract = [](auto&& lhs,
                             auto&& rhs) constexpr -> decltype(auto) {
  return boost::polygon::operators::operator-=(
      std::forward<decltype(lhs)>(lhs), std::forward<decltype(rhs)>(rhs));
};

// [Boost polygon set functions]
// Polygon set assignment (refinement).
// Removes overlaps in geometry of rhs and copies rhs to lhs.
constexpr auto Assign = [](auto&& lhs, auto&& rhs) constexpr -> decltype(auto) {
  return boost::polygon::assign(std::forward<decltype(lhs)>(lhs),
                                std::forward<decltype(rhs)>(rhs));
};

// Polygon set equivalence.
// Returns true if lhs and rhs are equivalent.
constexpr auto Equivalence = [](auto&& lhs, auto&& rhs) constexpr -> bool {
  return boost::polygon::equivalence(std::forward<decltype(lhs)>(lhs),
                                     std::forward<decltype(rhs)>(rhs));
};

// Polygon set area.
// Returns the area of the polygon set.
constexpr auto Area = [](auto&& set) constexpr -> decltype(auto) {
  return boost::polygon::area(std::forward<decltype(set)>(set));
};

}  // namespace moab

#endif  // MOAB_OPERATION_H_