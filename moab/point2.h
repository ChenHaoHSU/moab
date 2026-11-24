#ifndef MOAB_POINT2_H_
#define MOAB_POINT2_H_

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <string>
#include <type_traits>
#include <utility>

#include "absl/hash/hash.h"
#include "absl/log/check.h"
#include "absl/status/status.h"
#include "absl/status/status_macros.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "boost/geometry/core/access.hpp"
#include "boost/geometry/core/coordinate_dimension.hpp"
#include "boost/geometry/core/coordinate_system.hpp"
#include "boost/geometry/core/cs.hpp"
#include "boost/geometry/core/tag.hpp"
#include "boost/geometry/geometries/concepts/point_concept.hpp"
#include "boost/polygon/point_concept.hpp"
#include "moab/coordinate_conversion.h"
#include "moab/point2.pb.h"

namespace moab {

template <typename T>
class Point2 {
 public:
  // Type aliases. (Required by Boost geometry/polygon traits.)
  using coordinate_type = T;
  // Type of squared lengths (and areas). int64_t for integral coordinates to
  // avoid overflow.
  using area_type = std::conditional_t<std::is_integral_v<T>, int64_t, T>;

  // Constructors.
  Point2() : d_({0, 0}) {}
  explicit Point2(T x, T y) : d_({x, y}) {}
  // Aborts if 'proto' is invalid (see FromProto). Prefer FromProto.
  explicit Point2(const Point2Proto& proto) { CHECK_OK(SetFromProto(proto)); }
  Point2(const Point2&) = default;
  Point2(Point2&&) = default;
  ~Point2() = default;

  // Assignment operators.
  Point2& operator=(const Point2&) = default;
  Point2& operator=(Point2&&) = default;

  // Accessors.
  T x() const { return d_[0]; }
  T y() const { return d_[1]; }
  const T* data() const { return d_.data(); }
  constexpr std::size_t Size() const { return d_.size(); }
  std::pair<T, T> ToPair() const { return {d_[0], d_[1]}; }

  // Mutators.
  void Set(T x, T y) {
    d_[0] = x;
    d_[1] = y;
  }
  void SetX(T x) { d_[0] = x; }
  void SetY(T y) { d_[1] = y; }
  void SetDim(std::size_t i, T v) {
    DCHECK_LT(i, d_.size());
    d_[i] = v;
  }

  // Operations.
  void Shift(T dx, T dy) {
    d_[0] += dx;
    d_[1] += dy;
  }
  void ShiftX(T dx) { d_[0] += dx; }
  void ShiftY(T dy) { d_[1] += dy; }
  // Rotates the point counterclockwise by 90 degrees around the origin.
  void Rotate90() {
    T x = d_[0];
    d_[0] = -d_[1];
    d_[1] = x;
  }
  // Rotates the point by 180 degrees around the origin.
  void Rotate180() {
    d_[0] = -d_[0];
    d_[1] = -d_[1];
  }

  // Operators.
  // Operators - Subscript
  T& operator[](std::size_t i) {
    DCHECK_LT(i, d_.size());
    return d_[i];
  }
  const T& operator[](std::size_t i) const {
    DCHECK_LT(i, d_.size());
    return d_[i];
  }
  // Operators - Equality
  bool operator==(const Point2& p) const {
    return d_[0] == p.d_[0] && d_[1] == p.d_[1];
  }
  bool operator!=(const Point2& p) const { return !(*this == p); }
  // Operators - Inequality
  // Lexicographic order, for sorting and ordered containers; not a geometric
  // comparison. Compares x, then y.
  bool operator<(const Point2& p) const {
    return d_[0] != p.d_[0] ? d_[0] < p.d_[0] : d_[1] < p.d_[1];
  }
  bool operator>(const Point2& p) const { return p < *this; }
  bool operator<=(const Point2& p) const { return !(*this > p); }
  bool operator>=(const Point2& p) const { return !(*this < p); }
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
  Point2 operator+(const Point2& p) const {
    return Point2(d_[0] + p.d_[0], d_[1] + p.d_[1]);
  }
  Point2 operator-(const Point2& p) const {
    return Point2(d_[0] - p.d_[0], d_[1] - p.d_[1]);
  }
  Point2 operator+() const { return Point2(d_[0], d_[1]); }
  Point2 operator-() const { return Point2(-d_[0], -d_[1]); }

  // The scalar operators apply the scalar to every coordinate: p + 1 adds 1 to
  // both x and y.
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

  // Distance.
  // Returns the Manhattan (L1) distance to 'p': |dx| + |dy|.
  T ManhattanDistance(const Point2& p) const {
    return std::abs(d_[0] - p.d_[0]) + std::abs(d_[1] - p.d_[1]);
  }
  // Same as ManhattanDistance. Prefer ManhattanDistance, which names the
  // metric.
  T Distance(const Point2& p) const { return ManhattanDistance(p); }
  // Returns the squared Euclidean distance to 'p', dx^2 + dy^2, computed in
  // area_type. For integer coordinates it is exact while the result fits in
  // int64_t, which holds whenever each difference is below 2^31 (about 2.1e9).
  // Compare it against a squared spacing to avoid a square root:
  // SquaredEuclideanDistance(p) < s * s.
  area_type SquaredEuclideanDistance(const Point2& p) const {
    const area_type dx =
        static_cast<area_type>(d_[0]) - static_cast<area_type>(p.d_[0]);
    const area_type dy =
        static_cast<area_type>(d_[1]) - static_cast<area_type>(p.d_[1]);
    return dx * dx + dy * dy;
  }
  // Returns the Euclidean (L2) distance to 'p'.
  double EuclideanDistance(const Point2& p) const {
    return std::sqrt(static_cast<double>(SquaredEuclideanDistance(p)));
  }

  // String conversion.
  template <typename Sink>
  friend void AbslStringify(Sink& sink, const Point2& p) {
    absl::Format(&sink, "(%v %v)", p.d_[0], p.d_[1]);
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

  // Protobuf.
  // Returns a Point2Proto.
  Point2Proto ToProto() const noexcept;
  // Sets the Point2 from a Point2Proto. On error, returns the error from
  // FromProto and leaves the Point2 unchanged.
  absl::Status SetFromProto(const Point2Proto& proto);
  // Returns the Point2 in 'proto'. Returns InvalidArgument if no coordinate
  // type is set, or if a coordinate is not finite, does not fit in T, or is
  // not a whole number for an integral T.
  static absl::StatusOr<Point2> FromProto(const Point2Proto& proto);

 private:
  std::array<T, 2> d_;  // <x, y>
};

// Protobuf.
template <typename T>
Point2Proto Point2<T>::ToProto() const noexcept {
  Point2Proto proto;
  if constexpr (std::is_same_v<T, int> || std::is_same_v<T, int32_t>) {
    proto.mutable_point_int32()->set_x(d_[0]);
    proto.mutable_point_int32()->set_y(d_[1]);
  } else if constexpr (std::is_same_v<T, int64_t>) {
    proto.mutable_point_int64()->set_x(d_[0]);
    proto.mutable_point_int64()->set_y(d_[1]);
  } else if constexpr (std::is_same_v<T, float>) {
    proto.mutable_point_float()->set_x(d_[0]);
    proto.mutable_point_float()->set_y(d_[1]);
  } else if constexpr (std::is_same_v<T, double>) {
    proto.mutable_point_double()->set_x(d_[0]);
    proto.mutable_point_double()->set_y(d_[1]);
  } else {
    static_assert(std::is_same_v<T, int>, "Unsupported type.");
  }
  return proto;
}

template <typename T>
absl::Status Point2<T>::SetFromProto(const Point2Proto& proto) {
  ABSL_ASSIGN_OR_RETURN(*this, FromProto(proto));
  return absl::OkStatus();
}

template <typename T>
absl::StatusOr<Point2<T>> Point2<T>::FromProto(const Point2Proto& proto) {
  const auto from = [](const auto& p) -> absl::StatusOr<Point2<T>> {
    ABSL_ASSIGN_OR_RETURN(const T x,
                          internal::ConvertCoordinate<T>(p.x(), "[Point2] x"));
    ABSL_ASSIGN_OR_RETURN(const T y,
                          internal::ConvertCoordinate<T>(p.y(), "[Point2] y"));
    return Point2<T>(x, y);
  };
  switch (proto.type_variant_case()) {
    case Point2Proto::kPointInt32:
      return from(proto.point_int32());
    case Point2Proto::kPointInt64:
      return from(proto.point_int64());
    case Point2Proto::kPointFloat:
      return from(proto.point_float());
    case Point2Proto::kPointDouble:
      return from(proto.point_double());
    case Point2Proto::TYPE_VARIANT_NOT_SET:
      break;
  }
  return absl::InvalidArgumentError(
      "[Point2] The proto has no coordinate type set.");
}

// Aliases.
using Point2_i = Point2<int>;
using Point2_i32 = Point2<int32_t>;
using Point2_i64 = Point2<int64_t>;

}  // namespace moab

// Boost geometry traits.
namespace boost::geometry::traits {

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

template <typename T, std::size_t Dimension>
struct access<moab::Point2<T>, Dimension> {
  using coordinate_type = typename moab::Point2<T>::coordinate_type;

  static inline coordinate_type get(const moab::Point2<T>& point) {
    return point[Dimension];
  }
  static inline void set(moab::Point2<T>& point, const coordinate_type& value) {
    point[Dimension] = value;
  }
};

}  // namespace boost::geometry::traits

// Boost polygon traits.
namespace boost::polygon {

template <typename T>
struct geometry_concept<moab::Point2<T>> {
  using type = point_concept;
};

template <typename T>
struct point_traits<moab::Point2<T>> {
  using coordinate_type = typename moab::Point2<T>::coordinate_type;

  static inline coordinate_type get(const moab::Point2<T>& point,
                                    orientation_2d orient) {
    return point[orient.to_int()];
  }
};

template <typename T>
struct point_mutable_traits<moab::Point2<T>> {
  using coordinate_type = typename moab::Point2<T>::coordinate_type;

  static inline void set(moab::Point2<T>& point, orientation_2d orient,
                         coordinate_type value) {
    point[orient.to_int()] = value;
  }
  static inline moab::Point2<T> construct(coordinate_type x,
                                          coordinate_type y) {
    return moab::Point2<T>(x, y);
  }
};

}  // namespace boost::polygon

#endif  // MOAB_POINT2_H_