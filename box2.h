#ifndef MOAB_BOX2_H_
#define MOAB_BOX2_H_

#include <array>
#include <cstdint>

#include "absl/log/check.h"
#include "boost/geometry/core/access.hpp"
#include "boost/geometry/core/coordinate_dimension.hpp"
#include "boost/geometry/core/coordinate_system.hpp"
#include "boost/geometry/core/cs.hpp"
#include "boost/geometry/core/tag.hpp"
#include "boost/geometry/geometries/concepts/box_concept.hpp"
#include "boost/polygon/polygon.hpp"
#include "interval.h"
#include "point2.h"

namespace moab {

template <typename T>
class Box2 {
 public:
  // Type aliases.
  using coordinate_type = T;
  using point_type = Point2<T>;
  using interval_type = Interval<T>;

  // Constructors.
  Box2() : d_({point_type(0, 0), point_type(0, 0)}) {}
  explicit Box2(const point_type& p1, const point_type& p2) { Set(p1, p2); }
  explicit Box2(coordinate_type xl, coordinate_type yl, coordinate_type xh,
                coordinate_type yh) {
    Set(point_type(xl, yl), point_type(xh, yh));
  }
  Box2(const Box2& b) = default;
  Box2(Box2&& b) = default;
  ~Box2() = default;

  // Accessors.
  point_type& ll() { return d_[0]; }
  const point_type& ll() const { return d_[0]; }
  point_type& ur() { return d_[1]; }
  const point_type& ur() const { return d_[1]; }

  point_type& MinCorner() { return d_[0]; }
  const point_type& MinCorner() const { return d_[0]; }
  point_type& MaxCorner() { return d_[1]; }
  const point_type& MaxCorner() const { return d_[1]; }

  coordinate_type xl() const { return d_[0].x(); }
  coordinate_type yl() const { return d_[0].y(); }
  coordinate_type xh() const { return d_[1].x(); }
  coordinate_type yh() const { return d_[1].y(); }

  coordinate_type MinX() const { return d_[0].x(); }
  coordinate_type MinY() const { return d_[0].y(); }
  coordinate_type MaxX() const { return d_[1].x(); }
  coordinate_type MaxY() const { return d_[1].y(); }

  coordinate_type Width() const { return d_[1].x() - d_[0].x(); }
  coordinate_type Height() const { return d_[1].y() - d_[0].y(); }

  coordinate_type CenterX() const { return (d_[0].x() + d_[1].x()) / 2; }
  coordinate_type CenterY() const { return (d_[0].y() + d_[1].y()) / 2; }

  coordinate_type Area() const { return Width() * Height(); }
  coordinate_type HalfPerimeter() const { return (Width() + Height()); }
  coordinate_type Perimeter() const { return 2 * (Width() + Height()); }

  // Mutators.
  void Set(coordinate_type xl, coordinate_type yl, coordinate_type xh,
           coordinate_type yh) {
    DCHECK(xl <= xh && yl <= yh) << "Invalid box. xl: " << xl << ", yl: " << yl
                                 << ", xh: " << xh << ", yh: " << yh;
    d_[0].Set(xl, yl);
    d_[1].Set(xh, yh);
  }
  void Set(const point_type& ll, const point_type& ur) {
    Set(ll.x(), ll.y(), ur.x(), ur.y());
  }

  void set_xl(coordinate_type xl) { Set(xl, yl(), xh(), yh()); }
  void set_yl(coordinate_type yl) { Set(xl(), yl, xh(), yh()); }
  void set_xh(coordinate_type xh) { Set(xl(), yl(), xh, yh()); }
  void set_yh(coordinate_type yh) { Set(xl(), yl(), xh(), yh); }
  void set_ll(const point_type& p) { Set(p.x(), p.y(), xh(), yh()); }
  void set_ur(const point_type& p) { Set(xl(), yl(), p.x(), p.y()); }

  void SetMinCorner(const point_type& p) { Set(p.x(), p.y(), xh(), yh()); }
  void SetMaxCorner(const point_type& p) { Set(xl(), yl(), p.x(), p.y()); }

  // Operations.
  // Operations - Shift (It's safe. No need to check for validity.)
  void Shift(coordinate_type dx, coordinate_type dy) {
    d_[0].Shift(dx, dy);
    d_[1].Shift(dx, dy);
  }
  void ShiftX(coordinate_type dx) {
    d_[0].ShiftX(dx);
    d_[1].ShiftX(dx);
  }
  void ShiftY(coordinate_type dy) {
    d_[0].ShiftY(dy);
    d_[1].ShiftY(dy);
  }
  // Operations - Expand (It's unsafe. Need to check for validity.)
  void Expand(coordinate_type d) {
    Set(xl() - d, yl() - d, xh() + d, yh() + d);
  }
  void Expand(coordinate_type dx, coordinate_type dy) {
    Set(xl() - dx, yl() - dy, xh() + dx, yh() + dy);
  }
  void ExpandX(coordinate_type dx) { Set(xl() - dx, yl(), xh() + dx, yh()); }
  void ExpandY(coordinate_type dy) { Set(xl(), yl() - dy, xh(), yh() + dy); }

  // Operators.
  // Operators - Assignment
  Box2& operator=(const Box2& b) {
    d_ = b.d_;
    return *this;
  }
  Box2& operator=(Box2&& b) {
    d_ = std::move(b.d_);
    return *this;
  }
  // Operators - Subscript
  point_type& operator[](std::size_t i) { return d_[i]; }
  const point_type& operator[](std::size_t i) const { return d_.at(i); }
  // Operators - Equality
  bool operator==(const Box2& b) const {
    return d_[0] == b.d_[0] && d_[1] == b.d_[1];
  }
  bool operator!=(const Box2& b) const { return !(*this == b); }
  // Operators - Inequality
  bool operator<(const Box2& b) const {
    return d_[0] != b.d_[0] ? d_[0] < b.d_[0] : d_[1] < b.d_[1];
  }
  bool operator>(const Box2& b) const { return b < *this; }
  bool operator<=(const Box2& b) const { return !(*this > b); }
  bool operator>=(const Box2& b) const { return !(*this < b); }

  // String conversion.
  template <typename Sink>
  friend void AbslStringify(Sink& sink, const Box2& b) {
    absl::Format(&sink, "(%v %v)", b.d_[0], b.d_[1]);
  }
  std::string ToString() const { return absl::StrCat(*this); }
  friend std::ostream& operator<<(std::ostream& os, const Box2& b) {
    os << b.ToString();
    return os;
  }

  // Hash.
  template <typename H>
  friend H AbslHashValue(H h, const Box2& b) {
    return H::combine(std::move(h), b.d_[0], b.d_[1]);
  }

 private:
  std::array<point_type, 2> d_;
};  // class Box2

// Aliases.
using Box2_i = Box2<int>;
using Box2_i32 = Box2<int32_t>;
using Box2_i64 = Box2<int64_t>;

}  // namespace moab

// Boost geometry traits.
namespace boost::geometry::traits {

template <typename T>
struct tag<moab::Box2<T>> {
  using type = box_tag;
};

template <typename T>
struct indexed_access<moab::Box2<T>, min_corner, 0> {
  using box_type = moab::Box2<T>;
  using coordinate_type = typename box_type::coordinate_type;

  static inline coordinate_type get(box_type const& b) { return b[0][0]; }
  static inline void set(box_type& b, coordinate_type const& value) {
    b[0][0] = value;
  }
};

template <typename T>
struct indexed_access<moab::Box2<T>, min_corner, 1> {
  using box_type = moab::Box2<T>;
  using coordinate_type = typename box_type::coordinate_type;

  static inline coordinate_type get(box_type const& b) { return b[0][1]; }
  static inline void set(box_type& b, coordinate_type const& value) {
    b[0][1] = value;
  }
};

template <typename T>
struct indexed_access<moab::Box2<T>, max_corner, 0> {
  using box_type = moab::Box2<T>;
  using coordinate_type = typename box_type::coordinate_type;

  static inline coordinate_type get(box_type const& b) { return b[1][0]; }
  static inline void set(box_type& b, coordinate_type const& value) {
    b[1][0] = value;
  }
};

template <typename T>
struct indexed_access<moab::Box2<T>, max_corner, 1> {
  using box_type = moab::Box2<T>;
  using coordinate_type = typename box_type::coordinate_type;

  static inline coordinate_type get(box_type const& b) { return b[1][1]; }
  static inline void set(box_type& b, coordinate_type const& value) {
    b[1][1] = value;
  }
};

}  // namespace boost::geometry::traits

// Boost polygon traits.
namespace boost::polygon {

template <typename T>
struct geometry_concept<moab::Box2<T>> {
  using type = rectangle_concept;
};

template <typename T>
struct rectangle_traits<
    moab::Box2<T>, typename gtl_same_type<typename T::interval_type,
                                          typename T::interval_type>::type> {
  using rectangle_type = moab::Box2<T>;
  using coordinate_type = typename rectangle_type::coordinate_type;
  using interval_type = typename rectangle_type::interval_type;

  static inline interval_type get(const rectangle_type& rectangle,
                                  orientation_2d orient) {
    return interval_mutable_traits<interval_type>::construct(
        rectangle[0][orient.to_int()], rectangle[1][orient.to_int()]);
  }
};

template <typename T>
struct rectangle_mutable_traits<moab::Box2<T>> {
  using rectangle_type = moab::Box2<T>;
  using coordinate_type = typename rectangle_type::coordinate_type;
  using interval_type = typename rectangle_type::interval_type;

  template <typename T2>
  static inline void set(T& rectangle, orientation_2d orient,
                         const T2& interval) {
    rectangle[0][orient.to_int()] = interval_traits<T2>::get(interval, LOW);
    rectangle[1][orient.to_int()] = interval_traits<T2>::get(interval, HIGH);
  }
  template <typename T2, typename T3>
  static inline rectangle_type construct(const T2& interval_horizontal,
                                         const T3& interval_vertical) {
    return rectangle_type(interval_traits<T2>::get(interval_horizontal, LOW),
                          interval_traits<T3>::get(interval_vertical, LOW),
                          interval_traits<T2>::get(interval_horizontal, HIGH),
                          interval_traits<T3>::get(interval_vertical, HIGH));
  }
};

}  // namespace boost::polygon

#endif  // MOAB_BOX2_H_