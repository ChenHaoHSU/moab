#ifndef MOAB_POINT3_H_
#define MOAB_POINT3_H_

#include <array>
#include <cmath>
#include <cstdint>
#include <ostream>
#include <string>
#include <tuple>
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
#include "moab/point2.h"
#include "moab/point3.pb.h"

namespace moab {

template <typename T>
class Point3 {
 public:
  // Type aliases. (Required by Boost geometry/polygon traits.)
  using coordinate_type = T;

  // Constructors.
  Point3() : d_({0, 0, 0}) {}
  explicit Point3(T x, T y, T z) : d_({x, y, z}) {}
  explicit Point3(const Point3Proto& proto) { SetFromProto(proto); }
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

  void Rotate90(size_t axis1, size_t axis2) {
    // Counterclockwise rotation by 90 degrees on (axis1, axis2) plane.
    T x = d_[axis1];
    d_[axis1] = -d_[axis2];
    d_[axis2] = x;
  }
  void Rotate180(size_t axis1, size_t axis2) {
    // Rotation by 180 degrees on (axis1, axis2) plane.
    d_[axis1] = -d_[axis1];
    d_[axis2] = -d_[axis2];
  }
  void Rotate270(size_t axis1, size_t axis2) {
    // Counterclockwise rotation by 270 degrees on (axis1, axis2) plane.
    Rotate90(axis1, axis2);
    Rotate180(axis1, axis2);
  }

  // Operators.
  // Operators - Subscript
  T& operator[](std::size_t i) { return d_[i]; }
  const T& operator[](std::size_t i) const { return d_.at(i); }
  // Operators - Equality
  bool operator==(const Point3& p) const {
    return d_[0] == p.d_[0] && d_[1] == p.d_[1] && d_[2] == p.d_[2];
  }
  bool operator!=(const Point3& p) const { return !(*this == p); }
  // Operators - Inequality
  bool operator<(const Point3& p) const {
    return d_[0] != p.d_[0]
               ? d_[0] < p.d_[0]
               : d_[1] != p.d_[1] ? d_[1] < p.d_[1] : d_[2] < p.d_[2];
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
  Point3 operator+(const Point3& p) {
    return Point3(d_[0] + p.d_[0], d_[1] + p.d_[1], d_[2] + p.d_[2]);
  }
  Point3 operator-(const Point3& p) {
    return Point3(d_[0] - p.d_[0], d_[1] - p.d_[1], d_[2] - p.d_[2]);
  }
  Point3 operator+() const { return Point3(d_[0], d_[1], d_[2]); }
  Point3 operator-() const { return Point3(-d_[0], -d_[1], -d_[2]); }

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
  T Distance(const Point3& p) const {
    return std::abs(d_[0] - p.d_[0]) + std::abs(d_[1] - p.d_[1]) +
           std::abs(d_[2] - p.d_[2]);
  }

  // String conversion.
  template <typename Sink>
  friend void AbslStringify(Sink& sink, const Point3& p) {
    absl::Format(&sink, "(%d %d %d)", p.d_[0], p.d_[1], p.d_[2]);
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
  // Sets the Point2 from a Point3Proto.
  void SetFromProto(const Point3Proto& proto);

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
void Point3<T>::SetFromProto(const Point3Proto& proto) {
  if (proto.has_point_int32()) {
    d_[0] = proto.point_int32().x();
    d_[1] = proto.point_int32().y();
    d_[2] = proto.point_int32().z();
  } else if (proto.has_point_int64()) {
    d_[0] = proto.point_int64().x();
    d_[1] = proto.point_int64().y();
    d_[2] = proto.point_int64().z();
  } else if (proto.has_point_float()) {
    d_[0] = proto.point_float().x();
    d_[1] = proto.point_float().y();
    d_[2] = proto.point_float().z();
  } else if (proto.has_point_double()) {
    d_[0] = proto.point_double().x();
    d_[1] = proto.point_double().y();
    d_[2] = proto.point_double().z();
  } else {
    LOG(FATAL) << "Unsupported type.";
  }
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