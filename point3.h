#include <array>
#include <cstdint>
#include <utility>

#include "absl/log/log.h"

namespace moab {

template <typename T>
class Point3 {
 public:
  // Constructors.
  Point3() : d_({0, 0, 0}) {}
  explicit Point3(T x, T y, T z) : d_({x, y, z}) {}
  Point3(Point3& p) {
    LOG(INFO) << "Copy constructor";
    d_ = p.d_;
  }
  Point3(Point3&& p) {
    LOG(INFO) << "Move constructor";
    d_ = p.d_;
  }
  // Destructors.
  ~Point3() = default;

  // Accessors.
  T x() const { return d_[0]; }
  T y() const { return d_[1]; }
  T z() const { return d_[2]; }
  std::array<T, 3>& data() { return d_; }
  const std::array<T, 3>& data() const { return d_; }

  // Mutators.

  // Operators.
  Point3& operator=(const Point3& p) {
    LOG(INFO) << "Operator= ";
    d_ = p.d_;
    return *this;
  }

 private:
  std::array<T, 3> d_;  // (x, y)
};

using Point3_i = Point3<int>;
using Point3_i32 = Point3<int32_t>;
using Point3_i64 = Point3<int64_t>;

}  // namespace moab