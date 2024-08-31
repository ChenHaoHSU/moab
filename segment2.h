#ifndef MOAB_SEGMENT2_H_
#define MOAB_SEGMENT2_H_

#include <array>
#include <cstdint>
#include <ostream>
#include <string>
#include <utility>

#include "absl/hash/hash.h"
#include "absl/log/log.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "boost/geometry/core/access.hpp"
#include "boost/geometry/core/coordinate_dimension.hpp"
#include "boost/geometry/core/coordinate_system.hpp"
#include "boost/geometry/core/cs.hpp"
#include "boost/geometry/core/tag.hpp"
#include "boost/geometry/geometries/concepts/point_concept.hpp"
#include "boost/polygon/polygon.hpp"
#include "point2.h"

namespace moab {

template <typename T>
class Segment2 {
 public:
  // Constructors.
  Segment2() : d_({Point2<T>(0, 0), Point2<T>(0, 0)}) {}
  explicit Segment2(Point2<T> x, Point2<T> y) : d_({x, y}) {}
  Segment2(const Segment2& s) = default;
  Segment2(Segment2&& s) = default;
  // Destructors.
  ~Segment2() = default;

  // Accessors.
  Point2<T> p1() const { return d_[0]; }
  Point2<T> p2() const { return d_[1]; }
  Point2<T>* data() { return d_.data(); }
  const Point2<T>* data() const { return d_.data(); }

  constexpr std::size_t Size() const { return d_.size(); }

  std::pair<Point2<T>, Point2<T>> ToPair() const { return {d_[0], d_[1]}; }

  // Mutators.
  void Set(Point2<T> p1, Point2<T> p2) {
    d_[0] = p1;
    d_[1] = p2;
  }
  void SetP1(Point2<T> p) { d_[0] = p; }
  void SetP2(Point2<T> p) { d_[1] = p; }
  void SetP(std::size_t i, Point2<T> p) { d_[i] = p; }

  // Operations.
  void Shift(T dxP1, T dyP1, T dxP2, T dyP2) {
    d_[0].Sfhit(dxP1, dyP1);
    d_[1].Sfhit(dxP2, dyP2);
  }
  void ShiftP1(T dx, T dy) { d_[0].shift(dx, dy); }
  void ShiftP2(T dx, T dy) { d_[1].shift(dx, dy); }
  void Rotate90() {
    // Counterclockwise rotation by 90 degrees.
    d_[0].Rotate90();
    d_[1].Rotate90();
  }
  void Rotate180() {
    // Rotation by 180 degrees.
    d_[0].Rotate180();
    d_[1].Rotate180();
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
  //   bool operator<(const Segment2& p) const {
  //     return d_[0] != p.d_[0] ? d_[0] < p.d_[0] : d_[1] < p.d_[1];
  //   }
  //   bool operator>(const Segment2& p) const { return p < *this; }
  //   bool operator<=(const Segment2& p) const { return !(*this > p); }
  //   bool operator>=(const Segment2& p) const { return !(*this < p); }
  // Operators - Arithmetic
  Segment2& operator+=(const Segment2& s) {
    d_[0] += s.d_[0];
    d_[1] += s.d_[1];
    return *this;
  }
  Segment2& operator-=(const Segment2& s) {
    d_[0] -= s.d_[0];
    d_[1] -= s.d_[1];
    return *this;
  }
  Segment2 operator+(const Segment2& s) {
    return Segment2(d_[0] + s.d_[0], d_[1] + s.d_[1]);
  }
  Segment2 operator-(const Segment2& s) {
    return Segment2(d_[0] - s.d_[0], d_[1] - s.d_[1]);
  }
  Segment2 operator+() const { return Segment2(d_[0], d_[1]); }
  Segment2 operator-() const { return Segment2(-d_[0], -d_[1]); }

  Segment2& operator+=(T v) {
    d_[0] += v;
    d_[1] += v;
    return *this;
  }
  Segment2& operator-=(T v) {
    d_[0] -= v;
    d_[1] -= v;
    return *this;
  }
  Segment2& operator*=(T v) {
    d_[0] *= v;
    d_[1] *= v;
    return *this;
  }
  Segment2& operator/=(T v) {
    d_[0] /= v;
    d_[1] /= v;
    return *this;
  }
  Segment2 operator+(T v) const { return Segment2(d_[0] + v, d_[1] + v); }
  Segment2 operator-(T v) const { return Segment2(d_[0] - v, d_[1] - v); }
  Segment2 operator*(T v) const { return Segment2(d_[0] * v, d_[1] * v); }
  Segment2 operator/(T v) const { return Segment2(d_[0] / v, d_[1] / v); }

  // String conversion.
  template <typename Sink>
  friend void AbslStringify(Sink& sink, const Segment2& s) {
    absl::Format(&sink, "(%d %d) (%d %d)", s.d_[0][0], s.d_[0][1], s.d_[1][0],
                 s.d_[1][1]);
  }
  std::string ToString() const { return absl::StrCat(*this); }
  friend std::ostream& operator<<(std::ostream& os, const Segment2& s) {
    os << s.ToString();
    return os;
  }

  // Hash.
  template <typename H>
  friend H AbslHashValue(H h, const Segment2& s) {
    return H::combine(std::move(h), s.d_[0][0], s.d_[0][1], s.d_[1][0],
                      s.d_[1][1]);
  }

 private:
  std::array<Point2<T>, 2> d_;  // <p1, p2>
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
  static inline T get(moab::Segment2<T> const& s) { return s.p1(); }
  static inline void set(moab::Segment2<T>& s, moab::Point2<T> const& p) {
    p.SetP1(p);
  }
};

template <typename T>
struct access<moab::Segment2<T>, 1> {
  static inline T get(moab::Segment2<T> const& s) { return s.p2(); }
  static inline void set(moab::Segment2<T>& s, moab::Point2<T> const& p) {
    s.SetP2(p);
  }
};

}  // namespace boost::geometry::traits

// Boost polygon traits.
namespace boost::polygon {

template <typename T>
struct geometry_concept<moab::Segment2<T>> {
  using type = segment_concept;
};

template <typename T>
struct point_traits<moab::Segment2<T>> {
  static inline T get(const moab::Segment2<T>& s, direction_1d dir) {
    if (dir == LOW) {
      return s.p1();
    } else {
      return s.p2();
    }
  }
};

template <typename T>
struct point_mutable_traits<moab::Segment2<T>> {
  typedef int coordinate_type;

  static inline void set(moab::Segment2<T>& s, direction_1d dir,
                         moab::Point2<T> p) {
    if (dir == LOW) {
      s.SetP1(p);
    } else {
      s.SetP2(p);
    }
  }
  static inline moab::Segment2<T> construct(moab::Point2<T> p1,
                                            moab::Point2<T> p2) {
    return moab::Segment2<T>(p1, p2);
  }
};

}  // namespace boost::polygon

#endif  // MOAB_POINT2_H_