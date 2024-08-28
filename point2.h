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
#include "boost/polygon/point_concept.hpp"

namespace moab {

template <typename T>
class Point2 {
 public:
  // Constructors.
  Point2() : d_({0, 0}) {}
  explicit Point2(T x, T y) : d_({x, y}) {}
  Point2(Point2& p) = default;
  Point2(Point2&& p) = default;
  // Destructors.
  ~Point2() = default;

  // Accessors.
  T x() const { return d_[0]; }
  T y() const { return d_[1]; }
  T* data() { return d_.data(); }
  const T* data() const { return d_.data(); }

  constexpr std::size_t Size() const { return d_.size(); }

  std::pair<T, T> ToPair() const { return {d_[0], d_[1]}; }

  // Mutators.
  void Set(int x, int y) {
    d_[0] = x;
    d_[1] = y;
  }
  void SetX(int x) { d_[0] = x; }
  void SetY(int y) { d_[1] = y; }
  void SetDim(int i, T v) { d_[i] = v; }

  // Operations.
  void Shift(int dx, int dy) {
    d_[0] += dx;
    d_[1] += dy;
  }
  void ShiftX(int dx) { d_[0] += dx; }
  void ShiftY(int dy) { d_[1] += dy; }
  void Rotate90() {
    // Counterclockwise rotation by 90 degrees.
    T x = d_[0];
    d_[0] = -d_[1];
    d_[1] = x;
  }
  void Rotate180() {
    // Rotation by 180 degrees.
    d_[0] = -d_[0];
    d_[1] = -d_[1];
  }

  // Operators.
  // Operators - Assignment
  Point2& operator=(const Point2& p) {
    d_ = p.d_;
    return *this;
  }
  // Operators - Equality
  bool operator==(const Point2& p) const {
    return d_[0] == p.d_[0] && d_[1] == p.d_[1];
  }
  bool operator!=(const Point2& p) const { return !(*this == p); }
  // Operators - Inequality
  bool operator<(const Point2& p) const {
    return d_[0] != p.d_[0] ? d_[0] < p.d_[0] : d_[1] < p.d_[1];
  }
  bool operator>(const Point2& p) const { return p < *this; }
  bool operator<=(const Point2& p) const { return !(*this > p); }
  bool operator>=(const Point2& p) const { return !(*this < p); }
  // Operators - Subscript
  T& operator[](int i) { return d_[i]; }
  const T& operator[](int i) const { return d_.at(i); }
  // Operators - Arithmetic
  Point2& operator+=(const Point2& p) {
    d_[0] += p.d_[0];
    d_[1] += p.d_[1];
    return *this;
  }
  Point2& operator-=(const Point2& p) {
    d_[0] -= p.d_[0];
    d_[1] -= p.d_[1];
    return *this;
  }
  Point2 operator+(const Point2& p) {
    return Point2(d_[0] + p.d_[0], d_[1] + p.d_[1]);
  }
  Point2 operator-(const Point2& p) {
    return Point2(d_[0] - p.d_[0], d_[1] - p.d_[1]);
  }
  Point2 operator+() const { return Point2(d_[0], d_[1]); }
  Point2 operator-() const { return Point2(-d_[0], -d_[1]); }

  Point2& operator+=(T v) {
    d_[0] += v;
    d_[1] += v;
    return *this;
  }
  Point2& operator-=(T v) {
    d_[0] -= v;
    d_[1] -= v;
    return *this;
  }
  Point2& operator*=(T v) {
    d_[0] *= v;
    d_[1] *= v;
    return *this;
  }
  Point2& operator/=(T v) {
    d_[0] /= v;
    d_[1] /= v;
    return *this;
  }
  Point2 operator+(T v) const { return Point2(d_[0] + v, d_[1] + v); }
  Point2 operator-(T v) const { return Point2(d_[0] - v, d_[1] - v); }
  Point2 operator*(T v) const { return Point2(d_[0] * v, d_[1] * v); }
  Point2 operator/(T v) const { return Point2(d_[0] / v, d_[1] / v); }

  // String conversion.
  template <typename Sink>
  friend void AbslStringify(Sink& sink, const Point2& p) {
    absl::Format(&sink, "(%d %d)", p.d_[0], p.d_[1]);
  }
  std::string ToString() const { return absl::StrCat(*this); }
  friend std::ostream& operator<<(std::ostream& os, const Point2& p) {
    os << p.ToString();
    return os;
  }

  // Hash.
  template <typename H>
  friend H AbslHashValue(H h, const Point2& p) {
    return H::combine(std::move(h), p.d_[0], p.d_[1]);
  }

 private:
  std::array<T, 2> d_;  // <x, y>
};  // class Point2

// Aliases.
using Point2_i = Point2<int>;
using Point2_i32 = Point2<int32_t>;
using Point2_i64 = Point2<int64_t>;

}  // namespace moab

// Boost geometry traits.
namespace boost {
namespace geometry {
namespace traits {

template <typename T>
struct tag<moab::Point2<T>> {
  using type = point_tag;
};

template <typename T>
struct dimension<moab::Point2<T>> : boost::mpl::int_<2> {};

template <typename T>
struct coordinate_type<moab::Point2<T>> {
  using type = T;
};

template <typename T>
struct coordinate_system<moab::Point2<T>> {
  using type = boost::geometry::cs::cartesian;
};

template <typename T>
struct access<moab::Point2<T>, 0> {
  static inline T get(moab::Point2<T> const& p) { return p.x(); }
  static inline void set(moab::Point2<T>& p, T const& value) { p.SetX(value); }
};

template <typename T>
struct access<moab::Point2<T>, 1> {
  static inline T get(moab::Point2<T> const& p) { return p.y(); }
  static inline void set(moab::Point2<T>& p, T const& value) { p.SetY(value); }
};

}  // namespace traits
}  // namespace geometry
}  // namespace boost
