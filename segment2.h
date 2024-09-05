#ifndef MOAB_SEGMENT2_H_
#define MOAB_SEGMENT2_H_

#include <array>
#include <cstdint>
#include <ostream>
#include <string>
#include <utility>

#include "absl/hash/hash.h"
#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "boost/geometry/core/access.hpp"
#include "boost/geometry/core/coordinate_dimension.hpp"
#include "boost/geometry/core/coordinate_system.hpp"
#include "boost/geometry/core/cs.hpp"
#include "boost/geometry/core/tag.hpp"
#include "boost/geometry/geometries/concepts/segment_concept.hpp"
#include "boost/polygon/polygon.hpp"
#include "point2.h"

namespace moab {

template <typename T>
class Segment2 {
 public:
  // Type aliases. (Required by Boost geometry/poylgon traits.)
  using coordinate_type = T;
  using point_type = Point2<T>;

  // Constructors.
  Segment2() : d_({Point2<T>(0, 0), Point2<T>(0, 0)}) {}
  explicit Segment2(const Point2<T>& p0, const Point2<T>& p1) { Set(p0, p1); }
  explicit Segment2(T xl, T yl, T xh, T yh) {
    Set(Point2<T>(xl, yl), Point2<T>(xh, yh));
  }
  Segment2(const Segment2& s) = default;
  Segment2(Segment2&& s) = default;
  // Destructors.
  ~Segment2() = default;

  // Accessors.
  Point2<T>& p0() { return d_[0]; }
  const Point2<T>& p0() const { return d_.at(0); }
  Point2<T>& p1() { return d_[1]; }
  const Point2<T>& p1() const { return d_.at(1); }
  Point2<T>* data() { return d_.data(); }
  const Point2<T>* data() const { return d_.data(); }

  constexpr std::size_t Size() const { return d_.size(); }

  std::pair<Point2<T>, Point2<T>> ToPair() const { return {d_[0], d_[1]}; }

  // Mutators.
  void Set(T xl, T yl, T xh, T yh) {
    d_[0].Set(xl, yl);
    d_[1].Set(xh, yh);
  }
  void Set(const Point2<T>& p0, const Point2<T>& p1) {
    d_[0] = p0;
    d_[1] = p1;
  }
  void SetP0(const Point2<T>& p) { d_[0] = p; }
  void SetP1(const Point2<T>& p) { d_[1] = p; }
  void SetP(std::size_t i, const Point2<T>& p) {
    DCHECK(i < 2) << "Invalid index. i: " << i;
    d_[i] = p;
  }

  // Operations.
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

  // Operators.
  // Operators - Assignment
  Segment2& operator=(const Segment2& p) {
    d_ = p.d_;
    return *this;
  }
  Segment2& operator=(Segment2&& p) {
    d_ = std::move(p.d_);
    return *this;
  }
  // Operators - Subscript
  Point2<T>& operator[](std::size_t i) { return d_[i]; }
  const Point2<T>& operator[](std::size_t i) const { return d_.at(i); }
  // Operators - Equality
  bool operator==(const Segment2& p) const {
    return d_[0] == p.d_[0] && d_[1] == p.d_[1];
  }
  bool operator!=(const Segment2& p) const { return !(*this == p); }
  // Operators - Inequality
  bool operator<(const Segment2& p) const {
    return d_[0] != p.d_[0] ? d_[0] < p.d_[0] : d_[1] < p.d_[1];
  }
  bool operator>(const Segment2& p) const { return p < *this; }
  bool operator<=(const Segment2& p) const { return !(*this > p); }
  bool operator>=(const Segment2& p) const { return !(*this < p); }

  // String conversion.
  template <typename Sink>
  friend void AbslStringify(Sink& sink, const Segment2& s) {
    absl::Format(&sink, "(%v %v)", s.d_.at(0), s.d_.at(1));
  }
  std::string ToString() const { return absl::StrCat(*this); }
  friend std::ostream& operator<<(std::ostream& os, const Segment2& s) {
    os << s.ToString();
    return os;
  }

  // Hash.
  template <typename H>
  friend H AbslHashValue(H h, const Segment2& s) {
    return H::combine(std::move(h), s.d_.at(0), s.d_.at(1));
  }

 private:
  std::array<Point2<T>, 2> d_;  // <p0, p1>
};  // class Segment2

// Aliases.
using Segment2_i = Segment2<int>;
using Segment2_i32 = Segment2<int32_t>;
using Segment2_i64 = Segment2<int64_t>;

}  // namespace moab

// Boost geometry traits.
namespace boost::geometry::traits {

template <typename T>
struct tag<moab::Segment2<T>> {
  using type = segment_tag;
};

template <typename T>
struct dimension<moab::Segment2<T>> : boost::mpl::int_<2> {};

template <typename T>
struct coordinate_type<moab::Segment2<T>> {
  using type = T;
};

template <typename T>
struct coordinate_system<moab::Segment2<T>> {
  using type = boost::geometry::cs::cartesian;
};

template <typename T>
struct access<moab::Segment2<T>, 0> {
  using coordinate_type = typename moab::Segment2<T>::coordinate_type;
  using point_type = typename moab::Segment2<T>::point_type;

  static inline point_type get(const moab::Segment2<T>& s) { return s[0]; }
  static inline void set(moab::Segment2<T>& s, const point_type& p) {
    s[0] = p;
  }
};

template <typename T>
struct access<moab::Segment2<T>, 1> {
  using coordinate_type = typename moab::Segment2<T>::coordinate_type;
  using point_type = typename moab::Segment2<T>::point_type;

  static inline point_type get(const moab::Segment2<T>& s) { return s[1]; }
  static inline void set(moab::Segment2<T>& s, const point_type& p) {
    s[1] = p;
  }
};

}  // namespace boost::geometry::traits

// Boost poylgon traits.
namespace boost::polygon {

template <typename T>
struct geometry_concept<moab::Segment2<T>> {
  using type = segment_concept;
};

template <typename T>
struct segment_traits<moab::Segment2<T>> {
  using coordinate_type = typename moab::Segment2<T>::coordinate_type;
  using point_type = typename moab::Segment2<T>::point_type;

  static inline point_type get(const moab::Segment2<T>& s, direction_1d dir) {
    return s[dir.to_int()];
  }
};

template <typename T>
struct segment_mutable_traits<moab::Segment2<T>> {
  using coordinate_type = typename moab::Segment2<T>::coordinate_type;
  using point_type = typename moab::Segment2<T>::point_type;

  static inline void set(moab::Segment2<T>& s, direction_1d dir,
                         const point_type& p) {
    s[dir.to_int()] = p;
  }
  static inline moab::Segment2<T> construct(const point_type& p0,
                                            const point_type& p1) {
    return moab::Segment2<T>(p0, p1);
  }
};

}  // namespace boost::polygon

#endif  // MOAB_SEGMENT2_H_