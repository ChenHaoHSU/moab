#ifndef MOAB_BOX2_H_
#define MOAB_BOX2_H_

#include <array>
#include <cstdint>

#include "absl/log/check.h"
#include "point2.h"

namespace moab {

template <typename T>
class Box2 {
 public:
  // Constructors.
  Box2() : d_({Point2<T>(0, 0), Point2<T>(0, 0)}) {}
  explicit Box2(const Point2<T>& p1, const Point2<T>& p2) { Set(p1, p2); }
  explicit Box2(T xl, T yl, T xh, T yh) {
    Set(Point2<T>(xl, yl), Point2<T>(xh, yh));
  }
  Box2(const Box2& b) = default;
  Box2(Box2&& b) = default;
  ~Box2() = default;

  // Accessors.
  const Point2<T>& ll() const { return d_[0]; }
  const Point2<T>& ur() const { return d_[1]; }
  const Point2<T>& MinCorner() const { return d_[0]; }
  const Point2<T>& MaxCorner() const { return d_[1]; }

  T xl() const { return d_[0].x(); }
  T yl() const { return d_[0].y(); }
  T xh() const { return d_[1].x(); }
  T yh() const { return d_[1].y(); }

  T Width() const { return d_[1].x() - d_[0].x(); }
  T Height() const { return d_[1].y() - d_[0].y(); }

  T Area() const { return Width() * Height(); }
  T Perimeter() const { return 2 * (Width() + Height()); }

  // Mutators.
  void Set(const Point2<T>& ll, const Point2<T>& ur) {
    DCHECK(ll.x() <= ur.x() && ll.y() <= ur.y());
    d_[0] = ll;
    d_[1] = ur;
  }
  void SetLL(const Point2<T>& p) { Set(p, d_[1]); }
  void SetUR(const Point2<T>& p) { Set(d_[0], p); }
  void SetMinCorner(const Point2<T>& p) { Set(p, d_[1]); }
  void SetMaxCorner(const Point2<T>& p) { Set(d_[0], p); }

  // Operations.
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

  // Operators.
  // Operators - Assignment
  Box2& operator=(const Box2& b) {
    d_ = b.d_;
    return *this;
  }
  // Operators - Subscript
  Point2<T>& operator[](int i) { return d_[i]; }
  const Point2<T>& operator[](int i) const { return d_.at(i); }
  // Operators - Equality
  bool operator==(const Box2& b) const {
    return d_[0] == b.d_[0] && d_[1] == b.d_[1];
  }
  bool operator!=(const Box2& b) const { return !(*this == b); }
  // Operators - Inequality
  bool operator<(const Box2& b) const {
    return d_[0] != b.d_[0] ? d_[0] < b.d_[0] : d_[1] < b.d_[1];
  }
  bool operator>(const Box2& b) const { return b < *this; }
  bool operator<=(const Box2& b) const { return !(*this > b); }
  bool operator>=(const Box2& b) const { return !(*this < b); }

 private:
  std::array<Point2<T>, 2> d_;
};  // class Box2

// Aliases.
using Box2_i = Box2<int>;
using Box2_i32 = Box2<int32_t>;
using Box2_i64 = Box2<int64_t>;

}  // namespace moab

#endif  // MOAB_BOX2_H_