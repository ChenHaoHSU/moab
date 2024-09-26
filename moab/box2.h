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
  // Type aliases. (Required by Boost geometry/polygon traits.)
  using coordinate_type = T;
  using point_type = Point2<T>;
  using interval_type = Interval<T>;

  // Constructors.
  Box2() : d_({Point2<T>(0, 0), Point2<T>(0, 0)}) {}
  explicit Box2(const Point2<T>& p1, const Point2<T>& p2) { Set(p1, p2); }
  explicit Box2(T xl, T yl, T xh, T yh) { Set(xl, yl, xh, yh); }
  Box2(const Box2&) = default;
  Box2(Box2&&) = default;
  ~Box2() = default;

  // Assignment operators.
  Box2& operator=(const Box2&) = default;
  Box2& operator=(Box2&&) = default;

  // Accessors.
  Point2<T>& ll() { return d_[0]; }
  const Point2<T>& ll() const { return d_[0]; }
  Point2<T>& ur() { return d_[1]; }
  const Point2<T>& ur() const { return d_[1]; }

  Point2<T>& MinCorner() { return d_[0]; }
  const Point2<T>& MinCorner() const { return d_[0]; }
  Point2<T>& MaxCorner() { return d_[1]; }
  const Point2<T>& MaxCorner() const { return d_[1]; }

  T xl() const { return d_[0].x(); }
  T yl() const { return d_[0].y(); }
  T xh() const { return d_[1].x(); }
  T yh() const { return d_[1].y(); }

  T MinX() const { return d_[0].x(); }
  T MinY() const { return d_[0].y(); }
  T MaxX() const { return d_[1].x(); }
  T MaxY() const { return d_[1].y(); }

  T Width() const { return d_[1].x() - d_[0].x(); }
  T Height() const { return d_[1].y() - d_[0].y(); }

  T CenterX() const { return (d_[0].x() + d_[1].x()) / 2; }
  T CenterY() const { return (d_[0].y() + d_[1].y()) / 2; }

  T Area() const { return Width() * Height(); }
  T HalfPerimeter() const { return (Width() + Height()); }
  T Perimeter() const { return 2 * (Width() + Height()); }

  // Mutators.
  void Set(T xl, T yl, T xh, T yh) {
    DCHECK(xl <= xh && yl <= yh) << "Invalid box. xl: " << xl << ", yl: " << yl
                                 << ", xh: " << xh << ", yh: " << yh;
    d_[0].Set(xl, yl);
    d_[1].Set(xh, yh);
  }
  void Set(const Point2<T>& ll, const Point2<T>& ur) {
    Set(ll.x(), ll.y(), ur.x(), ur.y());
  }

  void set_xl(T xl) { Set(xl, yl(), xh(), yh()); }
  void set_yl(T yl) { Set(xl(), yl, xh(), yh()); }
  void set_xh(T xh) { Set(xl(), yl(), xh, yh()); }
  void set_yh(T yh) { Set(xl(), yl(), xh(), yh); }
  void set_ll(const Point2<T>& p) { Set(p.x(), p.y(), xh(), yh()); }
  void set_ur(const Point2<T>& p) { Set(xl(), yl(), p.x(), p.y()); }

  void SetMinCorner(const Point2<T>& p) { Set(p.x(), p.y(), xh(), yh()); }
  void SetMaxCorner(const Point2<T>& p) { Set(xl(), yl(), p.x(), p.y()); }

  // Operations.
  // Operations - Shift (It's safe. No need to check for validity.)
  void Shift(T dx, T dy) {
    d_[0].Shift(dx, dy);
    d_[1].Shift(dx, dy);
  }
  void ShiftX(T dx) {
    d_[0].ShiftX(dx);
    d_[1].ShiftX(dx);
  }
  void ShiftY(T dy) {
    d_[0].ShiftY(dy);
    d_[1].ShiftY(dy);
  }
  // Operations - Expand (It's unsafe. Need to check for validity.)
  void Expand(T d) { Set(xl() - d, yl() - d, xh() + d, yh() + d); }
  void Expand(T dx, T dy) { Set(xl() - dx, yl() - dy, xh() + dx, yh() + dy); }
  void ExpandX(T dx) { Set(xl() - dx, yl(), xh() + dx, yh()); }
  void ExpandY(T dy) { Set(xl(), yl() - dy, xh(), yh() + dy); }

  // Operators.
  // Operators - Subscript
  Point2<T>& operator[](std::size_t i) { return d_[i]; }
  const Point2<T>& operator[](std::size_t i) const { return d_.at(i); }
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
  std::array<Point2<T>, 2> d_;
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
struct point_type<moab::Box2<T>> {
  using type = typename moab::Box2<T>::point_type;
};

// Index 0: min corner, Index 1: max corner.
// Dimension 0: x, Dimension 1: y.
template <typename T, std::size_t Index, std::size_t Dimension>
struct indexed_access<moab::Box2<T>, Index, Dimension> {
  using box_type = moab::Box2<T>;
  using coordinate_type = typename box_type::coordinate_type;

  static inline coordinate_type get(const box_type& b) {
    return b[Index][Dimension];
  }
  static inline void set(box_type& b, const coordinate_type& value) {
    b[Index][Dimension] = value;
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
    moab::Box2<T>,
    typename gtl_same_type<typename moab::Box2<T>::interval_type,
                           typename moab::Box2<T>::interval_type>::type> {
  using coordinate_type = typename moab::Box2<T>::coordinate_type;
  using interval_type = typename moab::Box2<T>::interval_type;

  static inline interval_type get(const moab::Box2<T>& rectangle,
                                  orientation_2d orient) {
    return interval_mutable_traits<interval_type>::construct(
        rectangle[0][orient.to_int()], rectangle[1][orient.to_int()]);
  }
};

template <typename T>
struct rectangle_mutable_traits<moab::Box2<T>> {
  using coordinate_type = typename moab::Box2<T>::coordinate_type;
  using interval_type = typename moab::Box2<T>::interval_type;

  template <typename T2>
  static inline void set(moab::Box2<T>& rectangle, orientation_2d orient,
                         const T2& interval) {
    rectangle[0][orient.to_int()] = interval_traits<T2>::get(interval, LOW);
    rectangle[1][orient.to_int()] = interval_traits<T2>::get(interval, HIGH);
  }
  template <typename T2, typename T3>
  static inline moab::Box2<T> construct(
      const interval_type& interval_horizontal,
      const interval_type& interval_vertical) {
    return moab::Box2<T>(interval_traits<T2>::get(interval_horizontal, LOW),
                         interval_traits<T3>::get(interval_vertical, LOW),
                         interval_traits<T2>::get(interval_horizontal, HIGH),
                         interval_traits<T3>::get(interval_vertical, HIGH));
  }
};

}  // namespace boost::polygon

#endif  // MOAB_BOX2_H_