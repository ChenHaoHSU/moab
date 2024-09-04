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
  // Type aliases. (Required by Boost geometry/polygon traits.)
  using coordinate_type = T;
  using point_type = Point2<T>;

  // Constructors.
  Segment2() : d_({Point2<T>(0, 0), Point2<T>(0, 0)}) {}
  explicit Segment2(const Point2<T>& p1, const Point2<T>& p2) { Set(p1, p2); }
  explicit Segment2(T xl, T yl, T xh, T yh) {
    Set(Point2<T>(xl, yl), Point2<T>(xh, yh));
  }
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
  void Set(T p1x, T p1y, T p2x, T p2y) {
    DCHECK(p1x <= p2x) << "Invalid segment. p1x: " << p1x << ", p1y: " << p1y
                       << ", p2x: " << p2x << ", p2y: " << p2y;
    d_[0].Set(p1x, p1y);
    d_[1].Set(p2x, p2y);
  }
  void Set(const Point2<T>& p1, const Point2<T>& p2) {
    Set(p1.x(), p1.y(), p2.x(), p2.y());
  }
  void SetP1(Point2<T> p) { Set(p.x(), p.y(), d_[1].x(), d_[1].y()); }
  void SetP2(Point2<T> p) { Set(d_[0].x(), d_[0].y(), p.x(), p.y()); }
  void SetP(std::size_t i, Point2<T> p) { (i == 0) ? SetP1(p) : SetP2(p); }

  // Operations.
  void Shift(T dxP1, T dyP1, T dxP2, T dyP2) {
    d_[0].Sfhit(dxP1, dyP1);
    d_[1].Sfhit(dxP2, dyP2);
  }
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
  void ShiftP1(T dx, T dy) { d_[0].Shift(dx, dy); }
  void ShiftP2(T dx, T dy) { d_[1].Shift(dx, dy); }

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

  Segment2 operator+() const { return Segment2(d_[0], d_[1]); }
  Segment2 operator-() const { return Segment2(-d_[0], -d_[1]); }

  // String conversion.
  template <typename Sink>
  friend void AbslStringify(Sink& sink, const Segment2& s) {
    absl::Format(&sink, "((%d %d) (%d %d))", s.d_[0][0], s.d_[0][1], s.d_[1][0],
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
  using coordinate_type = typename moab::Segment2<T>::coordinate_type;
  using point_type = typename moab::Segment2<T>::point_type;
  static inline point_type get(moab::Segment2<T> const& s) { return s[0]; }
  static inline void set(moab::Segment2<T>& s, point_type const& p) {
    s[0] = p;
  }
};

template <typename T>
struct access<moab::Segment2<T>, 1> {
  using coordinate_type = typename moab::Segment2<T>::coordinate_type;
  using point_type = typename moab::Segment2<T>::point_type;
  static inline point_type get(moab::Segment2<T> const& s) { return s[1]; }
  static inline void set(moab::Segment2<T>& s, point_type const& p) {
    s[1] = p;
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
struct segment_traits<moab::Segment2<T>> {
  static inline T get(const moab::Segment2<T>& s, direction_1d dir) {
    return s[dir.to_int()];
  }
};

template <typename T>
struct segment_mutable_traits<moab::Segment2<T>> {
  typedef int coordinate_type;

  static inline void set(moab::Segment2<T>& s, direction_1d dir,
                         moab::Point2<T> p) {
    s[dir.to_int()] = p;
  }
  static inline moab::Segment2<T> construct(moab::Point2<T> p1,
                                            moab::Point2<T> p2) {
    return moab::Segment2<T>(p1, p2);
  }
};

}  // namespace boost::polygon

#endif  // MOAB_POINT2_H_