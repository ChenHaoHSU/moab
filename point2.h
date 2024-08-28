#include <array>
#include <cstdint>
#include <utility>

#include "absl/log/log.h"

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
  std::array<T, 2>& data() { return d_; }
  const std::array<T, 2>& data() const { return d_; }

  constexpr std::size_t Size() const { return d_.size(); }

  // Mutators.
  void Set(int x, int y) {
    d_[0] = x;
    d_[1] = y;
  }
  void SetX(int x) { d_[0] = x; }
  void SetY(int y) { d_[1] = y; }
  void Shift(int dx, int dy) {
    d_[0] += dx;
    d_[1] += dy;
  }
  void ShiftX(int dx) { d_[0] += dx; }
  void ShiftY(int dy) { d_[1] += dy; }

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

 private:
  std::array<T, 2> d_;  // <x, y>
};

using Point2_i = Point2<int>;
using Point2_i32 = Point2<int32_t>;
using Point2_i64 = Point2<int64_t>;

}  // namespace moab