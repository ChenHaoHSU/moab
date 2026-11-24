#ifndef MOAB_POINT3_H_
#define MOAB_POINT3_H_

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <string>
#include <tuple>
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
#include "moab/coordinate_conversion.h"
#include "moab/point2.h"
#include "moab/point3.pb.h"

namespace moab {

template <typename T>
class Point3 {
 public:
  // Type aliases. (Required by Boost geometry/polygon traits.)
  using coordinate_type = T;
  // Type of squared lengths (and areas). int64_t for integral coordinates to
  // avoid overflow.
  using area_type = std::conditional_t<std::is_integral_v<T>, int64_t, T>;

  // Constructors.
  Point3() : d_({0, 0, 0}) {}
  explicit Point3(T x, T y, T z) : d_({x, y, z}) {}
  // Aborts if 'proto' is invalid (see FromProto). Prefer FromProto.
  explicit Point3(const Point3Proto& proto) { CHECK_OK(SetFromProto(proto)); }
  Point3(const Point3&) = default;
  Point3(Point3&&) = default;
  ~Point3() = default;

  // Assignment operators.
  Point3& operator=(const Point3&) = default;
  Point3& operator=(Point3&&) = default;

  // Accessors.
  T x() const { return d_[0]; }
  T y() const { return d_[1]; }
  T z() const { return d_[2]; }
  T* data() { return d_.data(); }
  const T* data() const { return d_.data(); }
  constexpr std::size_t Size() const { return d_.size(); }

  // Returns the (x, y) point, dropping z.
  Point2<T> To2D() const { return Point2<T>(d_[0], d_[1]); }

  std::tuple<T, T, T> ToTuple() const {
    return std::make_tuple(d_[0], d_[1], d_[2]);
  }

  // Mutators.
  void Set(T x, T y, T z) {
    d_[0] = x;
    d_[1] = y;
    d_[2] = z;
  }
  void SetX(T x) { d_[0] = x; }
  void SetY(T y) { d_[1] = y; }
  void SetZ(T z) { d_[2] = z; }
  void SetDim(std::size_t i, T v) { d_[i] = v; }

  // Operations.
  void Shift(T dx, T dy, T dz) {
    d_[0] += dx;
    d_[1] += dy;
    d_[2] += dz;
  }
  void ShiftX(T dx) { d_[0] += dx; }
  void ShiftY(T dy) { d_[1] += dy; }
  void ShiftZ(T dz) { d_[2] += dz; }

  // The rotations turn the point around the origin in the plane of axes
  // 'axis1' and 'axis2' (0 = x, 1 = y, 2 = z), counterclockwise meaning from
  // axis1 toward axis2. The third coordinate is unchanged; for example,
  // (1, 2, 3).Rotate90(0, 1) gives (-2, 1, 3).

  // Rotates the point counterclockwise by 90 degrees.
  void Rotate90(size_t axis1, size_t axis2) {
    T x = d_[axis1];
    d_[axis1] = -d_[axis2];
    d_[axis2] = x;
  }
  // Rotates the point by 180 degrees.
  void Rotate180(size_t axis1, size_t axis2) {
    d_[axis1] = -d_[axis1];
    d_[axis2] = -d_[axis2];
  }
  // Rotates the point counterclockwise by 270 degrees.
  void Rotate270(size_t axis1, size_t axis2) {
    Rotate90(axis1, axis2);
    Rotate180(axis1, axis2);
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
  bool operator==(const Point3& p) const {
    return d_[0] == p.d_[0] && d_[1] == p.d_[1] && d_[2] == p.d_[2];
  }
  bool operator!=(const Point3& p) const { return !(*this == p); }
  // Operators - Inequality
  // Lexicographic order, for sorting and ordered containers; not a geometric
  // comparison. Compares x, then y, then z.
  bool operator<(const Point3& p) const {
    return d_[0] != p.d_[0]   ? d_[0] < p.d_[0]
           : d_[1] != p.d_[1] ? d_[1] < p.d_[1]
                              : d_[2] < p.d_[2];
  }
  bool operator>(const Point3& p) const { return p < *this; }
  bool operator<=(const Point3& p) const { return !(*this > p); }
  bool operator>=(const Point3& p) const { return !(*this < p); }

  // Operators - Arithmetic
  Point3& operator+=(const Point3& p) {
    d_[0] += p.d_[0];
    d_[1] += p.d_[1];
    d_[2] += p.d_[2];
    return *this;
  }
  Point3& operator-=(const Point3& p) {
    d_[0] -= p.d_[0];
    d_[1] -= p.d_[1];
    d_[2] -= p.d_[2];
    return *this;
  }
  Point3 operator+(const Point3& p) const {
    return Point3(d_[0] + p.d_[0], d_[1] + p.d_[1], d_[2] + p.d_[2]);
  }
  Point3 operator-(const Point3& p) const {
    return Point3(d_[0] - p.d_[0], d_[1] - p.d_[1], d_[2] - p.d_[2]);
  }
  Point3 operator+() const { return Point3(d_[0], d_[1], d_[2]); }
  Point3 operator-() const { return Point3(-d_[0], -d_[1], -d_[2]); }

  // The scalar operators apply the scalar to every coordinate: p + 1 adds 1 to
  // x, y and z.
  Point3& operator+=(T v) {
    d_[0] += v;
    d_[1] += v;
    d_[2] += v;
    return *this;
  }
  Point3& operator-=(T v) {
    d_[0] -= v;
    d_[1] -= v;
    d_[2] -= v;
    return *this;
  }
  Point3& operator*=(T v) {
    d_[0] *= v;
    d_[1] *= v;
    d_[2] *= v;
    return *this;
  }
  Point3& operator/=(T v) {
    d_[0] /= v;
    d_[1] /= v;
    d_[2] /= v;
    return *this;
  }
  Point3 operator+(T v) const {
    return Point3(d_[0] + v, d_[1] + v, d_[2] + v);
  }
  Point3 operator-(T v) const {
    return Point3(d_[0] - v, d_[1] - v, d_[2] - v);
  }
  Point3 operator*(T v) const {
    return Point3(d_[0] * v, d_[1] * v, d_[2] * v);
  }
  Point3 operator/(T v) const {
    return Point3(d_[0] / v, d_[1] / v, d_[2] / v);
  }

  // Distance.
  // Returns the Manhattan (L1) distance to 'p': |dx| + |dy| + |dz|.
  T ManhattanDistance(const Point3& p) const {
    return std::abs(d_[0] - p.d_[0]) + std::abs(d_[1] - p.d_[1]) +
           std::abs(d_[2] - p.d_[2]);
  }
  // Same as ManhattanDistance. Prefer ManhattanDistance, which names the
  // metric.
  T Distance(const Point3& p) const { return ManhattanDistance(p); }
  // Returns the squared Euclidean distance to 'p', dx^2 + dy^2 + dz^2,
  // computed exactly in area_type.
  area_type SquaredEuclideanDistance(const Point3& p) const {
    const area_type dx =
        static_cast<area_type>(d_[0]) - static_cast<area_type>(p.d_[0]);
    const area_type dy =
        static_cast<area_type>(d_[1]) - static_cast<area_type>(p.d_[1]);
    const area_type dz =
        static_cast<area_type>(d_[2]) - static_cast<area_type>(p.d_[2]);
    return dx * dx + dy * dy + dz * dz;
  }
  // Returns the Euclidean (L2) distance to 'p'.
  double EuclideanDistance(const Point3& p) const {
    return std::sqrt(static_cast<double>(SquaredEuclideanDistance(p)));
  }

  // String conversion.
  template <typename Sink>
  friend void AbslStringify(Sink& sink, const Point3& p) {
    absl::Format(&sink, "(%v %v %v)", p.d_[0], p.d_[1], p.d_[2]);
  }
  std::string ToString() const { return absl::StrCat(*this); }
  friend std::ostream& operator<<(std::ostream& os, const Point3& p) {
    os << p.ToString();
    return os;
  }

  // Hash.
  template <typename H>
  friend H AbslHashValue(H h, const Point3& p) {
    return H::combine(std::move(h), p.d_[0], p.d_[1], p.d_[2]);
  }

  // Protobuf.
  // Returns a Point3Proto.
  Point3Proto ToProto() const;
  // Sets the Point3 from a Point3Proto. On error, returns the error from
  // FromProto and leaves the Point3 unchanged.
  absl::Status SetFromProto(const Point3Proto& proto);
  // Returns the Point3 in 'proto'. Returns InvalidArgument if no coordinate
  // type is set, or if a coordinate is not finite, does not fit in T, or is
  // not a whole number for an integral T.
  static absl::StatusOr<Point3> FromProto(const Point3Proto& proto);

 private:
  std::array<T, 3> d_;  // (x, y, z)
};

// Protobuf.
template <typename T>
Point3Proto Point3<T>::ToProto() const {
  Point3Proto proto;
  if constexpr (std::is_same_v<T, int> || std::is_same_v<T, int32_t>) {
    proto.mutable_point_int32()->set_x(d_[0]);
    proto.mutable_point_int32()->set_y(d_[1]);
    proto.mutable_point_int32()->set_z(d_[2]);
  } else if constexpr (std::is_same_v<T, int64_t>) {
    proto.mutable_point_int64()->set_x(d_[0]);
    proto.mutable_point_int64()->set_y(d_[1]);
    proto.mutable_point_int64()->set_z(d_[2]);
  } else if constexpr (std::is_same_v<T, float>) {
    proto.mutable_point_float()->set_x(d_[0]);
    proto.mutable_point_float()->set_y(d_[1]);
    proto.mutable_point_float()->set_z(d_[2]);
  } else if constexpr (std::is_same_v<T, double>) {
    proto.mutable_point_double()->set_x(d_[0]);
    proto.mutable_point_double()->set_y(d_[1]);
    proto.mutable_point_double()->set_z(d_[2]);
  } else {
    static_assert(std::is_same_v<T, int>, "Unsupported type.");
  }
  return proto;
}

template <typename T>
absl::Status Point3<T>::SetFromProto(const Point3Proto& proto) {
  ABSL_ASSIGN_OR_RETURN(*this, FromProto(proto));
  return absl::OkStatus();
}

template <typename T>
absl::StatusOr<Point3<T>> Point3<T>::FromProto(const Point3Proto& proto) {
  const auto from = [](const auto& p) -> absl::StatusOr<Point3<T>> {
    ABSL_ASSIGN_OR_RETURN(const T x,
                          internal::ConvertCoordinate<T>(p.x(), "[Point3] x"));
    ABSL_ASSIGN_OR_RETURN(const T y,
                          internal::ConvertCoordinate<T>(p.y(), "[Point3] y"));
    ABSL_ASSIGN_OR_RETURN(const T z,
                          internal::ConvertCoordinate<T>(p.z(), "[Point3] z"));
    return Point3<T>(x, y, z);
  };
  switch (proto.type_variant_case()) {
    case Point3Proto::kPointInt32:
      return from(proto.point_int32());
    case Point3Proto::kPointInt64:
      return from(proto.point_int64());
    case Point3Proto::kPointFloat:
      return from(proto.point_float());
    case Point3Proto::kPointDouble:
      return from(proto.point_double());
    case Point3Proto::TYPE_VARIANT_NOT_SET:
      break;
  }
  return absl::InvalidArgumentError(
      "[Point3] The proto has no coordinate type set.");
}

// Aliases.
using Point3_i = Point3<int>;
using Point3_i32 = Point3<int32_t>;
using Point3_i64 = Point3<int64_t>;

}  // namespace moab

// Boost geometry traits.
namespace boost::geometry::traits {

template <typename T>
struct tag<moab::Point3<T>> {
  using type = point_tag;
};

template <typename T>
struct dimension<moab::Point3<T>> : boost::mpl::int_<3> {};

template <typename T>
struct coordinate_type<moab::Point3<T>> {
  using type = T;
};

template <typename T>
struct coordinate_system<moab::Point3<T>> {
  using type = boost::geometry::cs::cartesian;
};

template <typename T, std::size_t Dimension>
struct access<moab::Point3<T>, Dimension> {
  using coordinate_type = typename moab::Point3<T>::coordinate_type;

  static inline coordinate_type get(const moab::Point3<T>& p) {
    return p[Dimension];
  }
  static inline void set(moab::Point3<T>& p, const coordinate_type& value) {
    p[Dimension] = value;
  }
};

}  // namespace boost::geometry::traits

#endif  // MOAB_POINT3_H_