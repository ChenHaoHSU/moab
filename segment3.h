#ifndef MOAB_SEGMENT3_H_
#define MOAB_SEGMENT3_H_

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
#include "point3.h"

namespace moab {

template <typename T>
class Segment3 {
 public:
  // Type aliases. (Required by Boost geometry/polygon traits.)
  using coordinate_type = T;
  using point_type = Point3<T>;

  // Constructors.
  Segment3() : d_({Point3<T>(0, 0, 0), Point3<T>(0, 0, 0)}) {}
  explicit Segment3(const Point3<T>& p1, const Point3<T>& p2) { Set(p1, p2); }
  explicit Segment3(T xl, T yl, T zl, T xh, T yh, T zh) {
    Set(Point3<T>(xl, yl, zl), Point3<T>(xh, yh, zh));
  }
  Segment3(const Segment3& s) = default;
  Segment3(Segment3&& s) = default;
  // Destructors.
  ~Segment3() = default;

  // Accessors.
  Point3<T> p1() const { return d_[0]; }
  Point3<T> p2() const { return d_[1]; }
  Point3<T>* data() { return d_.data(); }
  const Point3<T>* data() const { return d_.data(); }

  constexpr std::size_t Size() const { return d_.size(); }

  std::pair<Point3<T>, Point3<T>> ToPair() const { return {d_[0], d_[1]}; }

  // Mutators.
  void Set(T p1x, T p1y, T p1z, T p2x, T p2y, T p2z) {
    DCHECK(p1x <= p2x) << "Invalid segment. p1x: " << p1x << ", p1y: " << p1y
                       << ", p1z: " << p1z << ", p2x: " << p2x
                       << ", p2y: " << p2y << ", p2z: " << p2z;
    d_[0].Set(p1x, p1y, p1z);
    d_[1].Set(p2x, p2y, p2z);
  }
  void Set(const Point3<T>& p1, const Point3<T>& p2) {
    Set(p1.x(), p1.y(), p1.z(), p2.x(), p2.y(), p2.z());
  }
  void SetP1(Point3<T> p) {
    Set(p.x(), p.y(), p.z(), d_[1].x(), d_[1].y(), d_[1].z());
  }
  void SetP2(Point3<T> p) {
    Set(d_[0].x(), d_[0].y(), d_[0].z(), p.x(), p.y(), p.z());
  }
  void SetP(std::size_t i, Point3<T> p) {
    DCHECK(0 <= i && i <= 1) << "Invalid SetP Index i" << i;
    (i == 0) ? SetP1(p) : SetP2(p);
  }

  // Operations.
  void Shift(T dxP1, T dyP1, T dzP1, T dxP2, T dyP2, T dzP2) {
    d_[0].Sfhit(dxP1, dyP1, dzP1);
    d_[1].Sfhit(dxP2, dyP2, dzP2);
  }
  void Shift(T dx, T dy, T dz) {
    d_[0].Shift(dx, dy, dz);
    d_[1].Shift(dx, dy, dz);
  }
  void ShiftX(T dx) {
    d_[0].ShiftX(dx);
    d_[1].ShiftX(dx);
  }
  void ShiftY(T dy) {
    d_[0].ShiftY(dy);
    d_[1].ShiftY(dy);
  }
  void ShiftZ(T dz) {
    d_[0].ShiftZ(dz);
    d_[1].ShiftZ(dz);
  }
  void ShiftP1(T dx, T dy, T dz) { d_[0].Shift(dx, dy, dz); }
  void ShiftP2(T dx, T dy, T dz) { d_[1].Shift(dx, dy, dz); }

  // Operators.
  // Operators - Assignment
  Segment3& operator=(const Segment3& p) {
    d_ = p.d_;
    return *this;
  }
  Segment3& operator=(Segment3&& p) {
    d_ = std::move(p.d_);
    return *this;
  }
  // Operators - Subscript
  Point3<T>& operator[](std::size_t i) { return d_[i]; }
  const Point3<T>& operator[](std::size_t i) const { return d_.at(i); }
  // Operators - Equality
  bool operator==(const Segment3& p) const {
    return d_[0] == p.d_[0] && d_[1] == p.d_[1];
  }
  bool operator!=(const Segment3& p) const { return !(*this == p); }
  // Operators - Inequality
  bool operator<(const Segment3& p) const {
    return d_[0] != p.d_[0] ? d_[0] < p.d_[0] : d_[1] < p.d_[1];
  }
  bool operator>(const Segment3& p) const { return p < *this; }
  bool operator<=(const Segment3& p) const { return !(*this > p); }
  bool operator>=(const Segment3& p) const { return !(*this < p); }

  Segment3 operator+() const { return Segment3(d_[0], d_[1]); }
  Segment3 operator-() const { return Segment3(-d_[0], -d_[1]); }

  // String conversion.
  template <typename Sink>
  friend void AbslStringify(Sink& sink, const Segment3& s) {
    absl::Format(&sink, "((%d %d %d) (%d %d %d))", s.d_[0][0], s.d_[0][1],
                 s.d_[0][2], s.d_[1][0], s.d_[1][1], s.d_[1][2]);
  }
  std::string ToString() const { return absl::StrCat(*this); }
  friend std::ostream& operator<<(std::ostream& os, const Segment3& s) {
    os << s.ToString();
    return os;
  }

  // Hash.
  template <typename H>
  friend H AbslHashValue(H h, const Segment3& s) {
    return H::combine(std::move(h), s.d_[0][0], s.d_[0][1], s.d_[0][2],
                      s.d_[1][0], s.d_[1][1], s.d_[1][2]);
  }

 private:
  std::array<Point3<T>, 2> d_;  // <p1, p2>
};  // class Segment3

// Aliases.
using Segment3_i = Segment3<int>;
using Segment3_i32 = Segment3<int32_t>;
using Segment3_i64 = Segment3<int64_t>;

}  // namespace moab

// Boost geometry traits.
namespace boost::geometry::traits {

template <typename T>
struct tag<moab::Segment3<T>> {
  using type = segment_tag;
};

template <typename T>
struct dimension<moab::Segment3<T>> : boost::mpl::int_<3> {};

template <typename T>
struct coordinate_type<moab::Segment3<T>> {
  using type = T;
};

template <typename T>
struct coordinate_system<moab::Segment3<T>> {
  using type = boost::geometry::cs::cartesian;
};

template <typename T>
struct access<moab::Segment3<T>, 0> {
  using coordinate_type = typename moab::Segment3<T>::coordinate_type;
  using point_type = typename moab::Segment3<T>::point_type;
  static inline point_type get(moab::Segment3<T> const& s) { return s[0]; }
  static inline void set(moab::Segment3<T>& s, point_type const& p) {
    s[0] = p;
  }
};

template <typename T>
struct access<moab::Segment3<T>, 1> {
  using coordinate_type = typename moab::Segment3<T>::coordinate_type;
  using point_type = typename moab::Segment3<T>::point_type;
  static inline point_type get(moab::Segment3<T> const& s) { return s[1]; }
  static inline void set(moab::Segment3<T>& s, point_type const& p) {
    s[1] = p;
  }
};

template <typename T>
struct access<moab::Segment3<T>, 2> {
  using coordinate_type = typename moab::Segment3<T>::coordinate_type;
  using point_type = typename moab::Segment3<T>::point_type;
  static inline point_type get(moab::Segment3<T> const& s) { return s[2]; }
  static inline void set(moab::Segment3<T>& s, point_type const& p) {
    s[2] = p;
  }
};

}  // namespace boost::geometry::traits

#endif  // MOAB_SEGMENT3_H_