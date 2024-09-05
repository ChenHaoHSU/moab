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
#include "point3.h"

namespace moab {

template <typename T>
class Segment3 {
 public:
  // Type aliases. (Required by Boost geometry traits.)
  using coordinate_type = T;
  using point_type = Point3<T>;

  // Constructors.
  Segment3() : d_({Point3<T>(0, 0, 0), Point3<T>(0, 0, 0)}) {}
  explicit Segment3(const Point3<T>& p0, const Point3<T>& p1) { Set(p0, p1); }
  explicit Segment3(T xl, T yl, T zl, T xh, T yh, T zh) {
    Set(Point3<T>(xl, yl, zl), Point3<T>(xh, yh, zh));
  }
  Segment3(const Segment3& s) = default;
  Segment3(Segment3&& s) = default;
  // Destructors.
  ~Segment3() = default;

  // Accessors.
  Point3<T>& p0() { return d_[0]; }
  const Point3<T>& p0() const { return d_.at(0); }
  Point3<T>& p1() { return d_[1]; }
  const Point3<T>& p1() const { return d_.at(1); }
  Point3<T>* data() { return d_.data(); }
  const Point3<T>* data() const { return d_.data(); }

  constexpr std::size_t Size() const { return d_.size(); }

  std::pair<Point3<T>, Point3<T>> ToPair() const { return {d_[0], d_[1]}; }

  // Mutators.
  void Set(T xl, T yl, T zl, T xh, T yh, T zh) {
    d_[0].Set(xl, yl, zl);
    d_[1].Set(xh, yh, zh);
  }
  void Set(const Point3<T>& p0, const Point3<T>& p1) {
    d_[0] = p0;
    d_[1] = p1;
  }
  void SetP0(const Point3<T>& p) { d_[0] = p; }
  void SetP1(const Point3<T>& p) { d_[1] = p; }
  void SetP(std::size_t i, const Point3<T>& p) {
    DCHECK(i < 2) << "Invalid SetP Index i" << i;
    d_[i] = p;
  }

  // Operations.
  // It's safe. No need to check for validity.
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

  // String conversion.
  template <typename Sink>
  friend void AbslStringify(Sink& sink, const Segment3& s) {
    absl::Format(&sink, "(%v %v)", s.d_.at(0), s.d_.at(1));
  }
  std::string ToString() const { return absl::StrCat(*this); }
  friend std::ostream& operator<<(std::ostream& os, const Segment3& s) {
    os << s.ToString();
    return os;
  }

  // Hash.
  template <typename H>
  friend H AbslHashValue(H h, const Segment3& s) {
    return H::combine(std::move(h), s.d_.at(0), s.d_.at(1));
  }

 private:
  std::array<Point3<T>, 2> d_;  // <p0, p1>
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

  static inline point_type get(const moab::Segment3<T>& s) { return s[0]; }
  static inline void set(moab::Segment3<T>& s, const point_type& p) {
    s[0] = p;
  }
};

template <typename T>
struct access<moab::Segment3<T>, 1> {
  using coordinate_type = typename moab::Segment3<T>::coordinate_type;
  using point_type = typename moab::Segment3<T>::point_type;

  static inline point_type get(const moab::Segment3<T>& s) { return s[1]; }
  static inline void set(moab::Segment3<T>& s, const point_type& p) {
    s[1] = p;
  }
};

template <typename T>
struct access<moab::Segment3<T>, 2> {
  using coordinate_type = typename moab::Segment3<T>::coordinate_type;
  using point_type = typename moab::Segment3<T>::point_type;

  static inline point_type get(const moab::Segment3<T>& s) { return s[2]; }
  static inline void set(moab::Segment3<T>& s, const point_type& p) {
    s[2] = p;
  }
};

}  // namespace boost::geometry::traits

#endif  // MOAB_SEGMENT3_H_