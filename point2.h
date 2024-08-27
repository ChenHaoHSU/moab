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
  Point2(Point2& p) {
    LOG(INFO) << "Copy constructor";
    d_ = p.d_;
  }
  Point2(Point2&& p) {
    LOG(INFO) << "Move constructor";
    d_ = p.d_;
  }
  // Destructors.
  ~Point2() = default;

  // Accessors.
  T x() const { return d_[0]; }
  T y() const { return d_[1]; }
  std::array<T, 2>& data() { return d_; }
  const std::array<T, 2>& data() const { return d_; }

  // Mutators.

  // Operators.
  Point2& operator=(const Point2& p) {
    LOG(INFO) << "Operator= ";
    d_ = p.d_;
    return *this;
  }

 private:
  std::array<T, 2> d_;  // (x, y)
};

using Point2_i = Point2<int>;
using Point2_i32 = Point2<int32_t>;
using Point2_i64 = Point2<int64_t>;

}  // namespace moab