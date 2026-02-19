//////////////////////
// Vector2.h
// Author: Yang Hsu
// Date: 2026-02-18
/////////////////////

#ifndef MOAB_VECTOR2_H_
#define MOAB_VECTOR2_H_

#include <array>
#include <cmath>
#include <cstdint>
#include <ostream>
#include <string>
#include <type_traits>
#include <utility>

#include "absl/hash/hash.h"
#include "absl/log/log.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "boost/qvm/vec_traits.hpp"
#include "moab/vector2.pb.h"

namespace moab {

template <typename T>
class Vector2 {
  using coordinate_type = T;

 public:
  // Constructors and destructor.
  Vector2() : d_({0, 0}) {}
  explicit Vector2(T x, T y) : d_({x, y}) {}
  explicit Vector2(const Vector2Proto& proto) { SetFromProto(proto); }
  Vector2(const Vector2&) = default;
  Vector2(Vector2&&) = default;
  ~Vector2() = default;

  // Assignment operators.
  Vector2& operator=(const Vector2&) = default;
  Vector2& operator=(Vector2&&) = default;

  // Accessors.

  T dx() const { return d_[0]; }
  T dy() const { return d_[1]; }
  T* data() { return d_.data(); }
  constexpr std::size_t Size() const { return d_.size(); }
  std::pair<T, T> ToPair() const { return {d_[0], d_[1]}; }

  // Mutators.
  void Set(T x, T y) {
    d_[0] = x;
    d_[1] = y;
  }
  void SetdX(T x) { d_[0] = x; }
  void SetdY(T y) { d_[1] = y; }
  void SetDim(std::size_t i, T v) { d_[i] = v; }

  // Operations.

  void Rotate90() {
    // Counter-clockwise rotation by 90 degrees: (x, y) -> (-y, x)
    T x = d_[0];
    d_[0] = -d_[1];
    d_[1] = x;
  }

  void Rotate180() {
    // Rotation by 180 degrees: (x, y) -> (-x, -y)
    d_[0] = -d_[0];
    d_[1] = -d_[1];
  }

  // Operators.
  // Operators - Subscript
  T& operator[](std::size_t i) { return d_[i]; }
  const T& operator[](std::size_t i) const { return d_.at(i); }

  // Operators - Equality
  template <typename U>
  bool operator==(const Vector2<U>& v) const {
    if (std::is_floating_point_v<T> || std::is_floating_point_v<U>) {
      constexpr double epsilon = 1e-9;
      return std::abs(static_cast<double>(d_[0]) -
                      static_cast<double>(v.dx())) < epsilon &&
             std::abs(static_cast<double>(d_[1]) -
                      static_cast<double>(v.dy())) < epsilon;
    }
    return d_[0] == v.d_[0] && d_[1] == v.d_[1];
  }
  bool operator!=(const Vector2& v) const { return !(*this == v); }

  // Operators -Inequality
  bool operator<(const Vector2& v) const {
    return d_[0] != v.d_[0] ? d_[0] < v.d_[0] : d_[1] < v.d_[1];
  }
  bool operator>(const Vector2& v) const { return v < *this; }
  bool operator<=(const Vector2& v) const { return !(*this > v); }
  bool operator>=(const Vector2& v) const { return !(*this < v); }
  // Operators - Arithmetic
  Vector2 operator+=(const Vector2& v) {
    d_[0] += v.d_[0];
    d_[1] += v.d_[1];
    return *this;
  }
  Vector2 operator-=(const Vector2& v) {
    d_[0] -= v.d_[0];
    d_[1] -= v.d_[1];
    return *this;
  }
  Vector2 operator+(const Vector2& v) const {
    return Vector2(d_[0] + v.d_[0], d_[1] + v.d_[1]);
  }
  Vector2 operator-(const Vector2& v) const {
    return Vector2(d_[0] - v.d_[0], d_[1] - v.d_[1]);
  }
  Vector2 operator+() const { return Vector2(d_[0], d_[1]); }
  Vector2 operator-() const { return Vector2(-d_[0], -d_[1]); }

  Vector2 operator+=(T s) {
    d_[0] += s;
    d_[1] += s;
    return *this;
  }

  Vector2 operator-=(T s) {
    d_[0] -= s;
    d_[1] -= s;
    return *this;
  }

  Vector2 operator*=(T s) {
    d_[0] *= s;
    d_[1] *= s;
    return *this;
  }
  Vector2 operator/=(T s) {
    d_[0] /= s;
    d_[1] /= s;
    return *this;
  }
  Vector2 operator+(T s) const { return Vector2(d_[0] + s, d_[1] + s); }
  Vector2 operator-(T s) const { return Vector2(d_[0] - s, d_[1] - s); }
  Vector2 operator*(T s) const { return Vector2(d_[0] * s, d_[1] * s); }
  Vector2 operator/(T s) const { return Vector2(d_[0] / s, d_[1] / s); }

  // String conversion.
  template <typename Sink>
  friend void AbslStringify(Sink& sink, const Vector2& v) {
    absl::Format(&sink, "(%v %v)", v.d_[0], v.d_[1]);
  }
  std::string ToString() const { return absl::StrCat(*this); }
  friend std::ostream& operator<<(std::ostream& os, const Vector2& v) {
    os << v.ToString();
    return os;
  }

  // Hash
  template <typename H>
  friend H AbslHashValue(H h, const Vector2& v) {
    return H::combine(std::move(h), v.d_[0], v.d_[1]);
  }

  // Protobuf
  // Returns a Vector2Proto.
  Vector2Proto ToProto() const;
  // Sets the Vector2 from a Vector2Proto.
  void SetFromProto(const Vector2Proto& proto);

 private:
  std::array<T, 2> d_;
};

// Protobuf
template <typename T>
Vector2Proto Vector2<T>::ToProto() const {
  Vector2Proto proto;
  if constexpr (std::is_same_v<T, int> || std::is_same_v<T, int32_t>) {
    proto.mutable_vector_int32()->set_dx(d_[0]);
    proto.mutable_vector_int32()->set_dy(d_[1]);
  } else if constexpr (std::is_same_v<T, int64_t>) {
    proto.mutable_vector_int64()->set_dx(d_[0]);
    proto.mutable_vector_int64()->set_dy(d_[1]);
  } else if constexpr (std::is_same_v<T, float>) {
    proto.mutable_vector_float()->set_dx(d_[0]);
    proto.mutable_vector_float()->set_dy(d_[1]);
  } else if constexpr (std::is_same_v<T, double>) {
    proto.mutable_vector_double()->set_dx(d_[0]);
    proto.mutable_vector_double()->set_dy(d_[1]);
  } else {
    static_assert(std::is_same_v<T, int>, "Unsupported type.");
  }
  return proto;
}

template <typename T>
void Vector2<T>::SetFromProto(const Vector2Proto& proto) {
  if (proto.has_vector_int32()) {
    d_[0] = proto.vector_int32().dx();
    d_[1] = proto.vector_int32().dy();
  } else if (proto.has_vector_int64()) {
    d_[0] = proto.vector_int64().dx();
    d_[1] = proto.vector_int64().dy();
  } else if (proto.has_vector_float()) {
    d_[0] = proto.vector_float().dx();
    d_[1] = proto.vector_float().dy();
  } else if (proto.has_vector_double()) {
    d_[0] = proto.vector_double().dx();
    d_[1] = proto.vector_double().dy();
  } else {
    LOG(FATAL) << "Unsupported type.";
  }
}

// aliases.

using Vector2_i = Vector2<int>;
using Vector2_i32 = Vector2<int32_t>;
using Vector2_i64 = Vector2<int64_t>;

}  // namespace moab

namespace boost::qvm {
template <typename T>
struct vec_traits<moab::Vector2<T>> {
  static int const dim = 2;
  using scalar_type = T;
  using vec_type = moab::Vector2<T>;

  template <int I>
  static inline scalar_type& write_element(vec_type& v) {
    return v[I];
  }
  template <int I>
  static inline scalar_type read_element(vec_type const& v) {
    return v[I];
  }

  static inline scalar_type& write_element_idx(int i, vec_type& v) {
    return v[i];
  }
  static inline scalar_type read_element_idx(int i, vec_type const& v) {
    return v[i];
  }
};
}  // namespace boost::qvm

#endif  // MOAB_VECTOR2_H_
